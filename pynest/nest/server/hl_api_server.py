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

import importlib
import inspect
import io
import sys

import flask
from flask import Flask, request, jsonify
from flask_cors import CORS, cross_origin

from werkzeug.exceptions import abort
from werkzeug.wrappers import Response

import nest
import RestrictedPython
import time

import traceback

import os

MODULES = os.environ.get('NEST_SERVER_MODULES', 'nest').split(',')
RESTRICTION_OFF = bool(os.environ.get('NEST_SERVER_RESTRICTION_OFF', False))
EXCEPTION_ERROR_STATUS = 400

if RESTRICTION_OFF:
    msg = 'NEST Server runs without a RestrictedPython trusted environment.'
    print(f'***\n*** WARNING: {msg}\n***')


__all__ = [
    'app',
    'do_exec',
    'set_mpi_comm',
    'run_mpi_app',
    'NodeCollection',
]

app = Flask(__name__)
CORS(app)

mpi_comm = None


@app.route('/', methods=['GET'])
def index():
    return jsonify({
        'nest': nest.__version__,
        'mpi': mpi_comm is not None,
    })

def do_exec(args, kwargs):
    try:
        source_code = kwargs.get('source', '')
        source_cleaned = clean_code(source_code)

        locals_ = dict()
        response = dict()
        if RESTRICTION_OFF:
            with Capturing() as stdout:
                exec(source_cleaned, get_globals(), locals_)
            if len(stdout) > 0:
                response['stdout'] = '\n'.join(stdout)
        else:
            code = RestrictedPython.compile_restricted(source_cleaned, '<inline>', 'exec')
            exec(code, get_restricted_globals(), locals_)
            if '_print' in locals_:
                response['stdout'] = ''.join(locals_['_print'].txt)

        if 'return' in kwargs:
            if isinstance(kwargs['return'], list):
                data = dict()
                for variable in kwargs['return']:
                    data[variable] = locals_.get(variable, None)
            else:
                data = locals_.get(kwargs['return'], None)
            response['data'] = nest.hl_api.serializable(data)
        return response

    except Exception as e:
        for line in traceback.format_exception(*sys.exc_info()):
            print(line)
        abort(Response(str(e), EXCEPTION_ERROR_STATUS))


def log(call_name, msg):
    print(f'==> MASTER 0/{time.time():.7f} ({call_name}): {msg}')


def do_call(call_name, args=[], kwargs={}):
    """Call a function of NEST or execute a script within the server.

    If the server is running in MPI-enabled mode, this function will
    distribute the name of the function to call aa well as args and
    kwargs to the worker processes using MPI.

    In case the call_name is "exec", this function will execute the
    script either by plain exec, or in a Restricted Python trusted
    environment.

    Please note that this function must only be called by the master.

    """
    global mpi_comm

    if mpi_comm is not None:
        assert mpi_comm.Get_rank() == 0

    if mpi_comm is not None:
        log(call_name, 'sending call bcast')
        mpi_comm.bcast(call_name, root=0)
        data = (args, kwargs)
        log(call_name, f'sending data bcast, data={data}')
        mpi_comm.bcast(data, root=0)

    if call_name == "exec":
        master_response = do_exec(args, kwargs)
    else:
        call = getattr(nest, call_name)
        args, kwargs = serialize(call_name, args, kwargs)
        args, kwargs = NodeCollection(call, args, kwargs)
        log(call_name, f'local call, args={args}, kwargs={kwargs}')
        master_response = call(*args, **kwargs)

    response = [None]
    if mpi_comm is not None:
        log(call_name, 'waiting for response gather')
        response = mpi_comm.gather(None, root=0)
        log(call_name, f'received response gather, data={response}')
    response[0] = nest.hl_api.serializable(master_response)

    return combine(response)


@app.route('/exec', methods=['GET', 'POST'])
@cross_origin()
def route_exec():
    """ Route to execute script in Python.
    """

    args, kwargs = get_arguments(request)
    response = do_call('exec', args, kwargs)
    return jsonify(response)


# --------------------------
# RESTful API
# --------------------------

nest_calls = dir(nest)
nest_calls = list(filter(lambda x: not x.startswith('_'), nest_calls))
nest_calls.sort()


@app.route('/api', methods=['GET'])
@cross_origin()
def route_api():
    """ Route to list call functions in NEST.
    """
    return jsonify(nest_calls)


@app.route('/api/<call>', methods=['GET', 'POST'])
@cross_origin()
def route_api_call(call):
    """ Route to call function in NEST.
    """
    print("\n========================================\n")
    args, kwargs = get_arguments(request)
    response = api_client(call, args, kwargs)
    return jsonify(response)


# ----------------------
# Helpers for the server
# ----------------------

class Capturing(list):
    """ Monitor stdout contents i.e. print.
    """
    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = io.StringIO()
        return self

    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        del self._stringio    # free up some memory
        sys.stdout = self._stdout


def clean_code(source):
    codes = source.split('\n')
    code_cleaned = filter(lambda code: not (code.startswith('import') or code.startswith('from')), codes)
    return '\n'.join(code_cleaned)


def get_arguments(request):
    """ Get arguments from the request.
    """
    args, kwargs = [], {}
    if request.is_json:
        json = request.get_json()
        if isinstance(json, list):
            args = json
        elif isinstance(json, dict):
            kwargs = json
            if 'args' in kwargs:
                args = kwargs.pop('args')
    elif len(request.form) > 0:
        if 'args' in request.form:
            args = request.form.getlist('args')
        else:
            kwargs = request.form.to_dict()
    elif len(request.args) > 0:
        if 'args' in request.args:
            args = request.args.getlist('args')
        else:
            kwargs = request.args.to_dict()
    return list(args), kwargs


def get_globals():
    """ Get globals for exec function.
    """
    copied_globals = globals().copy()

    # Add modules to copied globals
    modules = dict([(module, importlib.import_module(module)) for module in MODULES])
    copied_globals.update(modules)

    return copied_globals


def get_or_error(func):
    """ Wrapper to get data and status.
    """
    def func_wrapper(call, args, kwargs):
        try:
            return func(call, args, kwargs)
        except Exception as e:
            for line in traceback.format_exception(*sys.exc_info()):
                print(line)
            abort(Response(str(e), EXCEPTION_ERROR_STATUS))
    return func_wrapper


def get_restricted_globals():
    """ Get restricted globals for exec function.
    """
    def getitem(obj, index):
        if obj is not None and type(obj) in (list, tuple, dict, nest.NodeCollection):
            return obj[index]
        msg = f"Error while getting restricted globals: unidentified object '{obj}'."
        raise TypeError(msg)

    restricted_builtins = RestrictedPython.safe_builtins.copy()
    restricted_builtins.update(RestrictedPython.limited_builtins)
    restricted_builtins.update(RestrictedPython.utility_builtins)
    restricted_builtins.update(dict(
        max=max,
        min=min,
        sum=sum,
        time=time,
    ))

    restricted_globals = dict(
        __builtins__=restricted_builtins,
        _print_=RestrictedPython.PrintCollector,
        _getattr_=RestrictedPython.Guards.safer_getattr,
        _getitem_=getitem,
        _getiter_=iter,
        _unpack_sequence_=RestrictedPython.Guards.guarded_unpack_sequence,
        _write_=RestrictedPython.Guards.full_write_guard,
    )

    # Add modules to restricted globals
    modules = dict([(module, importlib.import_module(module)) for module in MODULES])
    restricted_globals.update(modules)

    return restricted_globals


def NodeCollection(call, args, kwargs):
    """ Get Node Collection as arguments for NEST functions.
    """

    objectnames = ['nodes', 'source', 'target', 'pre', 'post']
    paramKeys = list(inspect.signature(call).parameters.keys())
    args = [nest.NodeCollection(arg) if paramKeys[idx] in objectnames
            else arg for (idx, arg) in enumerate(args)]
    for (key, value) in kwargs.items():
        if key in objectnames:
            kwargs[key] = nest.NodeCollection(value)

    return args, kwargs


def serialize(call_name, args, kwargs):
    """Serialize arguments with keywords for calling functions in NEST.

    TODO: Explain why we need to run the inverse getter.

    TODO: Find out why the `if "params" in kwargs:` condition is
    needed in the MPI enabled version, but did not seem to be required
    in the normal NEST Server.

    When calling the inverse getter function, we only look at the
    information available on the master, as we're only interested in
    the keys. We still have to call the getters also on the workers,
    as not doing so might lead to deadlocks due to unmatched MPI
    calls, if the getters themselves initiate MPI communication calls
    internally.

    """

    if call_name.startswith('Set'):
        status = {}
        if call_name == 'SetDefaults':
            status = do_call('GetDefaults', [kwargs['model']])
        elif call_name == 'SetKernelStatus':
            status = do_call('GetKernelStatus')
        elif call_name == 'SetStructuralPlasticityStatus':
            status = do_call('GetStructuralPlasticityStatus', [kwargs['params']])
        elif call_name == 'SetStatus':
            status = do_call('GetStatus', [kwargs['nodes']])
        if "params" in kwargs:
            for key, val in kwargs['params'].items():
                if key in status:
                    kwargs['params'][key] = type(status[key])(val)
    return args, kwargs


@get_or_error
def api_client(call_name, args, kwargs):
    """ API Client to call function in NEST.
    """
    global mpi_comm

    call = getattr(nest, call_name)

    if callable(call):
        if 'inspect' in kwargs:
            response = {
                'data': getattr(inspect, kwargs['inspect'])(call)
            }
        else:
            response = do_call(call_name, args, kwargs)
    else:
        response = call
    return nest.hl_api.serializable(response)


def set_mpi_comm(comm):
    global mpi_comm
    mpi_comm = comm


def run_mpi_app():
    # NEST segfaults if someone messes with the number of threads, so we don't.
    app.run(threaded=False)


def combine(response):
    """Combine responses from different MPI processes.

    """

    return response[0]

if __name__ == "__main__":
    app.run()
