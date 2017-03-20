#!/usr/bin/tclsh
# This profile includes all the rules for checking NEST
#
# Do not apply T011 (curly braces), since that can get confused
# by conditional code inclusion.
#
# Do not apply F002 (file name length limits), since some benign model file
# names then become illegal; Vera++ 1.2.1 does not support parameters in
# profile files, so we cannot extend file name length limits here. 
#
# Do not apply L006 (limit on file length), since some legacy sli code 
# is too long; see also F002.

set rules {
  F001
  L001
  L002
  L003
  L005
  T001
  T004
  T005
  T006
  T007
  T010
  T012
  T013
  T017
  T018
  T019
}
