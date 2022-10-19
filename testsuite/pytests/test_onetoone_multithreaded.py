# -*- coding: utf-8 -*-
#
# test_onetoone_multithreaded.py
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
from nest.ll_api import sr

"""
Test for correct spike transmission through one-to-one connections on
multiple threads.
"""

import nest
import pytest


@nest.ll_api.check_stack
class TestOnetooneMultithreaded:
    """
    Test that spikes are transmitted correctly after one-to-one connections.

    Under certain circumstances in development code, spikes were not transmitted
    after connection with one-to-one rule when running on multiple threads. In
    cases observed, transmission only failed
        - if spikes were compressed
        - for some combinations of number of threads (>1) and number of neurons
    """

    @pytest.mark.parametrize("compressed_spikes", [False, True])
    @pytest.mark.parametrize("num_neurons", [4, 5])
    @pytest.mark.parametrize("num_threads", [1, 2, 3, 4])
    def test_spike_transmission(self, compressed_spikes, num_neurons, num_threads):
        """
        Create two groups of parrot neurons, pre and post, connected using one-to-one.
        Drive pre with spike generator with single spike.
        Record spikes from post.
        Expected: each post neuron fires one spike at time of generated spike + 2 delay
        """

        t_spike = 1.5
        delay = 1.0

        nest.ResetKernel()
        nest.local_num_threads = num_threads
        nest.use_compressed_spikes = compressed_spikes

        sg = nest.Create("spike_generator", params={"spike_times": [t_spike]})
        sr = nest.Create("spike_recorder")

        pre = nest.Create("parrot_neuron", num_neurons)
        post = nest.Create("parrot_neuron", num_neurons)

        nest.Connect(sg, pre)
        nest.Connect(pre, post, "one_to_one", syn_spec={'delay': delay, 'weight': 1})
        nest.Connect(post, sr)

        nest.Simulate(t_spike + 3 * delay)

        res = sr.get('events')
        assert set(res['senders']) == set(post.tolist())
        assert set(res['times']) == {t_spike + 2 * delay}
