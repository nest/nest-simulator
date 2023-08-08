# -*- coding: utf-8 -*-
#
# test_issue_2795.py
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
Regression test for Issue #2795 (GitHub).

The issue was that the following resulted in a crash instead of an error
message:

.. code-block:: python

   import nest

   nest.Connect([0], [0])

The reason was that ``node_id=0`` passed through the consistency check of
``ModelRangeManager::is_in_range`` due to the default values of the expected
first and last node id's also being 0 before a ``NodeCollection`` is created.
The consistency check now addresses the edge case of ``node_id=0``.
"""

import pytest

import nest


@pytest.mark.parametrize("node_id", [0, 1])
def test_connection_without_creation(node_id):
    """
    Ensure that connecting non-existent nodes fails.

    This test ensures that connecting node ids when no nodes have not been
    created results in an ``UnknownNode`` error, including the invalid
    ``node_id=0``.
    """

    with pytest.raises(nest.kernel.NESTErrors.UnknownNode):
        nest.Connect([node_id], [node_id])
