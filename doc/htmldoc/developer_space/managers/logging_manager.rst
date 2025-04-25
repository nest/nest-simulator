.. _logging_manager:

Logging Manager
===============

The LoggingManager in NEST centralizes logging by filtering messages based on severity levels and delivering them to
registered clients, ensuring thread safety with OpenMP. It enforces proper parameter usage by checking dictionary
entries and supports customizable output through callback functions, such as console logging or external tools.



.. doxygenclass:: nest::LoggingManager
   :members:
