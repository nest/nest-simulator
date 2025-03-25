.. _logging_manager:

Logging Manager
===============

The LoggingManager in NEST centralizes logging by filtering messages based on severity levels and delivering them to
registered clients, ensuring thread safety with OpenMP. It enforces proper parameter usage by checking dictionary
entries and supports customizable output through callback functions, such as console logging or external tools.


.. mermaid::

   classDiagram
    class ManagerInterface

    class LoggingManager {
        +LoggingManager()
        +~LoggingManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +set_status(params: DictionaryDatum&): void
        +get_status(status: DictionaryDatum&): void
        +register_logging_client(callback: deliver_logging_event_ptr): void
        +set_logging_level(level: severity_t): void
        +get_logging_level(): severity_t
        +publish_log(severity: severity_t, function: string, message: string, file: string, line: size_t): void
        +all_entries_accessed(dictionary: Dictionary, where: string, msg1: string, msg2: string, file: string, line: size_t): void
        -deliver_logging_event_(event: LoggingEvent const&): void
        -default_logging_callback_(event: LoggingEvent const&): void
        -client_callbacks_: vector<deliver_logging_event_ptr>
        -logging_level_: severity_t
        -dict_miss_is_error_: bool
    }

    LoggingManager --|> ManagerInterface: extends


.. doxygenclass:: nest::LoggingManager
