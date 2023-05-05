# -*- coding: utf-8 -*-
#
# test_issue_545.py
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
This test ensures that calling SetDefaults with a wrong datatype sets it correctly nonetheless.
"""

import nest
import pytest


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize('num_threads', [1, 4])
@pytest.mark.parametrize('data', [('stdp_synapse', 'tau_plus'), ('iaf_psc_alpha', 'C_m')])
def test_incorrect_integer_data_type_works_with_defaults(num_threads, data):
    nest.ResetKernel()
    nest.local_num_threads = num_threads
    model, option = data

    nest.SetDefaults(model,
                     {option: 10})
    assert nest.GetDefaults(model)[option] == 10
