# -*- coding: utf-8 -*-
#
# versionchecker.py
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
Check that the Python compiletime and runtime versions match.

"""

v_major_mismatch = sys.version_info.major != 3
v_minor_mismatch = sys.version_info.minor != 13
if v_major_mismatch or v_minor_mismatch:
    msg = "Python runtime version does not match 'nest' compiletime version. " + "Please use Python 3.13."
    raise Exception(msg)
