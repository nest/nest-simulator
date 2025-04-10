.. _module_manager:

Module Manager
=============

The ModuleManager is responsible for managing dynamic modules. Its primary functions include initializing and finalizing
modules, loading and unloading them dynamically, and handling their status. The class ensures that modules can be
safely installed and reinitialized, and it manages the search path for dynamic libraries, making it an essential
component for extending the simulator's functionality with external modules.


.. doxygenclass:: nest::ModuleManager
   :members:
