# -*- coding: utf-8 -*-
#
# test_memsize.py
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
Test models with calcium concentration.

This set of tests verify the behavior of the calcium concentration in models
that inherit from the strutural plasticity node class in the kernel.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


def test_memsize():
    """
    Verify that memsize is available and works somewhat reasonable.
    """

    m_pre = nest.memory_size

    n = nest.Create("parrot_neuron", 1000)
    nest.Connect(n, n)

    m_post = nest.memory_size

    assert m_pre > 0
    assert m_post > m_pre
