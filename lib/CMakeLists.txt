message("Hi from lib...")

set( LIB_DATA 
  sli/FormattedIO.sli
  sli/arraylib.sli
  sli/debug.sli
  sli/helpinit.sli
  sli/install-help.sli
  sli/librandom.sli
  sli/library.sli
  sli/mathematica.sli
  sli/misc_helpers.sli
  sli/oosupport.sli
  sli/processes.sli
  sli/ps-lib.sli
  sli/regexp.sli
  sli/sli-init.sli
  sli/typeinit.sli
  sli/nest-init.sli
  sli/unittest.sli
  sli/filesystem.sli
  )

set( LIB_EXTRA_DIST README.md )

install(DIRECTORY sli
  DESTINATION ${INSTALL_DATA_DIR})
