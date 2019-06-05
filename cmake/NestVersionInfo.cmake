message("hello world")


macro(get_version_info)
    set(NEST_VERSION_SUFFIX "rc1")

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
    message("isRelease: ${isRelease}")
    if (isRelease STREQUAL "nest-")
        string(SUBSTRING "${NEST_VERSION_BRANCH}${versionsuffix}" 5 99999 NEST_VERSION)
    endif()

    set(NEST_VERSION_STRING "${NEST_VERSION_BRANCH}${versionsuffix}${githash}")
    unset(branchname)
    unset(versionsuffix)
    unset(githash)

endmacro()

get_version_info()
message("#define NEST_VERSION_STRING \"${NEST_VERSION_STRING}\"")
message("#define NEST_VERSION_BRANCH \"${NEST_VERSION_BRANCH}\"")
message("#define NEST_VERSION_SUFFIX \"${NEST_VERSION_SUFFIX}\"")
message("#define NEST_VERSION \"${NEST_VERSION}\"")
message("#define NEST_VERSION_GITHASH \"${NEST_VERSION_GITHASH}\"")

