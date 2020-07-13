Getting help
============

The NEST documentation has grown tremendously over the years. The following material exists:

* `Read the Docs <https://nest-simulator.readthedocs.io/en/stable/>`_: This is the user 
  documentation of NEST. Besides providing a reference, it contains various NEST tutorials 
  and guides. Please make sure you select the documentation version appropriate for your 
  NEST version (as determined by ``nest.version()`` in Python). The "latest" version on 
  Read the Docs mirrors the latest version on GitHub.
* `Doxygen <https://www.doxygen.nl/index.html>`_: This is the developer documentation of 
  NEST. It can be built according to 
  :doc:`these instructions <documentation_workflow/developer_documentation_workflow>`. As 
  the reference files of the help generator documentation are also compiled from Doxygen 
  comments, it contains most of the help generator content as well. Of course, you should 
  develop for NEST based on the latest development version.
* `Help generator <https://www.nest-simulator.org/helpindex/>`_: This homegrown tool buils 
  legacy user user documentation that you can access by calling ``nest.helpindex()`` or 
  ``nest.help("term")`` in Python. For the latest stable version of NEST, you can also find 
  it `online <https://www.nest-simulator.org/helpindex/>`_. This documentation describes 
  SLI, a programming language used to interact with the NEST kernel.
* `Developer Space <https://nest.github.io/nest-simulator/>`_: This is a developer manual, 
  explaining the development infrastructure and workflow of NEST.
  
Further NEST-related content and discussions can be found on our:

* `GitHub page <https://github.com/nest/nest-simulator/>`_
* `Mailing list archive <https://www.nest-simulator.org/mailinglist/hyperkitty/list/users@nest-simulator.org/>`_
* `NEST Simulator website <https://nest-simulator.org>`_
* `NEST Initiative webpage <https://nest-initiative.org>`_

If any of the documentation is unclear, please submit an
`Issue <https://github.com/nest/nest-simulator/issues/new?assignees=&labels=&template=documentation_improvement.md&title=>`_ on
GitHub and check out our :doc:`User documentation workflow <documentation_workflow/user_documentation_workflow>`.


Have a specific question or problem with NEST?
----------------------------------------------

* Check out the :doc:`troubleshooting section <troubleshooting>` for common issues.

If your question is not on there and you can't find it using a search engine, ask our :doc:`Mailing List <community>`.

Getting help on the command line interface
------------------------------------------

* The ``nest.helpdesk()`` command will launch the ``help_generator``-generated documentation pages on your browser.
  See `Set up the integrated helpdesk`_ to specify the browser of your choice.

* The reference of the high-level Python API can be found `here <https://nest-simulator.readthedocs.io/en/stable/ref_material/pynest_apis.html>`_. The same content can be accessed via the docstring help system built into Python:

    .. code-block:: python
    
       import nest
       
       # list all functions, classes and attributes
       dir(nest)

       # Get docstring for function or class in Python
       help(nest.FunctionOrClassName)

       # A slightly more comfortable variant in IPython/Jupyter
       nest.FunctionOrClassName?
       

* To access a specific C++ or SLI reference page for an object, command or parameter (TODO currently not models - are models objects?) (TODO also not true for at least some parameters - e.g. no tau_minus_triplet entry), you can use the command:

    .. code-block:: python

       nest.help('name')

Model and Connection Information
-----------------
* To get a complete list of the models (simulated neural network components) available in NEST type:

    .. code-block:: python

       nest.Models()

   * To get a list of only neuron models use:

    .. code-block:: python

       nest.Models(mtype='nodes', sel=None) # lists only neuron models

   * To get a list of only synapse models use:

    .. code-block:: python

       nest.Models(mtype='synapses', sel=None) # lists only synapse models

* To get a list of available connection rules use:
   
    .. code-block:: python
    
       nest.ConnectionRules()

A directory of NEST models, with their reference documentation, is available `here <https://nest-simulator.readthedocs.io/en/stable/models/index.html>`_. Currently, this documentation of models generally **doesn't** contain a full description of the status dictionary (containing parameters and other attributes TODO the relation between parameters and status should be explained. Can one get rid of one of these words in the documentation? It is another source of confusion) in tabular form. To get a complete status dictionary (TODO really? or only parameters?) with default values for a model, e.g. "iaf_psc_alpha", use:

    .. code-block:: python
    
       nest.GetDefaults("iaf_psc_alpha") # returns a dictionary with the default status dictionary

If you want to know what a particular entry in that dictionary means and can't find it in the documentation, try your luck with Google or look in the source code.

* To get details on model parameters and usage use: (TODO This currently doesn't work for models. Bug or feature?)

    .. code-block:: python

       nest.help('model_name')

Set up the integrated helpdesk
------------------------------

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
