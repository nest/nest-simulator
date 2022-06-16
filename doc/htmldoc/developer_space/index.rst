.. _developer_space:

Developer space
===============

Here is all documentation pertaining to the development of NEST.
It is documentation for anyone needing to touch the code or documentation.

.. grid:: 3 

  .. grid-item-card:: 
       :link-type: ref
       :link: dev_install
       :class-card: sd-bg-success sd-text-white

       Install NEST from source



.. _contribute:

Contribute to NEST
------------------

NEST draws its strength from the many people that use and improve it. We
are happy to consider your contributions (e.g., new models, bug or
documentation fixes) for addition to the official version of NEST.

Please familiarize yourself with our guides and workflows:



.. grid:: 1 1 2 2

    .. grid-item-card:: Mailing list

      Have a question or problem about NEST? Get help from the NEST community:
      use our :ref:`mailing list <mail_guidelines>`.

    .. grid-item-card:: Create a GitHub issue

      If you have a feature request, bug report or other issue, create
      an issue on GitHub using `the templates <https://github.com/nest/nest-simulator/issues/new/choose>`_


.. grid:: 1 1 2 2

    .. grid-item-card:: Contribute code

       * New to git or need a refresher? See our :ref:`NEST git workflow <git_workflow>`
       * Follow the :ref:`C++ coding style guidelines <code_style_cpp>`
       * Review the :ref:`naming conventions for NEST <naming_conventions>`
       * Writing an extension module? See :doc:`extmod:index`
       * :ref:`check_code` to ensure correct formatting

    .. grid-item-card:: Contribute documentation

       * Review the :ref:`documentation style guide <doc_styleguide>`
       * For making changes to the PyNEST APIs, see our :ref:`pyapi_template`
       * If you have a Python example network to contribute, please refer to our
         :ref:`pyexample_template`
       * Check that documentation renders properly: See the :ref:`build documentation <doc_workflow>` guide for developer and user documentation

.. note:: Adding models to NEST

    If you are looking at creating a new model, please check out :doc:`NESTML <nestml:index>`:
    a modeling language supporting neuron and synapse specification, based on the syntax of Python.

In order to make sure that the NEST Initiative can manage the NEST code base in the long term,
you need to send us a completed and signed
:download:`NEST Contributor Agreement <static/NEST_Contributor_Agreement.pdf>` to transfer your
copyright to the NEST Initiative before we can merge your pull request.

----

Developer guides
----------------

.. grid:: 1 1 2 2

    .. grid-item-card:: Reviewer guidelines

        If you are requested to review a pull request, please
        check our :ref:`code_guidelines`


    .. grid-item-card::  Continuous integration

        * Here you can find details on our :ref:`CI workflow <cont_integration>`

.. grid:: 1 1 2 2

    .. grid-item-card:: SLI documentation
        :link: sli_doc
        :link-type: ref


    .. grid-item-card:: C++ documentation

        * see :ref:`devdoc_workflow`

.. grid:: 1 1 2 2

    .. grid-item-card:: Helpful guides

       Here are a few miscellaneous guides that you might find useful:


       * :ref:`Developing NEST with IDEs <nest_ides>`

       * :ref:`vim_sli`

.. toctree::
   :maxdepth: 1
   :hidden: 
   :glob:

   ../installation/linux_install
   workflows/*
   workflows/documentation_workflow/*
   guidelines/*
   guidelines/styleguide/styleguide
   guidelines/styleguide/vim_support_sli
   templates/*
   sli_docs/index
   *
