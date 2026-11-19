.. _test_uml:

Test uml page
-------------

Two diagrams should render on this page.

**Basic example:**


.. uml::

   Alice -> Bob: Hi!
   Alice <- Bob: How are you?


**C4 example:**


.. uml::

 !include <C4/C4_Context>
 Person(user, "C4 fans", "Hello to Sphinx!")
 System(nest, "NEST", "Simulates spiking neural networks")
 Rel(user, nest, "Uses")
