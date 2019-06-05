
macro(get_version_info)
   execute_process(
        COMMAND "git" "rev-parse" "--short" "HEAD"
        OUTPUT_VARIABLE NEST_VERSION_GITHASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    execute_process(
        COMMAND "git" "rev-parse" "--abbrev-ref" "HEAD"
        OUTPUT_VARIABLE NEST_VERSION_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    if (NEST_VERSION_SUFFIX)
        set(versionsuffix "-${NEST_VERSION_SUFFIX}")
    endif()

    if (NEST_VERSION_GITHASH)
        set(githash "@${NEST_VERSION_GITHASH}")
    endif()

    if (NOT NEST_VERSION_BRANCH)
        set(NEST_VERSION_BRANCH "UNKNOWN")
    endif()

    string(SUBSTRING "${NEST_VERSION_BRANCH}" 0 5 isRelease)
    if (isRelease STREQUAL "nest-")
        string(SUBSTRING "${NEST_VERSION_BRANCH}${versionsuffix}" 5 99999 NEST_VERSION)
    else()
        set(NEST_VERSION "${NEST_VERSION_BRANCH}${versionsuffix}")
    endif()

    set(NEST_VERSION_STRING "${NEST_VERSION_BRANCH}${versionsuffix}${githash}")
    unset(branchname)
    unset(versionsuffix)
    unset(githash)

endmacro()
