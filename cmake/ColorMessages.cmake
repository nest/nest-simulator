# ColorMessages.cmake
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

# This module defines functions to print warnings, errors and status with colors


string(ASCII 27 Esc)
set(Reset "${Esc}[m")
set(Bold "${Esc}[1m")
set(Red "${Esc}[31m")
set(Green "${Esc}[32m")
set(Blue "${Esc}[34m")
set(Cya "${Esc}[36m")
set(Magenta "${Esc}[35m")
set(Yellow "${Esc}[33m")
set(White "${Esc}[37m")
set(BoldRed "${Esc}[1;31m")
set(BoldGreen "${Esc}[1;32m")
set(BoldBlue "${Esc}[1;34m")
set(BoldCyan "${Esc}[1;36m")
set(BoldMagenta "${Esc}[1;35m")
set(BoldYellow "${Esc}[1;33m")
set(BoldWhite "${Esc}[1;37m")

function(print)      
    set(option HAS_COLOR)
    set(singleValued COLOR MODE TEXT)
    cmake_parse_arguments(MSG "${option}" "${singleValued}" "" ${ARGN})
    if (${MSG_HAS_COLOR})
        if ("${MSG_MODE}" STREQUAL "FATAL")
            message(FATAL_ERROR "${MSG_COLOR} ${MSG_TEXT} ${Reset}")
        elseif ("${MSG_MODE}" STREQUAL "WARNING")
            message(WARNING "${MSG_COLOR} ${MSG_TEXT}${Reset}")
        else()
            message(STATUS "${MSG_COLOR}${MSG_TEXT} ${Reset}") 
        endif()
    else()
        message(STATUS "${MSG_TEXT}") 
    endif()  
endfunction()


function(printWarning TEXT)
    print(HAS_COLOR MODE "WARNING" TEXT "Warning: ${TEXT}" COLOR ${Yellow})
endfunction()

function(printError TEXT)
    print(HAS_COLOR MODE "FATAL" TEXT "Error: ${TEXT}" COLOR ${Red})
endfunction()

function(printInfo TEXT)
    print(HAS_COLOR TEXT "Info: ${TEXT}" COLOR ${BoldGreen} )
endfunction()
