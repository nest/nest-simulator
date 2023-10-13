:orphan:

Test page
---------

.. uml::

   Alice -> Bob: Hi!
   Alice <- Bob: How are you?


.. uml::

 !include <C4/C4_Context>
 !define FONTAWESOME https://raw.githubusercontent.com/tupadr3/plantuml-icon-font-sprites/master/font-awesome-5
 !include FONTAWESOME/users.puml
 Person(user, "C4 fans", "Hello to Sphinx!", $sprite="users")
