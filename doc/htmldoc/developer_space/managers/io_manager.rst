.. _io_manager:

IO Manager
==========


The IOManager in NEST coordinates input/output operations, managing data paths, file prefixes, and file-overwrite
settings. It registers and routes data between recording/stimulation devices and their respective backends (e.g.,
handling file I/O for spike data or parameter logging). It ensures devices communicate with the correct backend while
deferring logging responsibilities to the LoggingManager.


.. doxygenclass:: nest::IOManager
   :members:
