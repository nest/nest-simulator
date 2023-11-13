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
 !define FONTAWESOME https://raw.githubusercontent.com/tupadr3/plantuml-icon-font-sprites/master/font-awesome-5
 !include FONTAWESOME/users.puml
 Person(user, "C4 fans", "Hello to Sphinx!", $sprite="users")
