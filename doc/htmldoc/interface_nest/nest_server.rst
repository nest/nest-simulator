.. _nest_server:

NEST Server
===========

What is NEST Server?
--------------------

NEST Server enables users to interact with the NEST simulation engine via a RESTful API. Using this approach, you can
perform the same basic operations as with :ref:`PyNEST <pynest_api>`, but instead of doing so by directly importing the
``nest`` module, all commands, including their arguments and result data, are channeled through HTTP requests and
responses over a TCP/IP connection.

The main advantage of using NEST in this way is that it decouples the *simulation backend* in the form of the NEST
simulation kernel from the *frontend*, i.e., the code that controls the simulation. In such a scenario, only the backend
(the *server*) depends on NEST, while the frontend could really be anything that can talk HTTP. Under the hood, NEST
Server forwards the commands it receives to PyNEST, and sends back the result data in response packets.

NEST Server was initially developed as a backend for :doc:`NEST Desktop <desktop:index>`, a web-based graphical frontend
for NEST. With growing interest in a more general server backend for NEST, the functionality of the original NEST Server
was extended to accommodate for this broader range of application.

Quickstart
----------

For native installations, the package can be simply installed via ``pip``::

  pip install nest-server

All NEST Server operations are managed using the ``nest-server`` command that can be run directly:

.. code-block:: text

   nest-server start

.. note::

   For more information, please read the documentation on `NEST Server <https://github.com/nest/nest-server>` repository.
