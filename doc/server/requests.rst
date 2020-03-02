Requests to NEST Server
-----------------------


Basic usage to send requests to NEST Server in Bash or Python.

Bash interface
==============

It requires curl package (`apt-get install curl`).

A simple request command with curl.

.. code-block:: bash

  curl localhost:5000


Send API request in JSON format

.. code-block:: bash

  curl -H "Content-Type: application/json" -d '{"model": "iaf_psc_alpha"}' localhost:5000/api/GetDefaults



Python interface
================

It requires requests package from pypi (`pip3 install requests`).

.. code-block:: python

  import requests
  requests.get('http://localhost:5000').json()

Send API request in JSON format

.. code-block:: python

  requests.post('http://localhost:5000/api/GetDefaults', json={'model': 'iaf_psc_alpha'}).json()


Send Python script for the simulation
=====================================

.. code-block:: python

  source = 'nest.ResetKernel()\npg = nest.Create("poisson_generator", params={"rate": 6500.})\nn = nest.Create("iaf_psc_alpha", 100)\nsd = nest.Create("spike_detector")\nnest.Connect(pg, n, syn_spec={\'weight\': 10.})\nnest.Connect(n, sd)\nnest.Simulate(1000.0)\n'
  requests.post('http://localhost:5000/exec', json={'source': source}).json()


Send script with returning data

.. code-block:: python

  source = 'a = nest.GetDefaults('iaf_psc_alpha')'
  requests.post('http://localhost:5000/exec', json={'source': source, 'return': 'a'}).json()
