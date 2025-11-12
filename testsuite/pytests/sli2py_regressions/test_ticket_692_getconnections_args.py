# -*- coding: utf-8 -*-
#
# test_ticket_692_getconnections_args.py
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
Regression test for Ticket #692.

Test ported from SLI regression test.
Ensure GetConnections validates the types of source and target arguments.

Author: Hans Ekkehard Plesser, 2013-04-18
"""


INVALID_VALUES = (0, 0.0, "", {}, "foo")


@pytest.mark.parametrize("key", ("source", "target"))
@pytest.mark.parametrize("value", INVALID_VALUES)
def test_ticket_692_getconnections_rejects_non_collections(key, value):
    """
    Ensure GetConnections raises when source/target is not a node collection or sequence.
    """

    nest.ResetKernel()

    with pytest.raises((TypeError, nest.kernel.NESTError)):
        nest.GetConnections(**{key: value})
