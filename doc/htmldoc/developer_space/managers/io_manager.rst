.. _io_manager:

IO Manager
==========

The IOManager in NEST coordinates input/output operations, managing data paths, file prefixes, and file-overwrite
settings. It registers and routes data between recording/stimulation devices and their respective backends (e.g.,
handling file I/O for spike data or parameter logging). It ensures devices communicate with the correct backend while
deferring logging responsibilities to the LoggingManager.


.. mermaid::

   classDiagram
    class ManagerInterface {
        +virtual ~ManagerInterface()
        // Other ManagerInterface methods/attributes
    }

    class IOManager {
        <<extends ManagerInterface>>
        +IOManager()
        +~IOManager()
        +check_recording_backend_device_status(Name, DictionaryDatum&)
        +get_recording_backend_device_defaults(Name, DictionaryDatum&)
        +get_recording_backend_device_status(Name, RecordingDevice&, DictionaryDatum&)
        +enroll_stimulator(Name, StimulationDevice&, DictionaryDatum&)
        +set_recording_value_names(Name, RecordingDevice&, vector<Name>&, vector<Name>&)
        +get_status(DictionaryDatum&)
        +set_status(DictionaryDatum&)
        +register_recording_backend<RecordingBackendT>(Name)
        +register_stimulation_backend<StimulationBackendT>(Name)
        -data_path_ string
        -data_prefix_ string
        -overwrite_files_ bool
        -recording_backends_ map<Name, RecordingBackend*>
        -stimulation_backends_ map<Name, StimulationBackend*>
        // Other private methods like post_step_hook(), prepare(), etc.
    }

    IOManager --|> ManagerInterface : Inherits
    IOManager "1" *-- "n" RecordingBackend : manages via recording_backends_
    IOManager "1" *-- "n" StimulationBackend : manages via stimulation_backends_


.. doxygenclass:: nest::IOManager
   :members:
