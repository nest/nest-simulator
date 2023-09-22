.. _developer_space:

Technical documentation
=======================

Here is all documentation pertaining to the development of NEST.
It is documentation for anyone needing to touch the code or documentation.

.. grid:: 3 

  .. grid-item-card:: 
       :link-type: ref
       :link: dev_install
       :class-card: sd-bg-success sd-text-white

       Install NEST from source


Contribute to NEST
------------------

For information on how to contribute to NEST whether creating issues, writing to the mailing list
or making changes to code or documentation, see our :ref:`contribute` guide.

Workflows and Guidelines
-------------------------

.. grid:: 1 1 2 2

   .. grid-item-card:: Workflows

       * New to git or need a refresher? See our :ref:`NEST git workflow <git_workflow>`
       * Check that documentation renders properly: See the :ref:`build documentation <doc_workflow>` guide for developer and user documentation
       * Here you can find details on our :ref:`CI workflow <cont_integration>`

   .. grid-item-card:: Guidelines

       * Follow the :ref:`C++ coding style guidelines <code_style_cpp>`
       * Review the :ref:`naming conventions for NEST <naming_conventions>`
       * :ref:`check_code` to ensure correct formatting
       * Review the :ref:`documentation style guide <doc_styleguide>`
       * If you are requested to review a pull request, please check our :ref:`code_guidelines`

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

   workflows/*
   workflows/documentation_workflow/*
   guidelines/*
   guidelines/styleguide/styleguide
   guidelines/styleguide/vim_support_sli
   templates/*
   sli_docs/index
