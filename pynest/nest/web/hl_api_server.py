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


__all__ = [
    'app'
]


app = Flask(__name__)
CORS(app)


@app.route('/', methods=['GET'])
@cross_origin()
def nest_index():
    data = init_data(request)
    data['response']['version'] = nest.version()
    return jsonify(data)


@app.route('/exec', methods=['GET', 'POST'])
@cross_origin()
def route_exec():
    data = init_data(request)
    args, kwargs = get_arguments(request, data)
    with Capturing() as stdout:
        try:
            exec(kwargs.get('source', ''))
            if 'return' in kwargs:
                if isinstance(kwargs['return'], list):
                    return_data = {}
                    for variable in kwargs['return']:
                        return_data[variable] = locals().get(variable, None)
                else:
                    return_data = locals().get(kwargs['return'], None)
                data['response']['data'] = nest.hl_api.serializable(return_data)
            data['response']['status'] = 'ok'
        except Exception as e:
            print(e)
            data['response']['data'] = None
            data['response']['status'] = 'error'
    data['response']['stdout'] = '\n'.join(stdout)
    return jsonify(data)


# --------------------------
# RESTful API
# --------------------------

nest_calls = dir(nest)
nest_calls = list(filter(lambda x: not x.startswith('_'), nest_calls))
nest_calls.sort()


@app.route('/api', methods=['GET'])
@cross_origin()
def nest_api():
    data = init_data(request)
    response = api_client(request, nest_calls, data)
    return jsonify(response)


@app.route('/api/<call>', methods=['GET', 'POST'])
@cross_origin()
def nest_api_call(call):
    data = init_data(request, call)
    args, kwargs = get_arguments(request, data)
    if call in nest_calls:
        call = getattr(nest, call)
        response = api_client(request, call, data, *args, **kwargs)
    else:
        data['response']['msg'] = 'The request cannot be called in NEST.'
        data['response']['status'] = 'error'
        response = data
    return jsonify(response)


# ---------------
# Helpers for web
# ---------------

class Capturing(list):
    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = io.StringIO()
        return self

    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        del self._stringio    # free up some memory
        sys.stdout = self._stdout


def init_data(request, call=None):
    url = request.url_rule.rule.split('/')[1]
    data = {
        'request': {
            'url': url,
        },
        'response': {}
    }
    if call:
        data['request']['call'] = call
    return data


def get_arguments(request, data):
    args, kwargs = [], {}
    if request.is_json:
        json = data['request']['json'] = request.get_json()
        if isinstance(json, list):
            args = json
        elif isinstance(json, dict):
            args = json.get('args', args)
            kwargs = json.get('kwargs', json)
    elif len(request.form) > 0:
        if 'args' in request.form:
            args = data['request']['form'] = request.form.getlist('args')
        else:
            kwargs = data['request']['form'] = request.form.to_dict()
    elif (len(request.args) > 0):
        if 'args' in request.args:
            args = data['request']['args'] = request.args.getlist('args')
        else:
            kwargs = data['request']['args'] = request.args.to_dict()
    return args, kwargs


def get_or_error(func):
    def func_wrapper(request, call, data, *args, **kwargs):
        try:
            data = func(request, call, data, *args, **kwargs)
            if 'data' not in data['response']:
                return data
            response = data['response']['data']
            if response is not None:
                data['response']['data'] = serialize(response, toFixed=False)
            data['response']['status'] = 'ok'
        except Exception as e:
            data['response']['msg'] = str(e)
            data['response']['status'] = 'error'
        return data
    return func_wrapper


def NodeCollection(data):
    if 'nodes' in data:
        data['nodes'] = nest.NodeCollection(data['nodes'])
    if 'source' in data:
        data['source'] = nest.NodeCollection(data['source'])
    if 'target' in data:
        data['target'] = nest.NodeCollection(data['target'])
    if 'pre' in data:
        data['pre'] = nest.NodeCollection(data['pre'])
    if 'post' in data:
        data['post'] = nest.NodeCollection(data['post'])
    return data


def serialize(data, toFixed=False):
    SLILiterals = [
        'element_type',
        'model',
        'record_from',
        'record_to',
        'recordables',
        'synapse_model',
        'type_id',
    ]

    params_infinite = [
        'V_min',
        'alpha',
    ]

    data = NodeCollection(data)
    if type(data) in [array.array, np.ndarray]:
        data = data.tolist()
    if isinstance(data, list) or isinstance(data, tuple):
        data = [serialize(d) for d in data]
        data.sort()
    elif isinstance(data, dict):
        for key, value in data.items():
            if key in SLILiterals:
                if isinstance(value, tuple):
                    data[key] = [d.name for d in data[key]]
                else:
                    data[key] = value.name
            elif key == 'events':
                for ekey, event in value.items():
                    if type(event) is np.ndarray:
                        data[key][ekey] = event.tolist()
            elif toFixed:
                data[key] = str(data[key])
            elif type(value) is np.ndarray:
                data[key] = value.tolist()
            elif key in params_infinite:
                if np.isinf(value):
                    data[key] = str(value)
            elif isinstance(value, tuple) and len(value) > 0:
                if isinstance(value[0], tuple) and len(value[0]) > 0:
                    data[key] = [[j.tolist() for j in i] for i in value]
    return data


@get_or_error
def api_client(request, call, data, *args, **kwargs):
    if callable(call):
        data['request']['call'] = call.__name__
        if str(kwargs.get('return_doc', 'false')) == 'true':
            response = call.__doc__
        elif str(kwargs.get('return_source', 'false')) == 'true':
            response = inspect.getsource(call)
        else:
            if call.__name__ == 'SetKernelStatus':
                kernelStatus = nest.GetKernelStatus()
                for paramKey, paramVal in kwargs['params'].items():
                    kwargs['params'][paramKey] = type(kernelStatus[paramKey])(paramVal)
            elif call.__name__ == 'SetStatus':
                status = nest.GetStatus(kwargs['nodes'])
                for paramKey, paramVal in kwargs['params'].items():
                    kwargs['params'][paramKey] = type(status[paramKey])(paramVal)
            response = call(*args, **kwargs)
    else:
        response = call
    data['response']['data'] = nest.hl_api.serializable(response)
    return data
