# This awk script clears all entries in SLI_EXTRA_MODULE_CANDIDATES in configure.ac
# This yields a configure.ac that can be bootstrapped unconditionally.
#
# Run as 
#   gawk -f $scriptdir/clear_extra_modules.awk configure.ac > configure.ac.tmp \
#     && mv configure.ac.tmp configure.ac

BEGIN { drop_lines=0; }

# clear module candidates, leaving mechanism intact
/SLI_NONPUBLIC_DYNMODULES=/ { 
  print "SLI_NONPUBLIC_DYNMODULES=\"\"";
  next;
}

# drop lines marked for removal
/## BEGIN CUT FROM RELEASE -------------- DO NOT EDIT THIS LINE/ { drop_lines=1; }
/## END   CUT FROM RELEASE -------------- DO NOT EDIT THIS LINE/ { drop_lines=0; next; }

# INSERT line is always kept, even if we are in drop mode
/## INSERT CODE FOR CANDIDATES HERE ----- DO NOT EDIT THIS LINE/ { print; next; }

{ if ( drop_lines == 0 ) print; }

