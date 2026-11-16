.. _kernel_managers:

Kernel managers
===============

The NEST kernel is split into managers, each responsible for one part of the simulation: ``VPManager`` for threads
and virtual processes, ``ConnectionManager`` for connections, ``EventDeliveryManager`` for spike delivery, and so
on. The ``KernelManager`` coordinates them, for example by initializing and finalizing all managers in a fixed
order.

Accessing a manager
-------------------

Every manager, including the ``KernelManager`` itself, is accessed through the variable template
``kernel::manager``:

.. code-block:: cpp

   #include "vp_manager.h"

   const size_t tid = kernel::manager< VPManager >.get_thread_id();

``kernel_manager.h`` only forward-declares the manager classes. A file that uses a manager must therefore include
that manager's header, as described in the :ref:`include guidelines <include_guidelines>`.

Access is as cheap as accessing a global variable. The compiler knows the address of each manager at compile or
link time, so there is no pointer to follow.

Lifetime of the managers
------------------------

The managers are not ordinary global variables. Their construction is controlled explicitly:

1. Before ``main()`` runs, each manager has a block of memory reserved for it (``kernel::manager_storage< T >``),
   but no manager object exists yet.
2. ``init_nest()`` calls ``kernel::create_managers()`` as its very first step. This constructs all managers in
   their reserved memory, one after the other, in a fixed order.
3. The managers are never destroyed. Their destructors do not run when the program ends.

No manager may be used before ``kernel::create_managers()`` has run. Code that runs outside of ``init_nest()``,
for example a C++ unit test, has to call ``kernel::create_managers()`` itself. Calling it more than once is
harmless.

Why the construction is explicit
--------------------------------

The obvious implementation would be a global variable per manager:

.. code-block:: cpp

   template < class T >
   inline T manager;

The C++ standard does not specify in which order such variables are constructed: for variables that are
instantiated from a template, the order is unspecified even within a single source file. Variables are destroyed
in the reverse order of their construction, so the destruction order is unspecified, too. Managers use each other,
for instance the ``ModuleManager`` constructor writes log messages through the ``LoggingManager``. Depending on
compiler and linker, a manager could therefore be used before it was constructed or after it was destroyed. In
practice, this showed up as double frees when the program exited.

Explicit construction in ``kernel::create_managers()`` makes the order fixed and visible in the code. Not destroying
the managers removes the destruction order problem entirely. This is also how NEST behaved before: the
``KernelManager`` used to be allocated with ``new`` and was never deleted.

How it is implemented
---------------------

The implementation is in ``nestkernel/kernel_manager.h`` and ``nestkernel/kernel_manager.cpp``:

.. code-block:: cpp

   template < class T >
   union ManagerStorage
   {
     constexpr ManagerStorage() noexcept : unused_() {}
     ~ManagerStorage() {}

     char unused_;
     T object;
   };

   template < class T >
   constinit inline ManagerStorage< T > manager_storage {};

   template < class T >
   constinit inline T& manager = manager_storage< T >.object;

* A union only initializes one of its members. The constructor of ``ManagerStorage`` initializes the dummy member
  ``unused_``, so the memory for ``object`` is reserved but no manager is constructed.
* The constructor is ``constexpr`` and the variable is ``constinit``. The compiler therefore sets up the storage at
  compile time, so it does not take part in the unspecified construction order described above.
* ``manager< T >`` is a reference to the manager inside the storage. It is also fixed at compile time, which is why
  accessing a manager needs no pointer indirection.
* The destructor of ``ManagerStorage`` is empty. It does not call the destructor of ``object``, so managers are
  never destroyed.
* ``kernel::create_managers()`` constructs each manager with placement ``new`` in its storage:

  .. code-block:: cpp

     ::new ( static_cast< void* >( &kernel::manager_storage< T >.object ) ) T();

Rules for manager code
----------------------

* A manager's constructor may only use managers that ``kernel::create_managers()`` constructs before it.
* Do not rely on a manager's destructor being called. Release resources in ``finalize()`` instead.
* When you add a new manager, add it to ``kernel::create_managers()`` and to ``KernelManager::managers``, at the
  same position in both. The order in ``KernelManager::managers`` determines the order of ``initialize()`` and
  ``finalize()``.
