# -*- coding: utf-8 -*-
#
# test_hpc_synapse.py
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
Test basic properties of HPC synapses as follows:

1. For all known synapses with _hpc ending and counterparts without _hpc
   connect spike generator to one neuron with normal, one with _hpc synapse,
   ensure that simulation yields identical membrane potentials.

2. Check that adding and freezing/thawing of nodes either is blocked or
   does not affect results, i.e., that the TargetIdentifierIndicies are ok.
   These tests proceed as follows:
   1. Connect spike_generator to N neurons with different weights,
      disable spiking in receptors. We use membrane potential after simulation
      as indicator for through which synpse a neuron received input.
   2. Build network once with plain static_synapse to get reference data.
   3. Then build with respective test cases and compare.
"""

import nest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()
    nest.set_verbosity("M_ERROR")





