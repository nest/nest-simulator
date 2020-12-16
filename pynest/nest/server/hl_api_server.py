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

from copy import deepcopy

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
            code = RestrictedPython.compile_restricted(source_cleaned, '<inline>', 'exec')  # noqa
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

    return combine(call_name, response)


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
    code_cleaned = filter(lambda code: not (code.startswith('import') or code.startswith('from')), codes)  # noqa
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
    modlist = [(module, importlib.import_module(module)) for module in MODULES]
    modules = dict(modlist)
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
        typelist = (list, tuple, dict, nest.NodeCollection)
        if obj is not None and type(obj) in typelist:
            return obj[index]
        msg = f"Error getting restricted globals: unidentified object '{obj}'."
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
    modlist = [(module, importlib.import_module(module)) for module in MODULES]
    modules = dict(modlist)
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
            status = do_call('GetStructuralPlasticityStatus', [kwargs['params']])  # noqa
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


def run_mpi_app(host="127.0.0.1", port=5000):
    # NEST segfaults if someone messes with the number of threads, so we don't.
    app.run(host=host, port=port, threaded=False)


def combine(call_name, response):
    """Combine responses from different MPI processes.

    This function combines the responses of all MPI processes and
    returns a single response object. The type of the result can vary
    depending on the call of the call that produced it.

    The combination of results is based on a cascade of heuristics
    based on the call that was issued and individual repsonse data:
      * if all responses are None, the combined response will also just
        be None
      * for some specific calls, the responses are known to be the same
        from all workers and the combined response is just the first
        element of the response list
      * if the response list contains only a single actual response and
        None otherwise, the combined response will be that one actual
        response
      * for calls to GetStatus on recording devices, the combined
        response will be a merged dictionary in the sense that all
        fields that contain a single value in the individual responsed
        are kept as a single values, while lists will be appended in
        order of appearance; dictionaries in the response are
        recursively treated in the same way
      * for calls to GetStatus on neurons, the combined response is just
        the single dictionary returned by the process on which the
        neuron is actually allocated

    """

    if all(v is None for v in response):
        result = None
    elif call_name in ('exec', 'Create', 'GetDefaults'):
        result = response[0]
    else:
        result = list(filter(lambda x: x is not None, response))
        if len(result) == 1:
            result = result[0]
        else:
            if all(type(v[0]) is dict for v in response):
                result = merge_dicts(response)
            else:
                msg = "Cannot combine data because of unknown reason"
                raise Exception(msg)

    return result


def merge_dicts(response):
    """Merge status dictionaries of recorders

    This function runs through a zipped list and performs the
    following steps:
      * sum up all n_events fields
      * if recording to memory: merge the event dictionaries by joining
        all contained arrays
      * if recording to ascii: join filenames arrays
      * take all other values directly from the device on the first
        process

    """

    result = []

    for device_dicts in zip(*response):

        # TODO: either stip fields like thread, vp, thread_local_id,
        # and local or make them lists that contain the values from
        # all dicts.

        element_type = device_dicts[0]['element_type']

        if element_type not in ('neuron', 'recorder', 'stimulator'):
            msg = f'Cannot combine data of element with type "{element_type}".'
            raise Exception(msg)

        if element_type == 'neuron':
            tmp = list(filter(lambda status: status['local'], device_dicts))
            assert len(tmp) == 1
            result.append(tmp[0])

        if element_type == 'recorder':
            tmp = deepcopy(device_dicts[0])
            tmp['n_events'] = 0

            for device_dict in device_dicts:
                tmp['n_events'] += device_dict['n_events']

            record_to = tmp['record_to']
            if record_to not in ('ascii', 'memory'):
                msg = f'Cannot combine data when recording to "{record_to}".'
                raise Exception(msg)

            if record_to == 'memory':
                event_keys = tmp['events'].keys()
                for key in event_keys:
                    tmp['events'][key] = []
                for device_dict in device_dicts:
                    for key in event_keys:
                        tmp['events'][key].extend(device_dict['events'][key])

            if record_to == 'ascii':
                tmp['filenames'] = []
                for device_dict in device_dicts:
                    tmp['filenames'].extend(device_dict['filenames'])

            result.append(tmp)

        if element_type == 'stimulator':
            result.append(device_dicts[0])

    return result


if __name__ == "__main__":
    app.run()
