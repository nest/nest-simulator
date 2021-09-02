Contributing to NEST
====================

NEST draws its strength from the many people that use and improve it. We
are happy to consider your contributions (e.g., new models, bug or
documentation fixes) for addition to the official version of NEST.

Please familiarize yourself with our:

* :doc:`NEST Git workflow <development_workflow>`
* :doc:`C++ coding style guidelines <coding_guidelines_cpp>`
* :doc:`Documentation style guide <styleguide/styleguide>`
* :doc:`Developing NEST with IDEs guide <nest_with_ides>`

In order to make sure that the NEST Initiative can manage the NEST code base in the long term,
you need to send us a completed and signed
:download:`NEST Contributor Agreement <NEST_Contributor_Agreement.pdf>` to transfer your
copyright to the NEST Initiative before we can merge your pull request.

Report bugs and request features
--------------------------------

If you find an error in the code or documentaton or want to suggest a feature, submit
`an issue on GitHub <https://github.com/nest/nest-simulator/issues>`_.

Make sure to check that your issue has not already been reported there before creating a new one.

.. _edit_nest:

Change code or documentation
----------------------------

Interested in creating or editing documentation? Check out our :doc:`../documentation_workflow/index`.

For making changes to the PyNEST APIs, please see our :doc:`templates/pyapi_template`.

If you are a Vim user and require support for SLI files, please refer to our
:doc:`styleguides/vim_support_sli`.

An explanation of our continuous integration pipeline can be found under :doc:`ci`.

Contribute a Python example script
----------------------------------

If you have a Python example network to contribute, please refer to our
:doc:`templates/example_template` to ensure you cover the required information.

.. _review_guidelines:

Code review guidelines
----------------------

See :doc:`code_review_guidelines`.

Writing an extension module
---------------------------

See https://github.com/nest/nest-extension-module.

Writing neuron and synapse models
---------------------------------

We recommend writing new neuron and synapse models in `NESTML <https://nestml.readthedocs.io/>`_. It will generate C++
code and build a NEST extension module containing the model.

See also https://github.com/nest/nest-extension-module for details about the generated C++ code.

Have a question?
----------------

If you want to get in contact with us, see our :ref:`nest_community` page for ways you can reach us.
