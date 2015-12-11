# needs the arguments:
# -DDOC_DIR=...
# -DDATA_DIR=...
# -DHELPDIRS=...
# -DINSTALL_DIR=...

if(NOT CMAKE_CROSSCOMPILING)
  message("Generate help from these directories:\n${HELPDIRS}")

  # set environment vars
  set(ENV{SLIDOCDIR} "${DOC_DIR}")
  set(ENV{SLIDATADIR} "${DATA_DIR}")
  set(ENV{NESTRCFILENAME} "/dev/null")
  execute_process(
    COMMAND ${INSTALL_DIR}/bin/sli --userargs=${HELPDIRS} "${DATA_DIR}/sli/install-help.sli"
    RESULT_VARIABLE RET_VAR
    OUTPUT_FILE "install-help.log"
    ERROR_FILE  "install-help.log"
  )
  # unset environment vars
  unset(ENV{SLIDOCDIR})
  unset(ENV{SLIDATADIR})
  unset(ENV{NESTRCFILENAME)

  if(RET_VAR EQUAL 0)
    message("SUCCESS")
  else()
    message("ERROR: ${RET_VAR}")
  endif()

endif()
