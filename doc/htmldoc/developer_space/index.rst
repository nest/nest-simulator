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



Guidelines and workflows for developing NEST
---------------------------------------------



.. note::

   Most of the guides and workflows for contributing to NEST can be found in our :ref:`contribute` guide.



.. grid:: 1 1 2 2

   .. grid-item-card:: Reviewer guidelines

       * If you are requested to review a pull request, please check our :ref:`code_guidelines`

   .. grid-item-card:: CI Workflow

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

   workflows/*
   workflows/documentation_workflow/*
   guidelines/*
   guidelines/styleguide/styleguide
   guidelines/styleguide/vim_support_sli
   templates/*
   sli_docs/index
   cppcomments
