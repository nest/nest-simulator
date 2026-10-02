# -*- coding: utf-8 -*-
#
# test_issue_3744.py
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


import nest


def test_issue_3744():
    """
    ResetKernel() must reset data_path and data_prefix back to empty strings
    when the NEST_DATA_PATH/NEST_DATA_PREFIX environment variables are not
    set, instead of leaving the previously set values in place.
    """

    nest.ResetKernel()
    assert nest.get(["data_path", "data_prefix"]) == ("", "")

    nest.set(data_path=".", data_prefix="foo")
    assert nest.get(["data_path", "data_prefix"]) == (".", "foo")

    nest.ResetKernel()
    assert nest.get(["data_path", "data_prefix"]) == ("", "")
