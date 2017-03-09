#!/usr/bin/tclsh
# This profile includes all the rules for checking NEST
#
# Do not apply T011 (curly braces), since that can get confused
# by conditional code inclusion.

parameter=max-dirname-length=63
parameter=max-filename-length=63
parameter=max-path-length=1023
parameter=max-file-length=8192  

rule=F001
rule=F002
rule=L001
rule=L002
rule=L003
rule=L005
rule=L006
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
