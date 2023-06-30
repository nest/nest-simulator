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
This test ensures that calling various setters with incorrect properties correctly raises an error.
"""

import nest
import pytest


@pytest.mark.skipif_missing_threads
def test_setters_raise_error_on_bad_properties():
    nest.ResetKernel()
    nest.local_num_threads = 4

    # test defaults
    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nest.SetDefaults("iaf_psc_alpha", {"tau_m": -10})

    # test neuron
    n = nest.Create("iaf_psc_alpha")
    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        n.set({"tau_m": -10})

    # test synapse
    with pytest.raises(nest.kernel.NESTErrors.BadDelay):
        nest.Connect(n, n, syn_spec={"delay": -10})
