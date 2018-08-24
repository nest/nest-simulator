# -*- coding: utf-8 -*-
#
# helpers.py
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

import re
import os
import shutil
import errno


def cut_it(separator, text):
    """
    Cut it
    ------

    Cut text by separator.
    """
    if separator:
        return re.split(separator, text)
    else:
        return [separator, text]


def check_ifdef(item, filetext, docstring):
    """
    Check the 'ifdef' context
    -------------------------

    If there is an 'ifdef' requirement write it to the data.
    """
    ifdefstring = r'(\#ifdef((.*?)\n(.*?)\n*))\#endif'
    require_reg = re.compile('HAVE\_((.*?)*)\n')
    # every doc in an #ifdef
    ifdefs = re.findall(ifdefstring, filetext, re.DOTALL)
    for ifitem in ifdefs:
        for str_ifdef in ifitem:
            initems = re.findall(docstring, str_ifdef, re.DOTALL)
            for initem in initems:
                if item == initem:
                    features = require_reg.search(str_ifdef)
                    return features.group()


def makedirs(path):
    """
    Forgiving version of os.makedirs, emulating the behavior of the
    shell command 'mkdir -p'. The function tries to create the
    directory at the given path including all subdirectories and
    returns silently if the directory already exists.
    """
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno != errno.EEXIST or not os.path.isdir(path):
            raise


def create_helpdirs(path):
    """
    Create the directories for the help files.
    """
    makedirs(os.path.join(path, 'sli'))
    makedirs(os.path.join(path, 'cc'))


def delete_helpdir(path):
    """
    Delete the directories for the help files.
    """
    try:
        shutil.rmtree(path)
    except OSError as exc:
        if exc.errno != errno.ENOENT:
            raise


def help_generation_required():
    """
    Check whether help extraction/installation is required.

    The check is based on the setting of the environment variable
    NEST_INSTALL_NODOC. If the variable is set, this function returns
    False, if not, it returns True.

    A corresponding message is printed if print_msg is True. The
    message is omitted if print_msg is set to False.
    """

    blue = "\033[94m"
    noblue = "\033[0m"

    if "NEST_INSTALL_NODOC" in os.environ:
        msg = "Not extracting help information for target 'install-nodoc'."
        print(blue + msg + noblue)
        return False
    else:
        msg = "Extracting help information. This may take a little while."
        print(blue + msg + noblue)
        return True
