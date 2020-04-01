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
from werkzeug.exceptions import BadRequest, NotFound
import nest


class NESTServerClient(object):

    def __init__(self, host='localhost', port=5000):
        self.url_api = 'http://{}:{}/api/'.format(host, port)
        self.headers = {'Content-type': 'application/json', 'Accept': 'text/plain'}

    def _nest_server_api(self, call, params={}):
        response = requests.post(self.url_api + call, json=params, headers=self.headers)
        if response.ok:
          return response.json()
        else:
          if response.status_code == 400:
              raise BadRequest(response.text)
          elif response.status_code == 404:
              raise NotFound(response.text)
          elif response.status_code == 409:
              raise nest.kernel.NESTError(response.text)

    def __getattr__(self, name):
        def method(**kwargs):
            return self._nest_server_api(name, kwargs)
        return method


'''
class Client(object):

    def __init__(self, url='http://localhost:5000/api/'):
        if url:
            self._url = url

        # for func in dir(nest):
        setattr(self.__class__, 'func', staticmethod(self._func))

    def _func(self):
        frame = inspect.currentframe()
        print(inspect.getframeinfo(frame))

    def _get(self, call):
        req = requests.get(self._url + call).json()
        response = req['response']
        return response['data']

    def _post(self, call, *args, **kwargs):
        print(__name__)
        req = requests.post(self._url + call, json=kwargs).json()
        response = req['response']
        return response['data']

    #
    # def Connect(self, pre, post, conn_spec=None, syn_spec=None, return_synapsecollection=False):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('Connect', **kwargs)
    #
    # def CopyModel(self, existing, new, params=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('CopyModel', **kwargs)
    #
    # def Create(self, model, n=1, params=None, positions=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('Create', **kwargs)
    #
    # def GetConnections(self, source=None, target=None, synapse_model=None, synapse_label=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('GetConnections', **kwargs)
    #
    # def GetDefaults(self, model, keys=None, output=''):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('GetDefaults', **kwargs)
    #
    # def GetKernelStatus(self, keys=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('GetKernelStatus', **kwargs)
    #
    # def GetStatus(self, nodes, keys=None, output=''):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('GetStatus', **kwargs)
    #
    # def Models(self, mtype='all', sel=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('Models', **kwargs)
    #
    # def ResetKernel(self):
    #     return self._get('ResetKernel')
    #
    # def SetDefaults(self, model, params, val=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('SetDefaults', **kwargs)
    #
    # def SetKernelStatus(self, params):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('SetKernelStatus', **kwargs)
    #
    # def SetStatus(self, nodes, params, val=None):
    #     kwargs = locals()
    #     kwargs.pop('self')
    #     return self._post('SetStatus', **kwargs)


if __name__ == '__main__':
    client = Client()
'''
