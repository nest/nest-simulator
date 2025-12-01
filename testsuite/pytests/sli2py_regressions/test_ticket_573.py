# -*- coding: utf-8 -*-
#
# test_ticket_573.py
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

"""
Regression test for Ticket #573.

Test ported from SLI regression test.
Ensure that CopyModel raises when the maximum number of synapse models is exceeded.

Author: Maximilian Schmidt, 2014-02-20
"""


def test_ticket_573_copy_model_respects_synapse_limit():
    """
    Ensure CopyModel raises once the synapse model limit is exceeded.
    """

    nest.ResetKernel()

    max_synapse_models = nest.max_num_syn_models
    existing_synapse_models = len(nest.synapse_models)
    max_additional_models = max_synapse_models - existing_synapse_models

    with pytest.raises(nest.kernel.NESTError, match="Synapse model count exceeded"):
        for idx in range(max_additional_models + 1):
            nest.CopyModel("static_synapse", f"ticket_573_syn_{idx}")
