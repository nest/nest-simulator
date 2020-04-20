# -*- coding: utf-8 -*-
#
# NESTServerClient.py
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

import requests
from werkzeug.exceptions import BadRequest


__all__ = [
    'NESTClientAPI',
    'NESTClientExec',
]


def encode(response):
    if response.ok:
        return response.json()
    elif response.status_code == 400:
        raise BadRequest(response.text)


class NESTClientAPI(object):

    def __init__(self, host='localhost', port=5000):
        self.url = 'http://{}:{}/api/'.format(host, port)
        self.headers = {'Content-type': 'application/json', 'Accept': 'text/plain'}

    def _nest_server_api_call(self, call, params={}):
        response = requests.post(self.url + call, json=params, headers=self.headers)
        return encode(response)

    def __getattr__(self, name):
        def method(*args, **kwargs):
            kwargs.update({'args': args})
            return self._nest_server_api_call(name, kwargs)
        return method


class NESTClientExec(object):

    def __init__(self, host='localhost', port=5000):
        self.url = 'http://{}:{}/exec'.format(host, port)
        self.headers = {'Content-type': 'application/json', 'Accept': 'text/plain'}

    def from_file(self, filename, return_vars=None):
        with open(filename, 'r') as f:
            lines = f.readlines()
        params = {'source': ''.join(lines), 'return': return_vars}
        print('Execute script code of {}'.format(filename))
        print('Return variables: {}'.format(return_vars))
        print(20*'-')
        print(params['source'])
        print(20*'-')
        return self.exec(params)

    def exec(self, params={'source': ''}):
        response = requests.post(self.url, json=params, headers=self.headers)
        return encode(response)
