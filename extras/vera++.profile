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

rule=F001
rule=L001
rule=L002
rule=L003
rule=L005
rule=T001
rule=T002
rule=T004
rule=T005
rule=T006
rule=T007
rule=T010
rule=T012
rule=T013
rule=T017
rule=T018
rule=T019
