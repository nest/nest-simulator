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

import array
import inspect
import io
import numpy as np
import os
import sys

import nest

import flask
from flask import Flask, request, jsonify
from flask_cors import CORS, cross_origin
from werkzeug import abort, Response


__all__ = [
    'app'
]

app = Flask(__name__)
CORS(app)


@app.route('/version', methods=['GET'])
@cross_origin()
def route_version():
    """ Route to fetch version of NEST Simulator.
    """
    return jsonify(nest.version())


@app.route('/exec', methods=['GET', 'POST'])
@cross_origin()
def route_exec():
    """ Route to execute script in Python.
    """
    args, kwargs = get_arguments(request)
    with Capturing() as stdout:
        try:
            source = kwargs.get('source', '')
            globals = {'__builtins__': None}
            locals = {
              'list': list,
              'nest': nest,
              'np': np,
              'print': print,
              'set': set,
            }
            exec(source, globals, locals)
            response = {}
            if 'return' in kwargs:
                if isinstance(kwargs['return'], list):
                    return_data = {}
                    for variable in kwargs['return']:
                        return_data[variable] = locals.get(variable, None)
                else:
                    return_data = locals.get(kwargs['return'], None)
                response['data'] = nest.hl_api.serializable(return_data)
            response['stdout'] = '\n'.join(stdout)
            return jsonify(data)
        except nest.kernel.NESTError as e:
            abort(Response(getattr(e, 'errormessage'), 400))
        except Exception as e:
            abort(Response(str(e), 400))


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
    data = api_client(call, *args, **kwargs)
    return jsonify(data)


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


def get_arguments(request):
    """ Get arguments from the request.
    """
    args, kwargs = [], {}
    if request.is_json:
        json = request.get_json()
        if isinstance(json, list):
            args = json
        elif isinstance(json, dict):
            args = json.get('args', args)
            kwargs = json.get('kwargs', json)
    elif len(request.form) > 0:
        if 'args' in request.form:
            args = request.form.getlist('args')
        else:
            kwargs = request.form.to_dict()
    elif (len(request.args) > 0):
        if 'args' in request.args:
            args = request.args.getlist('args')
        else:
            kwargs = request.args.to_dict()
    return args, kwargs


def get_or_error(func):
    """ Wrapper to get data and status.
    """
    def func_wrapper(call, *args, **kwargs):
        try:
            data = func(call,  *args, **kwargs)
            return data
        except nest.kernel.NESTError as e:
            abort(Response(getattr(e, 'errormessage'), 409))
        except Exception as e:
            abort(Response(str(e), 400))
        return data
    return func_wrapper


def NodeCollection(params):
    """ Get Node Collection as arguments for NEST functions.
    """
    keys = ['nodes', 'source', 'target', 'pre', 'post']
    for key in keys:
        if key in params:
            params[key] = nest.NodeCollection(params[key])
    return params


def serialize(call, params):
    """ Serialize arguments with keywords for call functions in NEST.
    """
    nodeCollection = NodeCollection(params)
    if call.startswith('Set'):
        status = {}
        if call == 'SetDefaults':
            status = nest.GetDefaults(nodeCollection['model'])
        elif call == 'SetKernelStatus':
            status = nest.GetKernelStatus()
        elif call == 'SetStructuralPlasticityStatus':
            status = nest.GetStructuralPlasticityStatus(nodeCollection['params'])
        elif call == 'SetStatus':
            status = nest.GetStatus(nodeCollection['nodes'])
        for key, val in kwargs['params'].items():
            if key in status:
                nodeCollection['params'][key] = type(status[key])(val)
    return nodeCollection


@get_or_error
def api_client(call, *args, **kwargs):
    """ API Client to call function in NEST.
    """
    call = getattr(nest, call)
    if callable(call):
        if str(kwargs.get('return_doc', 'false')) == 'true':
            response = call.__doc__
        elif str(kwargs.get('return_source', 'false')) == 'true':
            response = inspect.getsource(call)
        else:
            response = call(*args, **serialize(call.__name__, kwargs))
    else:
        response = call
    return nest.hl_api.serializable(response)
