# -*- coding: utf-8 -*-
#
# test_spl_synapse.py
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

# This script compares the two variants of the Tsodyks/Markram synapse in NEST.

import nest
import numpy as np
import math
import unittest


@nest.check_stack
class StdpStructplSynapseTestCase(unittest.TestCase):
    """Test SPL synapses."""

    def test_resize(self):
        """Resizing of potential connections"""

        syn_spec = {
            "model": "testsyn",
            "receptor_type": 1,
        }
        conn_spec = {
            "rule": "all_to_all",
        }

        # a single potential connection
        self.setUp_net(1)
        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")
        nest.SetStatus(syn, {'n_pot_conns': 1})
        nest.Simulate(210.)
        syn_status = nest.GetStatus(syn)
        assert len(syn_status[0]['w_jk']) == 1, "Resizing failed"

        # resizing potential conns
        self.setUp_net(1)
        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")
        nest.SetStatus(syn, {'n_pot_conns': 2})
        nest.Simulate(210.)
        syn_status = nest.GetStatus(syn)
        assert len(syn_status[0]['w_jk']) == 2, "Resizing failed"

    def test_resize_asymmetric(self):
        """Asymmetric resizing of potential connections"""
        syn_spec = {
            "model": "testsyn",
            "receptor_type": 1,
        }
        conn_spec = {
            "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        self.setUp_net(2)
        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")
        assert len(syn) == 2, "Not enought connections created"

        npot = [1, 2]
        for i, s in enumerate(syn):
            nest.SetStatus([s], {'n_pot_conns': npot[i]})

        nest.Simulate(210.)

        syn_status = nest.GetStatus(syn)
        assert len(syn_status[0]['w_jk']) == 1 \
            and len(syn_status[1]['w_jk']) == 2, "Asymmetric resizing failed"

    def test_decay_rpost_jk(self):
        """Decay of r_post without inputs"""
        self.setUp_decay()
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['r_post_jk'][0]

        dt = 0.001
        tau = syn_defaults['tau']
        prop = np.exp(-dt / tau)

        spike0 = 1. / tau * prop**80.  # propagate spike 1 by 80ms
        spike1 = 1. / tau * prop**60.  # propagate spike 2 by 60ms
        spike3 = 1. / tau * prop**40.  # propagate first spike 40ms
        val_exp = spike0 + spike1 + spike3

        self.assertAlmostEqualDetailed(
            val_exp, val, "Decay of r_post not as expected.")

    def test_decay_Rpost_jk(self):
        """Decay of R_post without inputs"""
        self.setUp_decay()
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['R_post_jk'][0]

        dt = 0.001
        tau = syn_defaults['tau_slow']
        prop = np.exp(-dt / tau)

        spike0 = 1. / tau * prop**80.  # propagate spike 1 by 80ms
        spike1 = 1. / tau * prop**60.  # propagate spike 2 by 60ms
        spike3 = 1. / tau * prop**40.  # propagate first spike 40ms
        val_exp = spike0 + spike1 + spike3

        self.assertAlmostEqualDetailed(
            val_exp, val, "Decay of R_post not as expected.")

    def test_decay_cjk(self):
        """Decay of c_jk without inputs"""
        self.setUp_decay(params={'p_fail': 1.}
                         )  # ensures r_jk = 0, and thus only decay
        nest.SetStatus(self.syn, {'c_jk': [1.]})

        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['c_jk']
        dt = 0.001
        tau = syn_defaults['tau_slow']
        prop = np.exp(-dt / tau)

        val_exp = 1. * prop**200.

        self.assertAlmostEqualDetailed(
            val_exp, val, "Decay of c_jk not as expected.")

    def test_decay_Rpre_deterministic(self):
        """Decay and passing for p_fail=0"""
        self.setUp_decay(params={'p_fail': 0.})
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['r_jk']
        dt = 0.001
        tau = syn_defaults['tau']
        prop = np.exp(-dt / tau)

        spike0 = 1. / tau * prop**100.  # propagate spike 1 by 100ms
        val_exp = spike0 + 1. / tau  # second spike increases this value
        self.assertAlmostEqualDetailed(
            val_exp,
            val,
            "Decay of r_jk not as expected (deterministic case).")

    def test_decay_Rpre_allfail(self):
        """No spikes pass for p_fail==1"""
        self.setUp_decay(params={'p_fail': 1.})
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        val = syn_status[0]['r_jk'][0]
        spike0 = 0.  # no spike passes
        val_exp = spike0
        self.assertAlmostEqualDetailed(
            val_exp, val, "Decay of r_jk not as expected (no spike case).")

    def test_decay_Rpre_half_fail(self):
        """Stochastic and selective increase of r_jk, plus decay"""
        self.setUp_decay(params={'p_fail': .2, 'n_pot_conns': 2})

        # this seed lets the first spike on syn 0 and the second spike on syn 1
        # pass
        msd = 4
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['r_jk']
        dt = 0.001
        tau = syn_defaults['tau']
        prop = np.exp(-dt / tau)

        spike0 = 1. / tau * prop**100.  # propagate spike 1 by 100ms
        # no spike increase on 1, only second spike increase on 2
        val_exp = [spike0, 1. / tau]
        for k in range(len(val)):
            self.assertAlmostEqualDetailed(
                val_exp[k],
                val[k],
                "Decay of r_jk[%i] not as expected (stochastic case)" % k)

    def test_create_time(self):
        """Weight creation and decay of creation steps"""
        self.setUp_iafpost(params={
            'p_fail': 1.,
            'n_pot_conns': 2,
            'w_jk': [0., 0.],
        })

        syn_defaults = nest.GetDefaults('testsyn')
        w0 = syn_defaults['w0']

        # this seed lets the first spike on syn 0 and the second spike on syn 1
        # pass
        msd = 4
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        # simulate past spike 1, this causes deletion of the synapses internally
        # and proper setting to nan.
        nest.Simulate(101.)

        # set creation steps between next spikes
        nest.SetStatus(self.syn, {'w_create_steps': [50, 150]})

        syn_status = nest.GetStatus(self.syn)[0]
        self.assertEqual(syn_status["n_delete"], 2,
                         "Synapses were not deleted")

        # after next spike we should have created the synapse
        nest.Simulate(100.)
        syn_status = nest.GetStatus(self.syn)

        # 1 synapse has been created
        self.assertEqual(
            syn_status[0]['n_create'], 1, "n_create not updated")

        self.assertEqual(
            syn_status[0]['w_create_steps'],
            (0, 50),
            "w_create_steps not properly updated")

        nest.Simulate(100.)
        syn_status = nest.GetStatus(self.syn)

        # 2 synapses have been created
        self.assertEqual(
            syn_status[0]['n_create'], 2, "n_create not updated")

        self.assertEqual(
            syn_status[0]['w_create_steps'],
            (0, 0),
            "w_create_steps not properly updated")

    def test_create_notransmit(self):
        """To be created synapses do not transmit"""
        self.setUp_iafpost(params={
            'p_fail': 0.,
            'n_pot_conns': 2,
            'w_jk': [1., 0.],
            't_grace_period': 0.,
            'w0': 0.5,
        })

        syn_defaults = nest.GetDefaults('testsyn')
        w0 = syn_defaults['w0']

        # set seed to something reproducible
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.Simulate(150.)
        w_jk_0 = nest.GetStatus(self.syn)[0]['w_jk']
        # set the voltage to resting potential to exclude errors further down
        nest.SetStatus(self.post, {'V_m': -70.})

        # create a synapse at the timestep of the next spike
        # --> create is counted from last update-causing presyn. spike
        nest.SetStatus(self.syn, {'w_create_steps': [0, 100]})

        # simulate past next spike
        nest.Simulate(100.)
        w_jk_1 = nest.GetStatus(self.syn)[0]['w_jk']

        # get events
        events = nest.GetStatus(self.rec)[0]['events']
        t = events['times']

        # first spike is only the first weight
        val_0 = events['V_m'][t == 101.] - events['V_m'][t == 100.]
        val_exp_0 = np.array(w_jk_0[0]).sum()

        # second spike is value of first weight + initialization weight
        # (initialization happens exactly in the timestep of the spike)
        val_1 = events['V_m'][t == 201.] - events['V_m'][t == 200.]
        val_exp_1 = np.array(w_jk_1[0] + w0).sum()

        self.assertAlmostEqualDetailed(
            val_exp_0,
            val_0,
            "Transmitted weights not correct, first spike.",
            places=12)

        self.assertAlmostEqualDetailed(
            val_exp_1,
            val_1,
            "Transmitted weights not correct, second spike.",
            places=12)

    def test_w_decay(self):
        """Decay of w_jk, proportional to alpha"""
        self.setUp_decay(params={
            'p_fail': 1.,
            'n_pot_conns': 1,
            'A2_corr': 0.,
            'A4_corr': 0.,
            'A4_post': 0.
        })

        nest.SetStatus(self.syn, {'w_jk': [1.]})

        syn_defaults = nest.GetDefaults('testsyn')
        alpha = syn_defaults['alpha']

        nest.Simulate(101.)
        syn_status = nest.GetStatus(self.syn)

        nest.Simulate(100.)
        syn_status = nest.GetStatus(self.syn)

        dt = 0.001
        prop = np.exp(-dt * alpha)

        val = syn_status[0]['w_jk'][0]
        val_exp = 1. * prop**200.

        self.assertAlmostEqualDetailed(
            val_exp, val, "Weight decay (alpha) not correct.")

    def test_w_cjk_terms(self):
        """Dynamics on w_jk with correlation terms"""
        self.setUp_decay(params={
            'p_fail': 0.,
            'n_pot_conns': 1,
            'A2_corr': 10. * 1e-3,  # these values were chosen
                                    # for ms time units,
            'A4_corr': 10. * 1e-9,
            'A4_post': 0.,
            'alpha': 0.
        })

        nest.SetStatus(self.syn, {'w_jk': [1.]})

        syn_defaults = nest.GetDefaults('testsyn')
        tau = syn_defaults['tau']
        tau_slow = syn_defaults['tau_slow']
        A2_corr = syn_defaults['A2_corr']
        A4_corr = syn_defaults['A4_corr']

        nest.Simulate(210.)
        syn_status = nest.GetStatus(self.syn)

        times = np.arange(0, 201, 1)
        post_times = np.where(np.in1d(times, [120., 140., 160.]))[0]
        pre_times = np.where(np.in1d(times, [100., 200.]))[0]

        dt = 0.001
        r_jk = np.zeros(201)
        for i in range(1, 201):
            r_jk[i] = r_jk[i - 1] * np.exp(-dt / tau)
            if i in pre_times:
                r_jk[i] += 1. / tau

        r_post = np.zeros(201)
        for i in range(1, 201):
            r_post[i] = r_post[i - 1] * np.exp(-dt / tau)
            if i in post_times:
                r_post[i] += 1. / tau

        c_jk = np.zeros(201)
        for i in range(1, 201):
            c_jk[i] = (
                (-1 + np.exp(dt * (-2 / tau + 1 / tau_slow))) * r_jk[i - 1] *
                r_post[i - 1] * tau + c_jk[i - 1] * (tau - 2 * tau_slow)) /\
                (np.exp(dt / tau_slow) * (tau - 2 * tau_slow))

        w_jk = np.zeros(201)
        w_jk[0] = 1.
        for i in range(1, 201):
            w_jk[i] = w_jk[i - 1] + dt * \
                (A2_corr * c_jk[i - 1] - A4_corr * c_jk[i - 1]**2)

        val = syn_status[0]['w_jk'][0]
        val_exp = w_jk[-1]

        self.assertAlmostEqualDetailed(
            val_exp,
            val,
            "Weight dynamics with correlation terms not correct",
            places=3)

    def test_w_delete(self):
        """Test deletion of connections from the pool of potential
        connections if weights are 0"""

        lam = 1e-6  # ms
        n_sample = int(1e3)
        self.setUp_iafpost(params={
            'p_fail': 0.,
            'n_pot_conns': n_sample,
            'lambda': lam * 1e3,  # units in [s],
            'w_jk': [1.] * n_sample
        })

        # this seed lets the first spike on syn 0 and the
        # second spike on syn 1 pass
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.Simulate(150.)
        # deactivate all synapses at the timestep of the next spike
        nest.SetStatus(self.syn, {'w_jk': [0.] * n_sample})

        # simulate past next spike
        nest.Simulate(100.)
        syn_status = nest.GetStatus(self.syn)[0]

        try:
            from scipy.stats import kstest
            create_vals = np.array(
                syn_status['w_create_steps'])

            # test for uniformity of transformed exponential distribution
            # with a one-sided KS test
            unif_vals = np.exp(- create_vals * lam)
            D, p_val = kstest(unif_vals, 'uniform')
            self.assertTrue(
                p_val > 0.05,
                "Generated creation times not distributed correctly.")
        except ImportError:
            raise RuntimeWarning(
                'Scipy not available: skipped statistical testing '
                'of creation times')

        self.assertEqual(
            syn_status["n_delete"], n_sample,
            "Number of deleted synapses not correct")

    def test_grace_period(self):
        """Test grace period behavior"""

        grace = .1
        create_steps = 50
        grace_steps = int(grace * 1e3)

        # Set up a parrot network with spikes pre and post,
        # centered around creation with grace period
        grace_steps = int(grace * 1e3)
        dyn_start = 51 + grace_steps + create_steps

        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            't_grace_period': grace,
            'w_jk': [0.]
        }

        self.setUp_net(1, params=pars)

        delay = 1.

        # spikes to update synapse. several around
        # when the dynamics pick up at dyn_start
        spikes = (np.arange(50, dyn_start - 50 + 1, 50) - delay).tolist() +\
            (np.arange(dyn_start - 2., dyn_start + 2.) - delay).tolist()

        nest.SetStatus(
            self.pre_spikes, {"spike_times": spikes})

        # postsynaptic spikes to feed traces
        nest.SetStatus(
            self.post_spikes,
            {"spike_times": (np.arange(50, dyn_start, 10) - delay).tolist()})

        syn_spec = {
            "model": "testsyn",
            "receptor_type": 1,
        }

        conn_spec = {
            "rule": "all_to_all",
        }

        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        self.syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")

        syn_defaults = nest.GetDefaults('testsyn')
        w0 = syn_defaults['w0']

        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        # simulate past first presynaptic spike
        nest.Simulate(51.)

        # create a synapse at time
        nest.SetStatus(self.syn, {'w_create_steps': [create_steps]})
        syn_status = nest.GetStatus(self.syn)[0]

        # simulate past next spike
        nest.Simulate(50.)  # t=101
        syn_status = nest.GetStatus(self.syn)[0]
        # here the w_create has passed, synapse is created and
        # w_create_steps is set to -grace
        self.assertEqual(
            syn_status['w_create_steps'][0],
            -grace_steps,
            "w_create_steps does not decrement correcly."
        )
        self.assertEqual(
            syn_status['w_jk'][0],
            w0,
            "weight not initialized properly"
        )
        self.assertEqual(
            syn_status['n_create'],
            1,
            "synapse not created"
        )

        # simulate past next spike
        nest.Simulate(50.)  # t=151
        syn_status = nest.GetStatus(self.syn)[0]
        # here the grace should count down by 50,
        # the synapse should be static at w0
        # and the traces should be doing computations
        self.assertEqual(
            syn_status['w_create_steps'][0],
            -(grace_steps - 50),
            "w_create_steps does not decrement correcly."
        )
        self.assertEqual(
            syn_status['w_jk'][0],
            w0,
            "weight not stable during grace period"
        )
        self.assertTrue(
            syn_status['r_post_jk'][0] > 0. and
            syn_status['r_jk'][0] > 0. and
            syn_status['R_post_jk'][0] > 0. and
            syn_status['c_jk'][0] > 0.,
            "traces not integrating during grace"
        )

        # simulate past next spike
        nest.Simulate(50.)  # t=201
        # here the grace should have counted down to exactly 0,
        # the synapse should still be static at w0
        # and the traces should be doing computations
        syn_status = nest.GetStatus(self.syn)[0]
        self.assertEqual(
            syn_status['w_create_steps'][0],
            0,
            "w_create_steps does not decrement correcly."
        )
        self.assertEqual(
            syn_status['w_jk'][0],
            w0,
            "weight not stable during grace period"
        )
        self.assertTrue(
            syn_status['r_post_jk'][0] > 0. and
            syn_status['r_jk'][0] > 0. and
            syn_status['R_post_jk'][0] > 0. and
            syn_status['c_jk'][0] > 0.,
            "traces not integrating during grace"
        )

        # simulate past next spike
        nest.Simulate(1.)  # t=202
        syn_status = nest.GetStatus(self.syn)[0]
        # here the grace should still be at exactly 0,
        # the synapse should start to be integrated away from w0
        # and the traces should be doing computations
        syn_status = nest.GetStatus(self.syn)[0]
        self.assertEqual(
            syn_status['w_create_steps'][0],
            0,
            "w_create_steps does not decrement correcly."
        )
        self.assertNotEqual(
            syn_status['w_jk'][0],
            w0,
            "weight not integrated after grace period"
        )
        self.assertTrue(
            syn_status['r_post_jk'][0] > 0. and
            syn_status['r_jk'][0] > 0. and
            syn_status['R_post_jk'][0] > 0. and
            syn_status['c_jk'][0] > 0.,
            "traces not integrating during grace"
        )

    def test_create_grace_transmit(self):
        """Test for creation event during grace period and transmission"""
        grace = .05
        grace_steps = int(grace * 1e3)
        create_steps = grace_steps

        params = {
            "p_fail": 0.,
            "n_pot_conns": 2,
            "w_jk": [1., 0.],
            "t_grace_period": grace,
        }

        self.setUp_iafpost(params=params)

        nest.SetDefaults('testsyn', {"w0": .5})
        syn_defaults = nest.GetDefaults('testsyn')
        w0 = syn_defaults['w0']

        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        spikes = [50., 100., 150., 200.]
        delay = 1.
        nest.SetStatus(
            self.pre_spikes,
            {"spike_times": (np.array(spikes) - delay).tolist()})

        # simulate past first presynaptic spike
        nest.Simulate(51.)

        nest.SetStatus(
            self.syn, {'w_create_steps': [-create_steps, create_steps]})
        syn_status = nest.GetStatus(self.syn)[0]
        w_jk_0 = syn_status["w_jk"]

        # simulate past next spike and reset membrane simplify dynamics
        nest.Simulate(1.)
        nest.SetStatus(self.post, {'V_m': -70.})
        nest.Simulate(49.)

        syn_status = nest.GetStatus(self.syn)[0]
        w_jk_1 = syn_status["w_jk"]

        self.assertEqual(
            syn_status["w_create_steps"],
            (0, -grace_steps)
        )

        # weight 0 stays constant, weight 1 is set to w0
        self.assertEqual(
            w_jk_1,
            (w_jk_0[0], w0)
        )

        # simulate past next spike and reset membrane simplify dynamics
        nest.Simulate(1.)
        nest.SetStatus(self.post, {'V_m': -70.})
        nest.Simulate(49.)

        syn_status = nest.GetStatus(self.syn)[0]
        w_jk_2 = syn_status["w_jk"]

        # all create steps have decayed now
        self.assertEqual(
            syn_status["w_create_steps"],
            (0, 0)
        )

        # weight 0 chages, weight 1 is still set to w0
        self.assertNotEqual(w_jk_2[0], w_jk_1[0])
        self.assertEqual(w_jk_2[1], w0)

        # simulate past next spike and reset membrane simplify dynamics
        nest.Simulate(1.)
        nest.SetStatus(self.post, {'V_m': -70.})
        nest.Simulate(49.)

        syn_status = nest.GetStatus(self.syn)[0]
        w_jk_3 = syn_status["w_jk"]
        self.assertNotEqual(w_jk_3[0], w_jk_2[0])
        self.assertNotEqual(w_jk_3[1], w_jk_2[1])

        # simulate past next spike and reset membrane simplify dynamics
        nest.Simulate(1.)
        nest.SetStatus(self.post, {'V_m': -70.})
        nest.Simulate(49.)

        # get events
        events = nest.GetStatus(self.rec)[0]['events']
        t = events['times']

        vals = [
            (events['V_m'][t == s + 1.] - events['V_m'][t == s]).tolist()[0]
            for s in spikes]

        vals_exp = [
            sum([w_jk_0[0], 0]),
            sum([w_jk_0[0], w0]),
            sum([w_jk_2[0], w0]),
            sum(w_jk_3)]

        for i in range(len(vals)):
            self.assertAlmostEqual(
                vals[i],
                vals_exp[i],
                msg="Transmission at spike %i not as expected." % i,
                places=12)

    @unittest.expectedFailure
    def test_fail_set_create(self):
        """Setting w_create_steps after positive weight fails"""
        self.setUp_decay(params={
            'p_fail': 0.,
            'n_pot_conns': 1,
        })

        nest.SetStatus(
            self.syn, {'w_jk': (1.), 'w_create_steps': 10.})

    def test_w_post_terms(self):
        """Dynamics on w_jk with postsynaptic terms only"""
        self.setUp_decay(params={
            'p_fail': 0.,
            'n_pot_conns': 1,
            'A2_corr': 0.,
            'A4_corr': 0.,
            'A4_post': 10. * 1e-9,
            'alpha': 0.
        })

        nest.SetStatus(self.syn, {'w_jk': [1.]})

        syn_defaults = nest.GetDefaults('testsyn')
        tau_slow = syn_defaults['tau_slow']
        A4_post = syn_defaults['A4_post']

        nest.Simulate(210.)
        syn_status = nest.GetStatus(self.syn)

        times = np.arange(0, 201, 1)
        post_times = np.where(np.in1d(times, [120., 140., 160.]))[0]

        dt = 0.001
        R_post = np.zeros(201)
        for i in range(1, 201):
            R_post[i] = R_post[i - 1] * np.exp(-dt / tau_slow)
            if i in post_times:
                R_post[i] += 1. / tau_slow

        w_jk = np.zeros(201)
        w_jk[0] = 1.
        for i in range(1, 201):
            w_jk[i] = w_jk[i - 1] - dt * A4_post * R_post[i - 1]**4

        val = syn_status[0]['w_jk'][0]
        val_exp = w_jk[-1]

        self.assertAlmostEqualDetailed(
            val_exp, val, "Weight dynamics on postsynaptic terms incorrect")

    def test_postsynaptic_total_weight(self):
        """Total transmitted weight per spike is stochastic sum of
        individual weights"""

        self.setUp_iafpost(params={"p_fail": 0.5, "n_pot_conns": 10})

        # set seed to something reproducible
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.Simulate(150.)
        w_jk_0 = nest.GetStatus(self.syn)[0]['w_jk']
        # set the voltage to resting potential to exclude errors further down
        nest.SetStatus(self.post, {'V_m': -70.})

        nest.Simulate(150.)
        w_jk_1 = nest.GetStatus(self.syn)[0]['w_jk']

        events = nest.GetStatus(self.rec)[0]['events']
        t = events['times']

        # first has spikes in idx [8, 9]
        val_0 = events['V_m'][t == 101.] - events['V_m'][t == 100.]
        val_exp_0 = np.array(w_jk_0)[[8, 9]].sum()

        # first has spikes in idx [0, 3, 6]
        val_1 = events['V_m'][t == 201.] - events['V_m'][t == 200.]
        val_exp_1 = np.array(w_jk_1)[[0, 3, 6]].sum()

        self.assertAlmostEqualDetailed(
            val_exp_0,
            val_0,
            "Transmitted weights not stochastic/correct, first spike.")

        self.assertAlmostEqualDetailed(
            val_exp_1,
            val_1,
            "Transmitted weights not stochastic/correct, second spike.")

    def test_deletion_dynamic(self):
        """Deletion of synapses with zero crossings"""
        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'w_jk': [10.],
            'w_create_steps': [0]
        }

        self.setUp_decay(params=pars)

        # set seed to something reproducible
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.SetStatus(self.syn, {
                'w_jk': [10.],
            })

        delay = 1.
        nest.SetStatus(self.pre_spikes, {
                "spike_times": (np.arange(2., 20.1*1e3, 1e3)-delay).tolist()
            })
        nest.SetStatus(self.post_spikes, {
                "spike_times": (np.arange(2., 20.1*1e3, 10)-delay).tolist()
            })

        nest.Simulate(10000.)
        stat = nest.GetStatus(self.syn)[0]

        self.assertEqual(stat['w_jk'][0], 0.)
        self.assertTrue(math.isnan(stat['r_jk'][0]))
        self.assertTrue(math.isnan(stat['c_jk'][0]))
        self.assertTrue(math.isnan(stat['r_post_jk'][0]))
        self.assertTrue(math.isnan(stat['R_post_jk'][0]))
        self.assertTrue(stat['w_create_steps'][0] > 0)

    def test_deletion_manual(self):
        """Manual deletion of synapses"""
        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'w_jk': [10.],
            'w_create_steps': [0],
            'sleep_mode': False  # otherwise deleted syn. sleeps
                                 # and we can't check updates
        }

        self.setUp_decay(params=pars)

        # set seed to something reproducible
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.SetStatus(self.syn, {
                'w_jk': [10.],
            })

        delay = 1.
        nest.SetStatus(self.pre_spikes, {
                "spike_times": (np.arange(50., 1001., 50.)-delay).tolist()
            })
        nest.SetStatus(self.post_spikes, {
                "spike_times": [1.]
            })

        nest.Simulate(51.)
        nest.SetStatus(self.syn, {
                'w_jk': [0.],
            })

        nest.Simulate(50.)
        stat = nest.GetStatus(self.syn)[0]
        self.assertEqual(stat['w_jk'][0], 0.)
        self.assertTrue(stat['w_create_steps'][0] > 0)
        self.assertTrue(math.isnan(stat['r_jk'][0]))
        self.assertTrue(math.isnan(stat['c_jk'][0]))
        self.assertTrue(math.isnan(stat['r_post_jk'][0]))
        self.assertTrue(math.isnan(stat['R_post_jk'][0]))

    def test_deletion_manual_with_steps(self):
        """Manual deletion of synapses, setting the steps"""
        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'w_jk': [10.],
            'w_create_steps': [0],
            'sleep_mode': False  # otherwise deleted syn. sleeps
                                 # and we can't check updates
        }

        self.setUp_decay(params=pars)

        # set seed to something reproducible
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus(
            {'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

        nest.SetStatus(self.syn, {
                'w_jk': [10.],
            })

        delay = 1.
        nest.SetStatus(self.pre_spikes, {
                "spike_times": (np.arange(50., 1001., 50.)-delay).tolist()
            })
        nest.SetStatus(self.post_spikes, {
                "spike_times": [1.]
            })

        nest.Simulate(51.)
        nest.SetStatus(self.syn, {
                'w_jk': [0.],
                'w_create_steps': [150]
            })

        nest.Simulate(50.)
        stat = nest.GetStatus(self.syn)[0]
        self.assertEqual(stat['w_jk'][0], 0.)
        self.assertEqual(stat['w_create_steps'][0], 100)
        self.assertTrue(math.isnan(stat['r_jk'][0]))
        self.assertTrue(math.isnan(stat['c_jk'][0]))
        self.assertTrue(math.isnan(stat['r_post_jk'][0]))
        self.assertTrue(math.isnan(stat['R_post_jk'][0]))

    def setUp_decay(self, params={}, n=1):
        """Set up a net with pre and post parrots connected
        by the SPL synapse"""

        syn_spec = {
            "model": "testsyn",
            "receptor_type": 1,
        }
        conn_spec = {
            "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        self.setUp_net(n, params)
        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        self.syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")

        # set the initial weight of all contacts to 0.1
        for syn_ in self.syn:
            n_pot_conns_ = nest.GetStatus([syn_],
                                          keys=['n_pot_conns'])[0][0]
            nest.SetStatus([syn_], params='w_jk',
                           val=[(np.ones(n_pot_conns_) * 0.1)])

    def setUp_net(self, n_post, params={}):
        """Set up a net and parrots"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for
        # delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [120. - delay, 140. - delay, 160. - delay]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {
                                 "spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {
                                  "spike_times": post_times})

        self.pre_spikes = pre_spikes
        self.post_spikes = post_spikes

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", n_post)

        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={
                     "delay": delay}, conn_spec={"rule": "all_to_all"})

        # create spike detector
        self.spikes = nest.Create("spike_detector")
        nest.Connect(self.pre_parrot, self.spikes,
                     conn_spec={"rule": "all_to_all"})
        nest.Connect(self.post_parrot, self.spikes,
                     conn_spec={"rule": "all_to_all"})

        pars = {
            'tau': 0.02,
            'tau_slow': 0.300,
            'w0': .2,
            'p_fail': .2
        }

        pars.update(params)
        nest.CopyModel('stdp_structpl_synapse_hom', 'testsyn', pars)

    def setUp_iafpost(self, params={}):
        """Total transmitted weight per spike is stochastic sum of
        individual weights"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for
        # delays)
        pre_times = [100. - delay, 200. - delay, 300. - delay]

        # create spike_generators with these times
        self.pre_spikes = nest.Create("spike_generator", 1, {
            "spike_times": pre_times})

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        nest.Connect(self.pre_spikes, self.pre_parrot,
                     syn_spec={"delay": delay})

        # create a iaf_psc_delta postsynaptic neuron
        self.post = nest.Create("iaf_psc_delta", 1)
        self.rec = nest.Create('multimeter', params={
            'record_from': ['V_m'], 'withtime': True})
        nest.Connect(self.rec, self.post)

        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
        }
        pars.update(params)

        nest.CopyModel('stdp_structpl_synapse_hom', 'testsyn', pars)
        syn_spec = {
            "model": "testsyn",
        }
        conn_spec = {
            "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        nest.Connect(
            self.pre_parrot, self.post, syn_spec=syn_spec, conn_spec=conn_spec)
        self.syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")

    def assertAlmostEqualDetailed(self, expected, given, message, places=7):
        """Improve assetAlmostEqual with detailed message. by Teo Stocco."""
        messageWithValues = "%s (expected: `%s` was: `%s`" % (
            message, str(expected), str(given))
        self.assertAlmostEqual(
            given, expected, msg=messageWithValues, places=places)

    def test_spike_multiplicity_pre(self):
        """Multiplicity of presynpatic spikes is correcly reproduced"""

        """ TODO add here true spike multiplicity. right now this just sends
        multiple spikes in the same timestep"""

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for
        # delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [150. - delay]

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", 1)

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {
                                 "spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {
                                  "spike_times": post_times})

        # connect twice
        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})

        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'tau': 10.
        }
        nest.CopyModel('stdp_structpl_synapse_hom', 'testsyn', pars)

        syn_spec = {
            "model": "testsyn",
            "receptor_type": 1,
        }

        conn_spec = {
            "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")

        nest.Simulate(150.)

        syn_defaults = nest.GetDefaults('testsyn')
        val_exp = 1. / syn_defaults['tau'] * 2.
        val = nest.GetStatus(syn)[0]['r_jk'][0]
        self.assertAlmostEqualDetailed(
            val_exp, val, "Multiple presynaptic spikes not treated properly")

    def test_spike_multiplicity_post(self):
        """Multiplicity of postsynaptic spikes is correcly reproduced"""

        """ TODO add here true spike multiplicity. right now this just sends
        multiple spikes in the same timestep"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for
        # delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [150. - delay]

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", 1)

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {
                                 "spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {
                                  "spike_times": post_times})

        # connect twice
        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})

        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'tau': 10.
        }
        nest.CopyModel('stdp_structpl_synapse_hom', 'testsyn', pars)

        syn_spec = {
            "model": "testsyn",
            "receptor_type": 1,
        }

        conn_spec = {
            "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        nest.Connect(self.pre_parrot, self.post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(
            source=self.pre_parrot, synapse_model="testsyn")

        nest.Simulate(210.)

        syn_defaults = nest.GetDefaults('testsyn')
        tau = syn_defaults['tau']
        tau_slow = syn_defaults['tau_slow']
        dt = nest.GetKernelStatus()['resolution'] * 0.001

        val_exp_r_post = 1. / syn_defaults['tau'] * 2. * np.exp(-dt / tau)**50.
        val_r_post = nest.GetStatus(syn)[0]['r_post_jk'][0]

        val_exp_R_post = 1. / \
            syn_defaults['tau_slow'] * 2. * np.exp(-dt / tau_slow)**50.
        val_R_post = nest.GetStatus(syn)[0]['R_post_jk'][0]

        self.assertAlmostEqualDetailed(
            val_exp_r_post,
            val_r_post,
            "r_post does not integrate multiple postsynaptic spikes properly"
        )
        self.assertAlmostEqualDetailed(
            val_exp_R_post,
            val_R_post,
            "R_post does not integrate multiple postsynaptic spikes properly"
        )


def suite():
    suite = unittest.makeSuite(StdpStructplSynapseTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
    # suite = unittest.TestSuite()
    # suite.addTest(StdpStructplSynapseTestCase("test_deletion_manual"))
    # runner = unittest.TextTestRunner(verbosity=2)
    # runner.run(suite)
