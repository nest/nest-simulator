.. _include_guidelines:

Include guidelines
==================

NEST follows the *include what you use* principle: every source and header file includes the headers that declare
everything it uses directly, and nothing more. A file must not rely on a header being pulled in indirectly through
another header.

This keeps the include tree small and makes headers independent of each other. Removing an include from one header
then only affects that header, not every file that happened to depend on it indirectly.

Managers
--------

``kernel_manager.h`` only forward-declares the managers. Including it gives you access to the ``kernel::manager``
template, but not to the manager classes themselves. If a file uses a manager, for example
``kernel::manager< VPManager >.get_thread_id()``, it must include the corresponding header, here ``vp_manager.h``.
See :ref:`kernel_managers` for how the managers are constructed.

Headers versus source files
---------------------------

* Prefer forward declarations in header files where a complete type is not needed, for example for references,
  pointers, and function parameters.
* Include the full headers in the source file (``.cpp``) or in the ``*_impl.h`` file that needs the complete type.
* Performance-critical functions are an exception to keeping definitions out of headers, see
  :ref:`performance_guidelines`.

Checking includes
-----------------

The `include-what-you-use <https://github.com/include-what-you-use/include-what-you-use>`_ tool reports missing and
superfluous includes. It was used to clean up the NEST code base and is the recommended way to check new code.
