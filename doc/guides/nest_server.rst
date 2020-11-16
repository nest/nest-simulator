NEST Server
===========


What is NEST Server?
--------------------

NEST Server is a server backend interacting with the NEST Simulator engine.
It enables users to retrieve data from NEST Simulator on other machines.

NEST Server was initially developed as a backend for NEST Desktop, a web-based GUI application for NEST Simulator, and
could be installed via ``pip``.
The idea was to design a client-server architecture concept for the NEST Desktop ecosystem.
However, the full operation of NEST Server requires PyNEST, the Python interface to NEST.
Previously, NEST Server was specially developed for NEST Desktop.

With growing interests in a server backend, NEST Server was migrated to the NEST Simulator code.
From NEST 3.0 onward, NEST Server is integrated in NEST Simulator source code.
To achieve this goal, the interface of NEST Server is generalized for a board application.

Under the hood, the RESTful API of NEST Server forwards command calls to NEST Simulator directly.
Additionally, NEST Server provides an execution of Python code script with NEST Simulator.



What is it good for?
--------------------

.. figure:: ../../_static/img/nest_server.png
    :align: center
    :alt: NEST Server concept
    :width: 240px

NEST Server can be considered as a backend coupling with NEST Simulator.
This backend can be deployed locally, on a remote machine or in a container.
The separation of the frontend and the backend ensures the independence of these systems.
It means that NEST Server can be deployed locally or in an encapsulated container such as Docker Engine.


Installation
------------

The source code of NEST Server is integrated in the NEST Simulator code.
It means that NEST Server will also be installed in the compilation progress of NEST Simulator.

.. tabs::

    .. tab:: Terminal (Linux/macOS)

        1. Install all requirements before starting a NEST Server instance:

        .. code-block:: sh

            apt-get update
            apt-get install build-essential python3-dev python3-pip
            python3 -m pip install RestrictedPython uwsgi flask flask-cors

        The ``nest-server`` command allows you to manage the NEST Server operations.

        2. To start NEST Server, enter:

        .. code-block:: sh

            nest-server start

        The command line options for ``nest-server start`` are:

        -d          Daemonize the server process
        -o          Print all output to both the console and log
        -h <HOST>   Use hostname/IP address <HOST> for the server [default: 127.0.0.1]
        -p <PORT>   Use port <PORT> for opening the socket [default: 5000]
        -u <UID>    Run the server under the user with ID <UID>


    .. tab:: Docker

        1. Pull an image or a repository from a registry:

        .. code-block:: sh

            docker pull nest-sim/nest:3.0

        2. Check `Docker Hub <https://hub.docker.com/r/nestsim/nest>`_ for more information of NEST container.

        3. Run a command in a new container for NEST Server:

        .. code-block:: sh

            docker run -it --rm -e LOCAL_USER_ID=`id -u $USER` -p 5000:5000 nestsim/nest:latest nest-server

        Usage:	docker run [OPTIONS] IMAGE [COMMAND] [ARG...]

        -i, --interactive                    Keep STDIN open even if not attached
        -t, --tty                            Allocate a pseudo-TTY
        --rm                                 Automatically remove the container when it exits
        -e, --env list                       Set environment variables
        -d, --detach                         Run container in background and print container ID
        -p, --publish list                   Publish a container's port(s) to the host

        For more information, check the `nest-docker <https://github.com/nest/nest-docker>`_ repository on GitHub.


Getting started
---------------

Upon starting NEST Server is started, you can send requests to it.
Here are instructions from different interfaces:

.. tabs::

    .. tab:: Web browser

        Many web browsers are able to retrieve data from NEST Server.
        It displays response data in the JSON format with successful GET requests.

        Once NEST Server is started, check if it is working.
          `<http://localhost:5000>`__.
        A list of call functions
            `<http://localhost:5000/api>`__
        A list of models
            `<http://localhost:5000/api/Models>`__
        Default values of neuron model: IAF cond alpha
            `<http://localhost:5000/api/GetDefaults?model=iaf_cond_alpha>`__

        .. note::

            Some browsers (e.g., Firefox) cannot display JSON data.
            Instead, they show syntax error because they cannot parse ``-infinite`` values (e.g. ``V_min`` of ``iaf_psc_alpha``).
            However, you still can view the raw data.

        You cannot send POST requests in the web browser.
        Please consider other interfaces for the POST requests method.

    .. tab:: Terminal (Linux/macOS)

        In your Terminal, ``curl`` is the preferred command line tool for transferring data to NEST Server.
        For more information, please visit the `curl website <https://curl.se/>`_.

        Here is a simple Terminal command:

        .. code-block:: sh

            curl localhost:5000

        NEST Server responds to the data in JSON format:

        .. code-block:: sh

            {"nest":"master@b08590af6"}

        You can retrieve data from the built-in functions of NEST Simulator via RESTful API.
        Get a list of functions:

        .. code-block:: sh

            curl localhost:5000/api

        .. note::

            You can display fancy outputs with ``curl -s`` and ``jq -r .``.

            A sample command line to show build-in functions:

            .. code-block:: sh

                curl -s localhost:5000/api | jq -r .

            For more detailed information, check the `jq documentation <https://stedolan.github.io/jq/>`_.

        Retrieve models of NEST Simulator:

        .. code-block:: sh

            curl localhost:5000/api/Models

        Retrieve selective models containing 'iaf':

        .. code-block:: sh

            curl localhost:5000/api/Models?sel=iaf


        **Advanced hacking**

        For POST requests to the NEST API Server, we recommend to use a Bash function:

        .. code-block:: sh

            #!/bin/bash
            NEST_API=localhost:5000/api

            nest-server-api() {
                if [ $# -eq 2 ]
                then
                    curl -H "Content-Type: application/json" -d "$2" $NEST_API/$1
                else
                    curl $NEST_API/$1
                fi
            }

        Now, we can send API requests to NEST Server using function ``nest-server-api`` function:

        .. code-block:: sh

            # Reset kernel
            nest-server-api ResetKernel

            # Create nodes
            nest-server-api Create '{"model": "iaf_psc_alpha", "n": 2}'
            nest-server-api Create '{"model": "poisson_generator", "params": {"rate": 6500.0}}'
            nest-server-api Create '{"model": "spike_recorder"}'

            # Connect nodes
            nest-server-api Connect '{"pre": [3], "post": [1,2], "syn_spec": {"weight": 10.0}}'
            nest-server-api Connect '{"pre": [1,2], "post": [4]}'

            # Simulate
            nest-server-api Simulate '{"t": 1000.0}'

            # Get events
            nest-server-api GetStatus '{"nodes": [4], "keys": "n_events"}'

        **Execute simulation script in NEST Server**

        You can send executable simulation code to ``localhost:5000/exec``.
        However, this approach might be challenged for the ``curl`` function, which could not fit in a single command
        line. We recommend to use the ``simulation_script.json`` file for ``curl``:

        .. code-block:: json

            {
              "source": "import nest\n# Reset kernel\nnest.ResetKernel()\n# Create nodes\nparams = {'rate': 6500.}\npg = nest.Create('poisson_generator', 1, params)\nneurons = nest.Create('iaf_psc_alpha', 1000)\nsr = nest.Create('spike_recorder')\n# Connect nodes\nnest.Connect(pg, neurons, syn_spec={'weight': 10.})\nnest.Connect(neurons[::10], sr)\n# Simulate\nnest.Simulate(1000.0)\n# Get events\nn_events = nest.GetStatus(sr, 'n_events')[0]\nprint('Number of events:', n_events)\n",
              "return": "n_events"
            }

        Then, execute ``curl`` to run simulation script from the ``simulation_script.json`` file:

        .. code-block:: sh

          curl -H "Content-Type: application/json" -d @simulation_script.json http://localhost:5000/exec


    .. tab:: Python

        Python provides the ``requests`` package for this purpose.
        For more information, check the `HTTP for Humans <https://requests.readthedocs.io/en/master/>`_ and
        `Requests <https://pypi.org/project/requests/>`_ pages.

        Install ``requests`` in your Terminal:

        .. code-block:: sh

            python3 -m pip install requests

        Now, you are able to send requests to NEST Server in the Python interface:

        .. code-block:: Python

            import requests
            requests.get('http://localhost:5000').json()

        Display a list of models:

        .. code-block:: Python

            requests.get('http://localhost:5000/api').json()

        Reset kernel in NEST engine (no response):

        .. code-block:: Python

            requests.get('http://localhost:5000/api/ResetKernel').json()

        Display a list of selective models containing 'iaf':

        .. code-block:: Python

            requests.post('http://localhost:5000/api/Models', json={"sel": "iaf"}).json()

        Create neurons in NEST engine and it returns a list of node ids:

        .. code-block:: Python

            neuron = requests.post('http://localhost:5000/api/Create', json={"model": "iaf_psc_alpha", "n": 100}).json()
            print(neuron)

        .. note::

            With this approach, we build NEST Server Client which is a Python class.
            See more info in NEST Server Client section below.

    .. tab:: JavaScript

        If you want to use the front end of a website, the script language is JavaScript.
        JavaScript provides libraries for sending requests to the server.
        Here, we create a basic HTML construction for GET requests using ``XMLHttpRequest``:

        .. code-block:: HTML

            <!DOCTYPE html>
            <html>
              <head>
                <meta charset="utf-8" />
              </head>
              <body>
                <script>
                  const xhr = new XMLHttpRequest();
                  xhr.open("GET", "http://localhost:5000");
                  xhr.addEventListener("readystatechange", () => {
                    if (xhr.readyState === 4) {                           // request done
                      console.log(xhr.responseText);
                    }
                  });
                  xhr.send(null);
                </script>
              </body>
            </html>

        **API requests**

        Here, we define a function with callback for GET requests in the previous HTML code:

        .. code-block:: JavaScript

            function getAPI(call, callback=console.log) {
                const xhr = new XMLHttpRequest();
                xhr.addEventListener("readystatechange", () => {
                    if (xhr.readyState === 4) {                           // request done
                        callback(xhr.responseText);
                    }
                });
                xhr.open("GET", "http://localhost:5000/api/" + call);     // send to api route of NEST Server
                xhr.send(null);
            }

        Now, we can send API-request to NEST Server:

        .. code-block:: JavaScript

            getAPI('Models');                                             // a list of models

        Next, we want to use API-requests with data.
        A POST request can handle the data in JSON-format.
        Thus, we define a function with callback for POST requests:

        .. code-block:: JavaScript

            function postAPI(call, data, callback=console.log) {
                const xhr = new XMLHttpRequest();
                xhr.addEventListener("readystatechange", () => {
                    if (xhr.readyState === 4) {                           // request done
                        callback(xhr.responseText);
                    }
                });
                xhr.open("POST", "http://localhost:5000/api/" + call);    // send to api route of NEST Server
                xhr.setRequestHeader('Access-Control-Allow-Headers', 'Content-Type');
                xhr.setRequestHeader('Content-Type', 'application/json');
                xhr.send(JSON.stringify(data));                           // serialize data
            }

        Here, we can send API-request to NEST Server:

        .. code-block:: JavaScript

            postAPI('GetDefaults', {"model": "iaf_psc_alpha"});           // default values of iaf_psc_alpha

        In summary, two functions were defined to retrieve data from NEST Simulator via RESTful API.

        **Send executable Python script**

        A code block for the complete simulation can be executed in NEST Server.
        For this purpose, we use the `exec` route of NEST Server.
        Here, we define a function with callback for POST requests to execute a script:

        .. code-block:: JavaScript

            function execScript(source, returnData="data", callback=console.log) {
                const data = {"source": source, "return": returnData};
                const xhr = new XMLHttpRequest();
                xhr.addEventListener("readystatechange", () => {
                    if (xhr.readyState === 4) {                           // request done
                        callback(xhr.responseText);
                    }
                });
                xhr.open("POST", "http://localhost:5000/exec");           // send to exec route of NEST Server
                xhr.setRequestHeader('Access-Control-Allow-Headers', 'Content-Type');
                xhr.setRequestHeader('Content-Type', 'application/json');
                xhr.send(JSON.stringify(data));                           // serialize data
            }

        Now, we can send an executable Python script to NEST Server:

        .. code-block:: JavaScript

            execScript("data = nest.GetDefaults('iaf_psc_alpha')");       // default values of iaf_psc_alpha

        An HTML client interfacing NEST Server API was prepared by Steffen Graber.
        You can find the sample source code `here <https://github.com/steffengraber/nest-jsclient>`_.


NEST Server Client
------------------

NEST Server Client is a Python class communicating with NEST Server.
This client-server architecture concept enables users to execute simulations on the client side without the need for a
NEST simulation engine.

API requests
~~~~~~~~~~~~

NEST Server Client has a dynamic method system which accepts the same method names as those in PyNEST.
It only forwards calls and its arguments towards NEST Simulator.
In the end, it looks like a typical simulation code for NEST Simulator.
Here, we show a comparison of codes for PyNEST and codes using NEST Server Client:

.. list-table::

    * - **Script in PyNEST** (``simulation_script.py``)
      - **Script via NEST Server Client**
    * - .. code-block:: Python

            import nest


            # Reset kernel
            nest.ResetKernel()

            # Create nodes
            params = {"rate": 6500.}
            pg = nest.Create("poisson_generator", 1, params)
            neurons = nest.Create("iaf_psc_alpha", 1000)
            sr = nest.Create("spike_recorder")

            # Connect nodes
            nest.Connect(pg, neurons, syn_spec={'weight': 10.})
            nest.Connect(neurons[::10], sr)

            # Simulate
            nest.Simulate(1000.0)

            # Get events
            n_events = nest.GetStatus(sr, 'n_events')[0]
            print('Number of events:', n_events)

      - .. code-block:: Python

            from NESTServerClient import NESTServerClient
            nsc = NESTServerClient()

            # Reset kernel
            nsc.ResetKernel()

            # Create nodes
            params = {"rate": 6500.}
            pg = nsc.Create("poisson_generator", 1, params)
            neurons = nsc.Create("iaf_psc_alpha", 1000)
            sr = nsc.Create("spike_recorder")

            # Connect nodes
            nsc.Connect(pg, neurons, syn_spec={'weight': 10.})
            nsc.Connect(neurons[::10], sr)

            # Simulate
            nsc.Simulate(1000.0)

            # Get events
            n_events = nsc.GetStatus(sr, 'n_events')[0]
            print('Number of events:', n_events)

Requests to execute scripts
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The NEST Server Client sends executable scripts to NEST Server with the ``exec_script`` method.

.. note::

    You do not need to import modules in code script.
    By default, only PyNEST modules are registered  during the starting progress of NEST Server.
    In case you want to work with other modules, see the section `Run with Python modules`_ below.

Here, you can see a simple code:

.. code-block:: Python

    from NESTServerClient import NESTServerClient
    nsc = NESTServerClient()
    response = nsc.exec_script("print('Hello world!')")
    print(response['stdout'])                                             # 'Hello world!'
    response = nsc.exec_script("models=nest.Models()", 'models')
    models = response['data']
    print(models)                                                         # a list of models

NEST Server Client is able to read scripts from files via the ``from_file`` method and then executes it on server side.
The code can be taken from a typical Python script (e.g. ``simulation_script.py`` in left column in Python section):

.. code-block:: Python

    from NESTServerClient import NESTServerClient
    nsc = NESTServerClient()
    response = nsc.from_file('simulation_script.py', 'n_events')
    n_events = response['data']
    print('Number of events:', n_events)


Limitations and security implications
-------------------------------------

The code execute function of Python comes with a security risk on the server side.
An unauthorized access with a Python script could corrupt the system.

To ensure keeping the server secure, we created a trusted environment with a limitation.
The limitation helps the server to control module imports.
Moreover `RestrictedPython`_ preserves a high security standard on the server side.

.. note::

    RESTful API is not affected from the restricted environment.

Run with Python modules
~~~~~~~~~~~~~~~~~~~~~~~

You are not able to import any modules during code execution.
When starting NEST Server, all registered modules are imported.
Here, we show codes to register more modules for code execution in NEST Server.
As an example, we want to run a script with NumPy:

.. code-block:: sh

    export NEST_SERVER_MODULES=nest,numpy
    nest-server start

Then, execute a code to create an array of NumPy and return it as list:

.. code-block:: Python

    from NESTServerClient import NESTServerClient
    nsc = NESTServerClient()
    response = nsc.exec_script("a = numpy.arange(10)", 'a')
    print(response['data'][::2])                                          # [0, 2, 4, 6, 8]

RestrictedPython
~~~~~~~~~~~~~~~~

RestictedPython is a tool helps defining a trusted environment and to execute untrusted code inside of it.
By default, NEST Server runs in a restricted environment with RestrictedPython.
However, some code might not work when restricted.
For more information, please visit the documentation of `RestrictedPython <https://restrictedpython.readthedocs.io>`_.


We also implemented an option to turn off the trusted environment.

.. warning::

    Without RestrictedPython, you should be aware of security risks to the server.

To run NEST Server without a restricted environment follow these steps:

.. code-block:: sh

    export NEST_SERVER_RESTRICTION_OFF=true
    nest-server start

And in Python interface:

.. code-block:: Python

    from NESTServerClient import NESTServerClient
    nsc = NESTServerClient()
    response = nsc.exec_script("print(nest.__version__)")
    print(response['stdout'])                                             # 'HEAD@ef42c5f2f'


Tools that use NEST Server
--------------------------

A variety of tools use NEST Server, including the
  - Backend for `NEST Desktop <https://nest-desktop.readthedocs.io>`_ (contact person: Sebastian Spreizer)
  - Interface for `NeuroRobotics Platform <https://neurorobotics.net/>`_ (contact person: Jochen Martin Eppler)
