Getting help
============

Have a specific question or problem with NEST?
----------------------------------------------

Check out the :doc:`troubleshooting section <troubleshooting>` for
common issues.

If your question is not on there, you are welcome to subscribe to our
:doc:`Mailing List <community>` and ask.

Getting help on the command line interface
------------------------------------------

* The ``helpdesk()`` command will launch the documentation pages on
  your browser.
  
* To access the High-level Python API reference material you can use
  the commands:

    .. code-block:: python

       # list all functions and attributes
       dir(nest)

       # Get docstring for function in Python ...
       help('nest.FunctionName')

       # ... or in IPython
       nest.FunctionName?

Model Information
-----------------

* To get a complete list of the models available in NEST type:

    .. code-block:: python

       nest.Models()

   * To get a list of only neuron models use:

    .. code-block:: python

       nest.Models(mtype='nodes')

   * To get a list of only synapse models use:

    .. code-block:: python

       nest.Models(mtype='synapses')

* To get details on model equations and parameters, use:

    .. code-block:: python

       nest.help('model_name')
