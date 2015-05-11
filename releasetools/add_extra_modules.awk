# This awk script adds extra candidate module directories to configure.ac
# The directories must be given as awk variable MODNAMES.
# Run as
#
#  cp configure.ac configure.ac.orig
#  gawk -v NEWMOD="joe doe" -f releasetools/add_extra_modules.awk configure.ac > configure.ac.new
#  mv configure.ac.new configure.ac
#
# Then bootstrap and configure.

BEGIN { 
  split(NEWMOD, MODNAMES, "[[:blank:]]+");
}

/SLI_EXTRA_MODULE_CANDIDATES=\".*\"/ { 
  split($0, parts, "\"");
  print "  SLI_EXTRA_MODULE_CANDIDATES=\"" parts[2] " " NEWMOD "\"";
  next;
}

/## INSERT CODE FOR CANDIDATES HERE ----- DO NOT EDIT THIS LINE/ {
  print $0
  for ( nm in MODNAMES ) {
    print "      " MODNAMES[nm] ")"
    print "        AC_CONFIG_FILES(" MODNAMES[nm] "/Makefile)"
    print "      ;;"
  }
  next;
}

{ print }

