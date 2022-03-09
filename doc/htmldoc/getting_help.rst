.. _getting_help:

Getting help
============

Have a specific question or problem with NEST?
----------------------------------------------

Check out the :ref:`troubleshooting section <troubleshooting>` for
common issues.

If your question is not on there, you are welcome to subscribe to our
:ref:`Mailing List <community>` and ask.

Getting help on the command line interface
------------------------------------------

* The :py:func:`.helpdesk` command will launch the documentation pages on your browser.

* To access the High-level Python API reference material you can use the commands:

    .. code-block:: python

       # list all functions and attributes
       dir(nest)

       # Get docstring for function in Python ...
       help('nest.FunctionName')

       # ... or in IPython
       nest.FunctionName?

Model information
-----------------

* To get a list of available neuron models, use:

    .. code-block:: python

       nest.node_models

* To get a list of available synapse models, use:

    .. code-block:: python

       nest.synapse_models

* To get details on model equations and parameters, use:

    .. code-block:: python

       nest.help('model_name')

