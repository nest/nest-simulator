Getting started
---------------

Start NEST Server:

.. code-block:: bash

   nest-server start


Alternatively, NEST Server can be served in Python interface, e.g. IPython, Jupyter.
Note that it works with Flask 0.12.5 or older (`pip3 install flask==0.12.5`).

.. code-block:: python

  import nest
  nest.server.app.run()
