# -*- coding: utf-8 -*-
#
# hl_api_server.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

import ast
import importlib
import inspect
import io
import logging
import os
import sys
import time
import traceback
from copy import deepcopy

import flask
import nest
import RestrictedPython
from flask import Flask, jsonify, request
from flask.logging import default_handler
from flask_cors import CORS
from nest.lib.hl_api_exceptions import NESTError

# This ensures that the logging information shows up in the console running the server,
# even when Flask's event loop is running.
logger = logging.getLogger()
logger.addHandler(default_handler)
logger.setLevel(os.getenv("NEST_SERVER_MPI_LOGGER_LEVEL", "INFO"))


def get_boolean_environ(env_key, default_value="false"):
    env_value = os.environ.get(env_key, default_value)
    return env_value.lower() in ["yes", "true", "t", "1"]


_default_origins = "http://localhost:*,http://127.0.0.1:*"
ACCESS_TOKEN = os.environ.get("NEST_SERVER_ACCESS_TOKEN", "")
AUTH_DISABLED = get_boolean_environ("NEST_SERVER_DISABLE_AUTH")
CORS_ORIGINS = os.environ.get("NEST_SERVER_CORS_ORIGINS", _default_origins).split(",")
EXEC_CALL_ENABLED = get_boolean_environ("NEST_SERVER_ENABLE_EXEC_CALL")
MODULES = os.environ.get("NEST_SERVER_MODULES", "import nest")
RESTRICTION_DISABLED = get_boolean_environ("NEST_SERVER_DISABLE_RESTRICTION")
EXCEPTION_ERROR_STATUS = 400

__all__ = [
    "app",
    "do_exec",
    "set_mpi_comm",
    "run_mpi_app",
    "nestify",
]

app = Flask(__name__)
# Inform client-side user agents that they should not attempt to call our server from any
# non-whitelisted domain.
CORS(app, origins=CORS_ORIGINS, methods=["GET", "POST"])

mpi_comm = None


def _check_security():
    """
    Checks the security level of the NEST Server instance.
    """

    msg = []
    if AUTH_DISABLED:
        msg.append("AUTH:\tThe authorization settings are disabled.")
    if "*" in CORS_ORIGINS:
        msg.append("CORS:\tThe allowed origins are not restricted.")
    if EXEC_CALL_ENABLED:
        msg.append("EXEC CALL:\tThe exec route is enabled and scripts can be executed.")
        if RESTRICTION_DISABLED:
            msg.append("RESTRICTION: The execution of scripts is not protected by RestrictedPython.")

    if len(msg) > 0:
        print(
            "WARNING: You chose to disable important access restrictions!\n"
            " This allows other computers to execute code on this machine as the current user!\n"
            " Be sure you understand the implications of these settings and take"
            " appropriate measures to protect your runtime environment!"
        )
        print("\n - ".join([" "] + msg) + "\n")


@app.before_request
def _setup_auth():
    """
    Authentication function that generates and validates the NESTServerAuth header with a
    bearer token.

    Cleans up references to itself and the running `app` from this module, as it may be
    accessible when the code execution sandbox fails.
    """
    try:
        # Import the modules inside of the auth function, so that if they fail the auth
        # returns a forbidden error.
        import gc  # noqa
        import hashlib  # noqa
        import inspect  # noqa
        import time  # noqa

        # Find our reference to the current function in the garbage collector.
        frame = inspect.currentframe()
        code = frame.f_code
        globs = frame.f_globals
        functype = type(lambda: 0)
        funcs = []
        for func in gc.get_referrers(code):
            if type(func) is functype:
                if getattr(func, "__code__", None) is code:
                    if getattr(func, "__globals__", None) is globs:
                        funcs.append(func)
                        if len(funcs) > 1:
                            return ("Unauthorized", 403)
        self = funcs[0]

        # Use the salted hash (unless `PYTHONHASHSEED` is fixed) of the location of this
        # function in the Python heap and the current timestamp to create a SHA512 hash.
        if not hasattr(self, "_hash"):
            if ACCESS_TOKEN:
                self._hash = ACCESS_TOKEN
            else:
                hasher = hashlib.sha512()
                hasher.update(str(hash(id(self))).encode("utf-8"))
                hasher.update(str(time.perf_counter()).encode("utf-8"))
                self._hash = hasher.hexdigest()[:48]
            if not AUTH_DISABLED:
                print(f"   Access token to NEST Server: {self._hash}")
                print("   Add this to the headers: {'NESTServerAuth': '<access_token>'}\n")

        if request.method == "OPTIONS":
            return

        # The first time we hit the line below is when below the function definition we
        # call `setup_auth` without any Flask request existing yet, so the function errors
        # and exits here after generating and storing the auth hash.
        auth = request.headers.get("NESTServerAuth", None)
        # We continue here the next time this function is called, before the Flask app
        # handles the first request. At that point we also remove this module's reference
        # to the running app.
        try:
            del globals()["app"]
        except KeyError:
            pass
        # Things get more straightforward here: Every time a request is handled, compare
        # the NESTServerAuth header to the hash, with a constant-time algorithm to avoid
        # timing attacks.
        if not (AUTH_DISABLED or auth == self._hash):
            return ("Unauthorized", 403)
    # DON'T LINT! Intentional bare except clause! Even `KeyboardInterrupt` and
    # `SystemExit` exceptions should not bypass authentication!
    except Exception:  # noqa
        return ("Unauthorized", 403)


print(80 * "*")
_check_security()
_setup_auth()
del _setup_auth
print(80 * "*")


@app.route("/", methods=["GET"])
def index():
    return jsonify(
        {
            "nest": nest.__version__,
            "mpi": mpi_comm is not None,
        }
    )


def do_exec(args, kwargs):
    source_code = kwargs.get("source", "")
    source_cleaned = clean_code(source_code)

    locals_ = dict()
    response = dict()
    if RESTRICTION_DISABLED:
        with Capturing() as stdout:
            globals_ = globals().copy()
            globals_.update(get_modules_from_env())
            get_or_error(exec)(source_cleaned, globals_, locals_)
        if len(stdout) > 0:
            response["stdout"] = "\n".join(stdout)
    else:
        code = RestrictedPython.compile_restricted(source_cleaned, "<inline>", "exec")  # noqa
        globals_ = get_restricted_globals()
        globals_.update(get_modules_from_env())
        get_or_error(exec)(code, globals_, locals_)
        if "_print" in locals_:
            response["stdout"] = "".join(locals_["_print"].txt)

    if "return" in kwargs:
        if isinstance(kwargs["return"], (list, tuple)):
            data = dict([(variable, locals_.get(variable, None)) for variable in kwargs["return"]])
        else:
            data = locals_.get(kwargs["return"], None)

        response["data"] = get_or_error(nest.serialize_data)(data)
    return response


def log(call_name, msg):
    msg = f"==> MASTER 0/{time.time():.7f} ({call_name}): {msg}"
    logger.debug(msg)


def do_call(call_name, args=[], kwargs={}):
    """Call a PYNEST function or execute a script within the server.

    If the server is run serially (i.e., without MPI), this function
    will do one of two things: If call_name is "exec", it will execute
    the script given in args via do_exec(). If call_name is the name
    of a PyNEST API function, it will call that function and pass args
    and kwargs to it.

    If the server is run with MPI, this function will first communicate
    the call type ("exec" or API call) and the args and kwargs to all
    worker processes. Only then will it execute the call in the same
    way as described above for the serial case. After the call, all
    worker responses are collected, combined and returned.

    Please note that this function must only be called on the master
    process (i.e., the task with rank 0) in a distributed scenario.

    """

    if mpi_comm is not None:
        assert mpi_comm.Get_rank() == 0

    if mpi_comm is not None:
        log(call_name, "sending call bcast")
        mpi_comm.bcast(call_name, root=0)
        data = (args, kwargs)
        log(call_name, f"sending data bcast, data={data}")
        mpi_comm.bcast(data, root=0)

    if call_name == "exec":
        master_response = do_exec(args, kwargs)
    else:
        call, args, kwargs = nestify(call_name, args, kwargs)
        log(call_name, f"local call, args={args}, kwargs={kwargs}")
        master_response = call(*args, **kwargs)

    response = [master_response]
    if mpi_comm is not None:
        log(call_name, "waiting for response gather")
        response = mpi_comm.gather(response[0], root=0)
        log(call_name, f"received response gather, data={response}")

    return combine(response)


@app.route("/exec", methods=["GET", "POST"])
def route_exec():
    """Route to execute script in Python."""

    if EXEC_CALL_ENABLED:
        args, kwargs = get_arguments(request)
        response = do_call("exec", args, kwargs)
        return jsonify(response)
    else:
        flask.abort(
            403,
            "The route `/exec` has been disabled. Please contact the server administrator.",
        )


# --------------------------
# RESTful API
# --------------------------

nest_calls = dir(nest)
nest_calls = list(filter(lambda x: not x.startswith("_"), nest_calls))
nest_calls.sort()


@app.route("/api", methods=["GET"])
def route_api():
    """Route to list call functions in NEST."""
    return jsonify(nest_calls)


@app.route("/api/<call>", methods=["GET", "POST"])
def route_api_call(call):
    """Route to call function in NEST."""
    print(f"\n{'='*40}\n", flush=True)
    args, kwargs = get_arguments(request)
    log("route_api_call", f"call={call}, args={args}, kwargs={kwargs}")
    response = api_client(call, args, kwargs)
    return jsonify(response)


# ----------------------
# Helpers for the server
# ----------------------


class Capturing(list):
    """Monitor stdout contents i.e. print."""

    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = io.StringIO()
        return self

    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        del self._stringio  # free up some memory
        sys.stdout = self._stdout


class ErrorHandler(Exception):
    status_code = 400
    lineno = -1

    def __init__(self, message: str, lineno: int = None, status_code: int = None, payload=None):
        super().__init__()
        self.message = message
        if status_code is not None:
            self.status_code = status_code
        if lineno is not None:
            self.lineno = lineno
        self.payload = payload

    def to_dict(self):
        rv = dict(self.payload or ())
        rv["message"] = self.message
        if self.lineno != -1:
            rv["lineNumber"] = self.lineno
        return rv


# https://flask.palletsprojects.com/en/2.3.x/errorhandling/
@app.errorhandler(ErrorHandler)
def error_handler(e):
    return jsonify(e.to_dict()), e.status_code


# It comments lines starting with 'import' or 'from' otherwise the line number of error would be wrong.
def clean_code(source):
    codes = source.split("\n")
    codes_cleaned = []  # noqa
    for code in codes:
        if code.startswith("import") or code.startswith("from"):
            codes_cleaned.append("#" + code)
        else:
            codes_cleaned.append(code)
    return "\n".join(codes_cleaned)


def get_arguments(request):
    """Get arguments from the request."""
    args, kwargs = [], {}
    if request.is_json:
        json = request.get_json()
        if isinstance(json, str) and len(json) > 0:
            args = [json]
        elif isinstance(json, list):
            args = json
        elif isinstance(json, dict):
            kwargs = json
            if "args" in kwargs:
                args = kwargs.pop("args")
    elif len(request.form) > 0:
        if "args" in request.form:
            args = request.form.getlist("args")
        else:
            kwargs = request.form.to_dict()
    elif len(request.args) > 0:
        if "args" in request.args:
            args = request.args.getlist("args")
        else:
            kwargs = request.args.to_dict()
    return list(args), kwargs


def get_lineno(err, tb_idx):
    lineno = -1
    if hasattr(err, "lineno") and err.lineno is not None:
        lineno = err.lineno
    else:
        tb = sys.exc_info()[2]
        lineno = traceback.extract_tb(tb)[tb_idx][1]
    return lineno


def get_modules_from_env():
    """Get modules from environment variable NEST_SERVER_MODULES.

    This function converts the content of the environment variable NEST_SERVER_MODULES:
    to a formatted dictionary for updating the Python `globals`.

    Here is an example:
        `NEST_SERVER_MODULES="import nest; import numpy as np; from numpy import random"`
    is converted to the following dictionary:
        `{'nest': <module 'nest'> 'np': <module 'numpy'>, 'random': <module 'numpy.random'>}`
    """
    modules = {}
    try:
        parsed = ast.iter_child_nodes(ast.parse(MODULES))
    except (SyntaxError, ValueError):
        raise SyntaxError("The NEST server module environment variables contains syntax errors.")
    for node in parsed:
        if isinstance(node, ast.Import):
            for alias in node.names:
                modules[alias.asname or alias.name] = importlib.import_module(alias.name)
        elif isinstance(node, ast.ImportFrom):
            for alias in node.names:
                modules[alias.asname or alias.name] = importlib.import_module(f"{node.module}.{alias.name}")
    return modules


def get_or_error(func):
    """Wrapper to get data and status."""

    def func_wrapper(call, *args, **kwargs):
        try:
            return func(call, *args, **kwargs)

        except NESTError as err:
            error_class = err.errorname + " (NESTError)"
            detail = err.errormessage
            lineno = get_lineno(err, 1)

        except (KeyError, SyntaxError, TypeError, ValueError) as err:
            error_class = err.__class__.__name__
            detail = err.args[0]
            lineno = get_lineno(err, 1)

        except Exception as err:
            error_class = err.__class__.__name__
            detail = err.args[0]
            lineno = get_lineno(err, -1)

        for line in traceback.format_exception(*sys.exc_info()):
            print(line, flush=True)

        if lineno == -1:
            message = "%s: %s" % (error_class, detail)
        else:
            message = "%s at line %d: %s" % (error_class, lineno, detail)

        raise ErrorHandler(message, lineno)

    return func_wrapper


def get_restricted_globals():
    """Get restricted globals for exec function."""

    def getitem(obj, index):
        typelist = (list, tuple, dict, nest.NodeCollection)
        if obj is not None and type(obj) in typelist:
            return obj[index]
        msg = f"Error getting restricted globals: unidentified object '{obj}'."
        raise TypeError(msg)

    restricted_builtins = RestrictedPython.safe_builtins.copy()
    restricted_builtins.update(RestrictedPython.limited_builtins)
    restricted_builtins.update(RestrictedPython.utility_builtins)
    restricted_builtins.update(
        dict(
            max=max,
            min=min,
            sum=sum,
            time=time,
        )
    )

    restricted_globals = dict(
        __builtins__=restricted_builtins,
        _print_=RestrictedPython.PrintCollector,
        _getattr_=RestrictedPython.Guards.safer_getattr,
        _getitem_=getitem,
        _getiter_=iter,
        _unpack_sequence_=RestrictedPython.Guards.guarded_unpack_sequence,
        _write_=RestrictedPython.Guards.full_write_guard,
    )

    return restricted_globals


def nestify(call_name, args, kwargs):
    """Get the NEST API call and convert arguments if neccessary."""

    call = getattr(nest, call_name)
    objectnames = ["nodes", "source", "target", "pre", "post"]
    paramKeys = list(inspect.signature(call).parameters.keys())
    args = [nest.NodeCollection(arg) if paramKeys[idx] in objectnames else arg for (idx, arg) in enumerate(args)]
    for key, value in kwargs.items():
        if key in objectnames:
            kwargs[key] = nest.NodeCollection(value)

    return call, args, kwargs


@get_or_error
def api_client(call_name, args, kwargs):
    """API Client to call function in NEST."""

    call = getattr(nest, call_name)

    if callable(call):
        if "inspect" in kwargs:
            response = {"data": getattr(inspect, kwargs["inspect"])(call)}
        else:
            response = do_call(call_name, args, kwargs)
    else:
        response = call

    return nest.serialize_data(response)


def set_mpi_comm(comm):
    global mpi_comm
    mpi_comm = comm


def run_mpi_app(host="127.0.0.1", port=52425):
    # NEST crashes with a segmentation fault if the number of threads
    # is changed from the outside. Calling run() with threaded=False
    # prevents Flask from performing such changes.
    app.run(host=host, port=port, threaded=False)


def combine(response: list) -> dict | list | None:
    """Combine responses from different MPI processes.

    In a distributed scenario, each MPI process creates its own share
    of the response from the data available locally. To present a
    coherent view on the reponse data for the caller, this data has to
    be combined.

    If this function is run serially (i.e., without MPI), it just
    returns the response data from the only process immediately.

    The type of the returned result can vary depending on the call
    that produced it.

    The combination of results is based on a cascade of heuristics
    based on the call that was issued and individual repsonse data:
      * if all responses are None, the combined response will also just
        be None
      * the responses are known to be the same from the master and all
        workers. In this case, the combined response is just the master
        response
      * if the response list contains only a single actual response and
        None otherwise, the combined response will be that one actual
        response
       * if the response contains one list per process, the combined
        response will be those first list

    """

    if type(response) is not list or len(response) == 0:
        return response

    # return first dictionary if the response contains only one element.
    elif mpi_comm is None or len(response) == 1:
        return response[0]

    elif all(v is None for v in response):
        return None

    # return first list/tuple if the response only consists of lists or tuples.
    elif all(isinstance(v, (list, tuple)) for v in response):
        return response[0]  # TODO consider alternative: _flatten(response)

    # return a single merged dictionary if there are many of them
    elif all(type(v) is dict for v in response):
        return merge_dicts(response)

    log("combine()", f"ERROR: cannot combine response={response}")
    msg = "Cannot combine data because of unknown reason"
    raise Exception(msg)  # pylint: disable=W0719


def merge_dicts(response: list[dict]) -> dict:
    """Merge dictionaries of the response.

    This function runs through a zipped list and performs the
    following steps:
      * sum up all n_events fields
      * if recording to memory: merge the event dictionaries by joining
        all contained arrays
      * if recording to ascii: join filenames arrays
      * take all other values directly from the device on the first
        process

    Parameters
    ----------
    response: list
        list of response

    """

    # return first dictionary if the response contains only one element.
    if len(response) == 1:
        return response[0]

    # if the response comes from recorders in exec call.
    elif "data" in response[0]:
        data = response[0]["data"]
        if len(data) == 1:
            data_key = list(data.keys())[0]
            response_data = [res["data"][data_key] for res in response]
            merged = [_merge_dict([d[idx] for d in response_data]) for idx in range(len(response_data[0]))]
            return dict([["data", dict([[data_key, merged]])]])

    # if the response contains duplicates because of the MPI.
    else:
        keys = [list(r.keys()) for r in response]
        if len(keys[0]) == len(set(sum(keys, []))):
            return response[0]

    log("merge_dict()", f"ERROR: cannot merge dict={response}")
    msg = "Cannot merge dict because of unknown reason"
    raise Exception(msg)  # pylint: disable=W0719


def _flatten(data: list[dict | list]) -> list:
    """Flatten nested list.

    Parameters
    ----------
    data: list
        Nested list of data

    """
    return [elem for dl in data for elem in dl]


def _merge_dict(data: list[dict]) -> dict:
    """Merge dictionaries of the list.

    Parameters
    ----------
    data: list
        List of dictionary data

    """
    data_keys = list(set(_flatten(data)))
    return dict([(data_key, _flatten([d[data_key] for d in data])) for data_key in data_keys])


def merge_response(response: list):
    if "events" in response[0]["data"]:
        events = [res["data"]["events"] for res in response]
        merged = [_merge_event([e[idx] for e in events]) for idx in range(len(events[0]))]
        return [{"data": {"events": merged}}]
    else:
        return response


def _flatten(xss):
    return [x for xs in xss for x in xs]


def _merge_event(event: list):
    eventKeys = list(set(_flatten([e for e in event])))
    return dict([(eKey, _flatten([e[eKey] for e in event])) for eKey in eventKeys])


if __name__ == "__main__":
    app.run()
