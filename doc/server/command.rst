Command API
-----------


The command nest-server provides options (Defaults for host=127.0.0.1, port=5000)

.. code-block:: bash

   nest-server <command> [-o] [-h <host>] [-p <port>] [-u <user>]

Showing usage for nest-server

.. code-block:: bash

   nest-server

Start NEST Server serving at default address.

.. code-block:: bash

   nest-server start

Stop NEST Server serving at default address.

.. code-block:: bash

   nest-server stop

List status of NEST Server serving at different addresses.

.. code-block:: bash

   nest-server status

Monitor the requests of NEST Server.

.. code-block:: bash

   nest-server log
