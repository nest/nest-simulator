Getting Help
=================


Have a specific question or problem with NEST?
------------------------------------------------

* Check out the :doc:`troubleshooting section <troubleshooting>` for common issues.

If your question is not on there, ask our :doc:`Mailing List <community>`.

Getting help on the command line interface
-------------------------------------------

* The ``helpdesk()`` command will launch the documentation pages on your browser.
  See `Set up the integrated helpdesk`_ to specify the browser of your choice.

* To access the High-level Python API reference material you can use the commands:

    .. code-block:: python

       # list all functions and attributes
       dir(nest)

       # Get docstring for function in python
       help('nest.FunctionName')

       # or in ipython
       nest.FunctionName?

* To access a specific C++ or SLI reference page for an object, command or parameter you can use the command:

    .. code-block:: python

       nest.help('name')

Model Information
~~~~~~~~~~~~~~~~~~~

* To get a complete list of the models available in NEST type:

    .. code-block:: python

       nest.Models()

   * To get a list of only neuron models use:

    .. code-block:: python

       nest.Models(mtype='nodes', sel=None)

   * To get a list of only synapse models use:

    .. code-block:: python

       nest.Models(mtype='synapses', sel=None)

* To get details on model parameters and usage use:

    .. code-block:: python

       nest.help('model_name')


Set up the integrated helpdesk
--------------------------------

The command ``helpdesk`` needs to know which browser to launch in order
to display the help pages. The browser is set as an option of
``helpdesk``. Please see the file ``~/.nestrc`` for an example setting
``firefox`` as browser. Please note that the command ``helpdesk`` does
not work if you have compiled NEST with MPI support, but you have to
enter the address of the helpdesk (``file://$PREFIX/share/doc/nest(``)
manually into the browser. Please replace ``$PREFIX`` with the prefix
you chose during the configuration of NEST. If you did not explicitly
specify one, it is most likely set to ``/usr`` or ``/usr/local``
depending on what system you use.

