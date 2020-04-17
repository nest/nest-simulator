# -*- coding: utf-8 -*-
#
# hl_api_client.py
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


class NESTServerClient(object):

    def __init__(self, host='localhost', port=5000):
        self.url_api = 'http://{}:{}/api/'.format(host, port)
        self.headers = {'Content-type': 'application/json', 'Accept': 'text/plain'}

    def _nest_server_api(self, call, params={}):
        response = requests.post(self.url_api + call, json=params, headers=self.headers)
        if response.ok:
            return response.json()
        elif response.status_code == 400:
            raise BadRequest(response.text)

    def __getattr__(self, name):
        def method(*args, **kwargs):
            kwargs.update({'args': args})
            return self._nest_server_api(name, kwargs)
        return method
