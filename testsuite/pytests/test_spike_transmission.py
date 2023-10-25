# -*- coding: utf-8 -*-
#
# test_spike_transmission.py
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
Test correct spike transmission for multiple threads.
"""

import nest
import pytest

# This is a hack until I find out how to use the have_threads fixture to
# implement this switch. Then, one should also see if we can parametrize
# the entire class instead of parametrizing each test in the class in the
# same way.
if nest.ll_api.sli_func("is_threaded"):
    THREAD_NUMBERS = [1, 2, 3, 4]
else:
    THREAD_NUMBERS = [1]


@nest.ll_api.check_stack
class TestSpikeTransmission:
    """
    Test that spikes are transmitted correctly for different numbers of spikes.

    Tests in this class send a single spike between parrot neurons and test that
    the spike arrives at the correct time and once per connection at each target.
    """

    dt = 0.1  # time step
    t_spike = 0.5  # time at which to send spike
    delay = 1.0  # transmission delay
    t_arrival = t_spike + 2 * delay  # expected spike arrival time

    @classmethod
    def _simulate_network(cls, n_pre, n_post, conn_rule, num_threads, compressed_spikes):
        """
        Simulate network for given parameters and return spike recorder events.

        n_pre parrot neurons connected to n_post parrot neurons with given rule.
        """

        nest.ResetKernel()
        nest.resolution = cls.dt
        nest.local_num_threads = num_threads
        nest.use_compressed_spikes = compressed_spikes

        sg = nest.Create("spike_generator", params={"spike_times": [cls.t_spike]})
        sr = nest.Create("spike_recorder")

        pre = nest.Create("parrot_neuron", n_pre)
        post = nest.Create("parrot_neuron", n_post)

        nest.Connect(sg, pre)
        nest.Connect(pre, post, conn_rule, syn_spec={"delay": cls.delay, "weight": 1})
        nest.Connect(post, sr)

        nest.Simulate(cls.t_spike + 3 * cls.delay)

        return post, sr.events

    @pytest.mark.parametrize("compressed_spikes", [False, True])
    @pytest.mark.parametrize("num_neurons", [4, 5])
    @pytest.mark.parametrize("num_threads", THREAD_NUMBERS)
    def test_one_to_one(self, compressed_spikes, num_neurons, num_threads):
        """
        Test for one-to-one connectivity.

        Connect num_neurons pre to num_neurons post neurons with one-to-one rule.

        Expectation: Each post neuron receives exactly one spike from each pre neuron.
        """

        post_pop, spike_data = self._simulate_network(
            num_neurons, num_neurons, "one_to_one", num_threads, compressed_spikes
        )
        assert sorted(spike_data["senders"]) == sorted(post_pop.tolist())
        assert all(spike_data["times"] == self.t_arrival)

    @pytest.mark.parametrize("compressed_spikes", [False, True])
    @pytest.mark.parametrize("num_neurons", [4, 5])
    @pytest.mark.parametrize("num_threads", THREAD_NUMBERS)
    def test_one_to_all(self, compressed_spikes, num_neurons, num_threads):
        """
        Test for one-to-all connectivity.

        Connect one pre to num_neurons post neurons with all-to-all rule.

        Expectation: Each post neuron receives exactly one spike from pre neuron.
        """

        post_pop, spike_data = self._simulate_network(1, num_neurons, "all_to_all", num_threads, compressed_spikes)
        assert sorted(spike_data["senders"]) == sorted(post_pop.tolist())
        assert all(spike_data["times"] == self.t_arrival)

    @pytest.mark.parametrize("compressed_spikes", [False, True])
    @pytest.mark.parametrize("num_neurons", [4, 5])
    @pytest.mark.parametrize("num_threads", THREAD_NUMBERS)
    def test_all_to_one(self, compressed_spikes, num_neurons, num_threads):
        """
        Test for all-to-one connectivity.

        Connect num_neurons pre to one post neuron with all-to-all rule.

        Expectation: The post neuron receives exactly one spike from each pre neuron.
        """

        post_pop, spike_data = self._simulate_network(num_neurons, 1, "all_to_all", num_threads, compressed_spikes)
        # post_pop is one neuron, which receives a spike from each pre neuron
        assert all(spike_data["senders"] == num_neurons * post_pop.tolist())
        assert all(spike_data["times"] == self.t_arrival)

    @pytest.mark.parametrize("compressed_spikes", [False, True])
    @pytest.mark.parametrize("num_neurons", [4, 5])
    @pytest.mark.parametrize("num_threads", THREAD_NUMBERS)
    def test_all_to_all(self, compressed_spikes, num_neurons, num_threads):
        """
        Test for all-to-all connectivity.

        Connect num_neurons pre to num_neurons post with all-to-all rule.

        Expectation: Each post neuron receives exactly one spike from each pre neuron.
        """

        post_pop, spike_data = self._simulate_network(
            num_neurons, num_neurons, "all_to_all", num_threads, compressed_spikes
        )
        assert sorted(spike_data["senders"]) == sorted(num_neurons * post_pop.tolist())
        assert all(spike_data["times"] == self.t_arrival)
