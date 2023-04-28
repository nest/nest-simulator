# -*- coding: utf-8 -*-
#
# test_rows_cols_pos.py
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
import pytest

# TODO: This test looks useless!


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def test_correct_shape():
    nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([1, 1]))


def test_correct_position():
    nest.Create("iaf_psc_alpha", positions=nest.spatial.free(pos=[[0, 0]], extent=[1.0, 1.0]))


def test_invalid_shape():
    with pytest.raises(Exception):
        nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([1]))
