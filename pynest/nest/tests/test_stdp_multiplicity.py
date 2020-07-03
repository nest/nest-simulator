# -*- coding: utf-8 -*-
#
# test_stdp_multiplicity.py
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

# This script tests the parrot_neuron in NEST.

import nest
import unittest
import math
import numpy as np


@nest.ll_api.check_stack
class StdpSpikeMultiplicity(unittest.TestCase):
    """
    Test correct handling of spike multiplicity in STDP.

    This test originated from work on issue #77.

    Concerning the definition of STDP for multiplicity > 1, consider the
    following (see also @flinz on #82): A plastic synapse is created between
    two parrot neurons. As reference case, we use two precise spiking parrot
    neurons driven by spike trains n*h - k*delta with delta < h, and 1 < k < K
    such that K delta < h, i.e., we get K spikes in each time step (h is
    resolution). If we transmit the same spike trains via plain parrot
    neurons, they will be handled as spikes with multiplicity K. Then, in the
    limit delta -> 0, the weight changes observed in the synapse between the
    precise parrots shall converge to the weight changes observed between the
    plain parrots.

    We test the resulting weights as follows:

    1. Weights obtained using parrot_neuron must be identical independent of
       delta, since in this case all spikes are at the end of the step, i.e.,
       all spikes have identical times independent of delta.

    2. We choose delta values that are decrease by factors of 2. The
       plasticity rules depend on spike-time differences through

           exp(dT / tau)

       where dT is the time between pre- and postsynaptic spikes. We construct
       pre- and postsynaptic spike times so that

          dT = pre_post_shift + m * delta

       with m * delta < resolution << pre_post_shift. The time-dependence
       of the plasticity rule is therefore to good approximation linear in
       delta.

       We can thus test as follows: Let w_pl be the weight obtained with the
       plain parrot, and w_ps_j the weight obtained with the precise parrot
       for delta_j = delta0 / 2^j. Then,

         ( w_ps_{j+1} - w_pl ) / ( w_ps_j - w_pl ) ~ 0.5  for all j

       i.e., the difference between plain and precise weights halves each
       time delta is halved.
    """

    def run_protocol(self, pre_post_shift):
        """
        Create network and simulate for each delta value.

        Returns a dict with the synaptic weight at end of simulation for
        plain and precise parrots, one weight per delta value.

        All values for the plain parrot case should be identical, and
        the values for the precise parrot case should converge to that value
        for delta -> 0.

        All delta values must fulfill

           multiplicity * delta < resolution / 2

        so that in the plain case off-grid spike times are rounded up
        to the end of the step and thus belong to the same step as the
        corresponding precise spikes.

        :param pre_post_shift: Delay between pre- and postsynaptic trains
        :returns: {'parrot': [<weights>], 'parrot_ps': [<weights>]}
        """

        multiplicity = 2**3
        resolution = 2.**-4
        tics_per_ms = 1. / resolution * multiplicity * 4
        deltas = [resolution / multiplicity / 2**m for m in range(2, 10)]

        delay = 1.

        # k spikes will be emitted at these two times
        pre_spike_times_base = [100., 200.]

        nest.set_verbosity("M_WARNING")

        post_weights = {'parrot': [], 'parrot_ps': []}

        for delta in deltas:
            assert multiplicity * delta < resolution / 2., "Test inconsistent."

            nest.ResetKernel()
            nest.SetKernelStatus({'tics_per_ms': tics_per_ms,
                                  'resolution': resolution})

            pre_times = sorted(t_base - k * delta
                               for t_base in pre_spike_times_base
                               for k in range(multiplicity))
            post_times = [pre_time + pre_post_shift
                          for pre_time in pre_times]

            # create spike_generators with these times
            pre_sg = nest.Create("spike_generator",
                                 params={"spike_times": pre_times,
                                         'allow_offgrid_times': True})
            post_sg = nest.Create("spike_generator",
                                  params={"spike_times": post_times,
                                          'allow_offgrid_times': True})
            pre_sg_ps = nest.Create("spike_generator",
                                    params={"spike_times": pre_times,
                                            'precise_times': True})
            post_sg_ps = nest.Create("spike_generator",
                                     params={"spike_times": post_times,
                                             'precise_times': True})

            # create parrot neurons and connect spike_generators
            pre_parrot = nest.Create("parrot_neuron")
            post_parrot = nest.Create("parrot_neuron")
            pre_parrot_ps = nest.Create("parrot_neuron_ps")
            post_parrot_ps = nest.Create("parrot_neuron_ps")

            nest.Connect(pre_sg, pre_parrot,
                         syn_spec={"delay": delay})
            nest.Connect(post_sg, post_parrot,
                         syn_spec={"delay": delay})
            nest.Connect(pre_sg_ps, pre_parrot_ps,
                         syn_spec={"delay": delay})
            nest.Connect(post_sg_ps, post_parrot_ps,
                         syn_spec={"delay": delay})

            # create spike detector --- debugging only
            spikes = nest.Create("spike_detector")
            nest.Connect(
                pre_parrot + post_parrot +
                pre_parrot_ps + post_parrot_ps,
                spikes
            )

            # connect both parrot neurons with a stdp synapse onto port 1
            # thereby spikes transmitted through the stdp connection are
            # not repeated postsynaptically.
            nest.Connect(
                pre_parrot, post_parrot,
                syn_spec={'synapse_model': 'stdp_synapse', 'receptor_type': 1})
            nest.Connect(
                pre_parrot_ps, post_parrot_ps,
                syn_spec={'synapse_model': 'stdp_synapse', 'receptor_type': 1})

            # get STDP synapse and weight before protocol
            syn = nest.GetConnections(source=pre_parrot,
                                      synapse_model="stdp_synapse")
            w_pre = syn.get('weight')
            syn_ps = nest.GetConnections(source=pre_parrot_ps,
                                         synapse_model="stdp_synapse")
            w_pre_ps = syn_ps.get('weight')

            sim_time = max(pre_times + post_times) + 5 * delay
            nest.Simulate(sim_time)

            # get weight post protocol
            w_post = syn.get('weight')
            w_post_ps = syn_ps.get('weight')

            assert w_post != w_pre, "Plain parrot weight did not change."
            assert w_post_ps != w_pre_ps, "Precise parrot \
                weight did not change."

            post_weights['parrot'].append(w_post)
            post_weights['parrot_ps'].append(w_post_ps)

        return post_weights

    def test_ParrotNeuronSTDPProtocolPotentiation(self):
        """Check weight convergence on potentiation."""

        post_weights = self.run_protocol(pre_post_shift=10.0)
        w_plain = np.array(post_weights['parrot'])
        w_precise = np.array(post_weights['parrot_ps'])

        assert all(w_plain == w_plain[0]), 'Plain weights differ'
        dw = w_precise - w_plain
        dwrel = dw[1:] / dw[:-1]
        assert all(np.round(dwrel, decimals=3) ==
                   0.5), 'Precise weights do not converge.'

    def test_ParrotNeuronSTDPProtocolDepression(self):
        """Check weight convergence on depression."""

        post_weights = self.run_protocol(pre_post_shift=-10.0)
        w_plain = np.array(post_weights['parrot'])
        w_precise = np.array(post_weights['parrot_ps'])

        assert all(w_plain == w_plain[0]), 'Plain weights differ'
        dw = w_precise - w_plain
        dwrel = dw[1:] / dw[:-1]
        assert all(np.round(dwrel, decimals=3) ==
                   0.5), 'Precise weights do not converge.'


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite = unittest.TestLoader().loadTestsFromTestCase(StdpSpikeMultiplicity)
    return unittest.TestSuite([suite])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
