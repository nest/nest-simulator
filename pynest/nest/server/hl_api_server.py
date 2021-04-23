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

import os
MODULES = os.environ.get('NEST_SERVER_MODULES', 'nest').split(',')
RESTRICTION_OFF = bool(os.environ.get('NEST_SERVER_RESTRICTION_OFF', False))
EXCEPTION_ERROR_STATUS = 400
NEST_ERROR_STATUS = 400

if RESTRICTION_OFF:
    print('*** WARNING: NEST Server is run without a RestrictedPython trusted environment. ***')


__all__ = [
    'app'
]

app = Flask(__name__)
CORS(app)


@app.route('/', methods=['GET'])
def index():
    return jsonify({'nest': nest.__version__})


@app.route('/exec', methods=['GET', 'POST'])
@cross_origin()
def route_exec():
    """ Route to execute script in Python.
    """
    try:
        args, kwargs = get_arguments(request)
        source_code = kwargs.get('source', '')
        source_cleaned = clean_code(source_code)

        locals = dict()
        response = dict()
        if RESTRICTION_OFF:
            with Capturing() as stdout:
                exec(source_cleaned, get_globals(), locals)
            if len(stdout) > 0:
                response['stdout'] = '\n'.join(stdout)
        else:
            code = RestrictedPython.compile_restricted(source_cleaned, '<inline>', 'exec')
            exec(code, get_restricted_globals(), locals)
            if '_print' in locals:
                response['stdout'] = ''.join(locals['_print'].txt)

        if 'return' in kwargs:
            if isinstance(kwargs['return'], list):
                data = dict()
                for variable in kwargs['return']:
                    data[variable] = locals.get(variable, None)
            else:
                data = locals.get(kwargs['return'], None)
            response['data'] = nest.hl_api.serializable(data)
        return jsonify(response)

    except nest.kernel.NESTError as e:
        print('NEST error: {}'.format(e))
        abort(Response(getattr(e, 'errormessage'), NEST_ERROR_STATUS))
    except Exception as e:
        print('Error: {}'.format(e))
        abort(Response(str(e), EXCEPTION_ERROR_STATUS))


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
    args, kwargs = get_arguments(request)
    call = getattr(nest, call)
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
        except nest.kernel.NESTError as e:
            print('NEST error: {}'.format(e))
            abort(Response(getattr(e, 'errormessage'), NEST_ERROR_STATUS))
        except TypeError as e:
            print('Type error: {}'.format(e))
            abort(Response(str(e), EXCEPTION_ERROR_STATUS))
        except Exception as e:
            print('Error: {}'.format(e))
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
    args = [nest.NodeCollection(arg) if (paramKeys[idx] in objectnames) else arg for (idx, arg) in enumerate(args)]
    for (key, value) in kwargs.items():
        if key in objectnames:
            kwargs[key] = nest.NodeCollection(value)
    return args, kwargs


def serialize(call, args, kwargs):
    """ Serialize arguments with keywords for call functions in NEST.
    """
    args, kwargs = NodeCollection(call, args, kwargs)
    if call.__name__.startswith('Set'):
        status = {}
        if call.__name__ == 'SetDefaults':
            status = nest.GetDefaults(kwargs['model'])
        elif call.__name__ == 'SetKernelStatus':
            status = nest.GetKernelStatus()
        elif call.__name__ == 'SetStructuralPlasticityStatus':
            status = nest.GetStructuralPlasticityStatus(kwargs['params'])
        elif call.__name__ == 'SetStatus':
            status = nest.GetStatus(kwargs['nodes'])
        for key, val in kwargs['params'].items():
            if key in status:
                kwargs['params'][key] = type(status[key])(val)
    return args, kwargs


@get_or_error
def api_client(call, args, kwargs):
    """ API Client to call function in NEST.
    """
    if callable(call):
        if 'inspect' in kwargs:
            response = {
                'data': getattr(inspect, kwargs['inspect'])(call)
            }
        else:
            args, kwargs = serialize(call, args, kwargs)
            response = call(*args, **kwargs)
    else:
        response = call
    return nest.hl_api.serializable(response)


if __name__ == "__main__":
    app.run()
