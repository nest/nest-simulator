# needs the arguments:
# -DDOC_DIR=...
# -DDATA_DIR=...
# -DSOURCE_DIR=...
# -DINSTALL_DIR=...

if(NOT CMAKE_CROSSCOMPILING)
  file(REMOVE_RECURSE "${DOC_DIR}/help")
  file(MAKE_DIRECTORY "${DOC_DIR}/help")

  # TODO limit to used source folders
  # base dirs + dirs/sli
  file(GLOB children ${SOURCE_DIR}/*)
  set(HELPDIRS "")
  foreach(child ${children})
    if(IS_DIRECTORY ${child})
      set(HELPDIRS "${HELPDIRS}${child}:")
      # Automatically include sli directory if it exists
      if(IS_DIRECTORY ${child}/sli)
        set(HELPDIRS "${HELPDIRS}${child}/sli:")
      endif()
    endif()
  endforeach()

  # testsuite dirs
  file(GLOB children ${SOURCE_DIR}/testsuite/*/)
  foreach(child ${children})
    if(IS_DIRECTORY ${child})
      set(HELPDIRS "${HELPDIRS}${child}:")
    endif()
    if(IS_DIRECTORY ${child}/pass)
      set(HELPDIRS "${HELPDIRS}${child}/pass:")
    endif()
    if(IS_DIRECTORY ${child}/fail)
      set(HELPDIRS "${HELPDIRS}${child}/fail:")
    endif()
  endforeach()

  message("Generate help from these directories:\n${HELPDIRS}")

  # set environment vars
  set(ENV{SLIDOCDIR} "${DOC_DIR}")
  set(ENV{SLIDATADIR} "${DATA_DIR}")
  set(ENV{NESTRCFILENAME} "/dev/null")
  execute_process(
    COMMAND ${INSTALL_DIR}/bin/sli --userargs="${HELPDIRS}" ${SOURCE_DIR}/lib/sli/install-help.sli
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
