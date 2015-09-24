# -*- coding: utf-8 -*-
#
# hl_api_info.py
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

"""
Functions to get information on NEST.
"""

from .hl_api_helper import *

@check_stack
def sysinfo():
    """
    Print information on the platform on which NEST was compiled.
    """

    sr("sysinfo")


@check_stack
def version():
    """
    Return the NEST version.
    """

    sr("statusdict [[ /kernelname /version ]] get")
    return " ".join(spp())
    

@check_stack
def authors():
    """
    Print the authors of NEST.
    """

    sr("authors")


@check_stack
def helpdesk(browser="firefox"):
    """
    Open the NEST helpdesk in the given browser. The default browser is firefox.
    """
    
    sr("/helpdesk << /command (%s) >> SetOptions" % browser)
    sr("helpdesk")


@check_stack
def help(obj=None, pager="less"):
    """
    Show the help page for the given object using the given pager. The
    default pager is less.
    """

    if obj is not None:
        sr("/page << /command (%s) >> SetOptions" % pager)
        sr("/%s help" % obj)
    else:
        print("Type 'nest.helpdesk()' to access the online documentation in a browser.")
        print("Type 'nest.help(object)' to get help on a NEST object or command.")
        print()
        print("Type 'nest.Models()' to see a list of available models in NEST.")
        print()
        print("Type 'nest.authors()' for information about the makers of NEST.")
        print("Type 'nest.sysinfo()' to see details on the system configuration.")
        print("Type 'nest.version()' for information about the NEST version.")
        print()
        print("For more information visit http://www.nest-simulator.org.")


@check_stack
def get_verbosity():
    """
    Return verbosity level of NEST's messages.
    """
    
    sr('verbosity')
    return spp()


@check_stack
def set_verbosity(level):
    """
    Change verbosity level for NEST's messages. level is a string and
    can be one of M_FATAL, M_ERROR, M_WARNING, or M_INFO.
    """

    sr("%s setverbosity" % level)


@check_stack
def get_argv():
    """
    Return argv as seen by NEST. This is similar to Python sys.argv
    but might have changed after MPI initialization.
    """
    sr ('statusdict')
    statusdict = spp ()
    return statusdict['argv']


@check_stack
def message(level,sender,text):
    """
    Print a message using NEST's message system.
    """

    sps(level)
    sps(sender)
    sps(text)
    sr('message')
