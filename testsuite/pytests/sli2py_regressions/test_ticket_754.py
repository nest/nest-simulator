# -*- coding: utf-8 -*-
#
# test_ticket_754.py
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
Test that rng_seed and rng_type is handled correctly also in connection with changing VP numbers.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


DEFAULT_SEED = 143202461
DEFAULT_RNG_TYPE = "mt19937_64"


def test_confirm_base_seed():
    """
    Confirm that NEST has expected default seed.
    """

    assert nest.rng_seed == DEFAULT_SEED and nest.rng_type == DEFAULT_RNG_TYPE


def test_reset_kernel_resets_seed():
    """
    Ensure that ResetKernel returns to default seed.
    """

    nest.rng_seed = 123
    nest.rng_type = "mt19937"
    nest.ResetKernel()
    assert nest.rng_seed == DEFAULT_SEED and nest.rng_type == DEFAULT_RNG_TYPE


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("proc_param_name", ["local_num_threads", "total_num_virtual_procs"])
class TestSeedingAndProcSetting:
    """
    Tests that setting seed and number of thread or of VPs works in arbitrary order.
    """

    NUM_PROCS = 2
    TEST_SEED = 123

    def test_default_seed_after_proc_change(self, proc_param_name):
        """
        Test that just setting process number retains default seed and type.
        """

        nest.set(**{proc_param_name: self.NUM_PROCS})
        assert nest.rng_seed == DEFAULT_SEED and nest.rng_type == DEFAULT_RNG_TYPE

    def test_set_procs_then_seed(self, proc_param_name):
        """
        Test that changing number of processes and then seeding and gives correct seed.
        """

        nest.set(**{proc_param_name: self.NUM_PROCS})
        nest.rng_seed = self.TEST_SEED
        assert nest.rng_seed == self.TEST_SEED

    def test_seed_then_set_procs(self, proc_param_name):
        """
        Test that seeding and then setting number of procs gives correct seed.
        """

        nest.rng_seed = self.TEST_SEED
        nest.set(**{proc_param_name: self.NUM_PROCS})
        assert nest.rng_seed == self.TEST_SEED

    def test_seed_and_set_procs_simultaneously(self, proc_param_name):
        """
        Test that changing number of processes and seeding in one call gives correct seed.
        """

        nest.set(**{"rng_seed": self.TEST_SEED, proc_param_name: self.NUM_PROCS})
        assert nest.rng_seed == self.TEST_SEED


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("proc_param_name", ["local_num_threads", "total_num_virtual_procs"])
class TestRngTypeAndProcSetting:
    """
    Tests that setting rng type and number of thread or of VPs works in arbitrary order.
    """

    NUM_PROCS = 2
    TEST_RNG_TYPE = "mt19937"

    def test_set_procs_then_rng_type(self, proc_param_name):
        """
        Test that changing number of processes and then changing rng type and gives correct rng type.
        """

        nest.set(**{proc_param_name: self.NUM_PROCS})
        nest.rng_type = self.TEST_RNG_TYPE
        assert nest.rng_type == self.TEST_RNG_TYPE

    def test_set_rng_type_then_procs(self, proc_param_name):
        """
        Test that setting rng type and then setting number of procs gives correct rng type.
        """

        nest.rng_type = self.TEST_RNG_TYPE
        nest.set(**{proc_param_name: self.NUM_PROCS})
        assert nest.rng_type == self.TEST_RNG_TYPE

    def test_set_rng_type_and_procs_simultaneously(self, proc_param_name):
        """
        Test that changing number of processes and rng type in one call gives correct rng type.
        """

        nest.set(**{"rng_type": self.TEST_RNG_TYPE, proc_param_name: self.NUM_PROCS})
        assert nest.rng_type == self.TEST_RNG_TYPE
