.. _developer_space:

Contribute to NEST
==================

Here is all documentation pertaining to contributing to NEST.


.. note::

   For documentation relating to C++ source code, see the
   link to GitHubpages


.. _contribute:


NEST draws its strength from the many people that use and improve it. We
are happy to consider your contributions (e.g., new models, bug or
documentation fixes) for addition to the official version of NEST.

**Please familiarize yourself with our guides and workflows!**

Have a question, feature request or want to report an issue?
-------------------------------------------------------------

.. grid:: 1 1 2 2

    .. grid-item-card:: Mailing list

      Have a question or problem about NEST? Get help from the NEST community:
      use our :ref:`mailing list <mail_guidelines>`.

    .. grid-item-card:: Create a GitHub issue

      If you have a feature request, bug report or other issue, create
      an issue on GitHub using `the templates <https://github.com/nest/nest-simulator/issues/new/choose>`_

Contribute code or docs to the NEST project
-------------------------------------------

.. grid:: 1 1 2 2
    :gutter: 1

    .. grid-item-card:: Before you contribute

       * New to git or need a refresher? See our :ref:`NEST git workflow <git_workflow>`
       * :ref:`Install NEST from source <dev_install>`
       * :ref:`required_dev_tools`
       * Review the :ref:`naming conventions for NEST <naming_conventions>`
       * :ref:`Developing NEST with IDEs <nest_ides>`

    .. grid-item-card:: Contribute code, modules, models

       * Adding neuron or synapses models to NEST: Check out :doc:`NESTML <nestml:index>`:
         a modeling language supporting neuron and synapse specification, based on the syntax of Python.
       * Modifying or adding C++ code? see `GitHubpages link`
       * For making changes to the PyNEST APIs, see our :ref:`pyapi_template`
       * Writing an extension module? See :doc:`extmod:index`

    .. grid-item-card:: Contribute documentation

       * If you have a Python example network to contribute, please refer to our
         :ref:`pyexample_template`
       * Review the :ref:`documentation style guide <doc_styleguide>`.
       * If you do contribute neuron or synapse models to nest-simulator, then ensure the documentation for the model
         meets our criteria. For an example, see the model docs in the
         `header file <https://github.com/nest/nest-simulator/blob/main/models/iaf_psc_alpha.h>`_ for ``iaf_psc_alpha``.
       * Check that documentation renders properly: See the :ref:`build documentation steps <doc_workflow>`.


    .. grid-item-card:: Before we approve your contribution

       * In order that the NEST Initiative can manage the NEST code base in the long term,
         you need to transfer the copyright by sending us a completed and signed form to ``contact[at]nest-initiative.org``.

         :download:`NEST Contributor Agreement <static/NEST_Contributor_Agreement.pdf>`

       * Reviewers must be assigned to the pull-request and check the relevant changes. If you are requested to review a pull request, please
         see our :ref:`code_guidelines`.

       * Continuous integration is used to ensure that NEST runs as expected. All tests must pass before merging.
         Here you can find details on our :ref:`CI workflow <cont_integration>`.


.. toctree::
   :maxdepth: 1
   :hidden:
   :glob:

   workflows/*
   workflows/documentation_workflow/index
   guidelines/*
   guidelines/styleguide/styleguide
   templates/*
