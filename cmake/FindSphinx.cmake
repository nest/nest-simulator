# Based on information found on https://www.vortech.nl/en/integrating-sphinx-in-cmake

include(FindPackageHandleStandardArgs)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
if( Python_FOUND )
    get_filename_component(_PYTHON_DIR "${Python_EXECUTABLE}" DIRECTORY)
    set(
        _PYTHON_PATHS
        "${_PYTHON_DIR}"
        "${_PYTHON_DIR}/bin"
        "${_PYTHON_DIR}/Scripts"
        )
endif()

function(check_sphinx validator_result_var item)
  execute_process(COMMAND "${item}" "--version" ERROR_VARIABLE _err)
  if(_err)
    set(${validator_result_var} FALSE PARENT_SCOPE)
  endif()
endfunction()

find_program(
    SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build.exe
    HINTS ${_PYTHON_PATHS}
    VALIDATOR check_sphinx
    )

mark_as_advanced(SPHINX_EXECUTABLE)

find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
