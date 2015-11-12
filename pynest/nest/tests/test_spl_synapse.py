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
import unittest


@nest.check_stack
class SplSynapseTestCase(unittest.TestCase):
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
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(source=self.pre_parrot, synapse_model="testsyn")
        nest.SetStatus(syn, {'n_pot_conns': 1})
        nest.Simulate(210.)
        syn_status = nest.GetStatus(syn)
        assert len(syn_status[0]['w_jk']) == 1, "Resizing failed"

        # resizing potential conns
        self.setUp_net(1)
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(source=self.pre_parrot, synapse_model="testsyn")
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
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(source=self.pre_parrot, synapse_model="testsyn")
        assert len(syn) == 2, "Not enought connections created"

        npot = [1, 2]
        for i, s in enumerate(syn):
            nest.SetStatus([s], {'n_pot_conns': npot[i]})

        nest.Simulate(210.)

        syn_status = nest.GetStatus(syn)
        assert len(syn_status[0]['w_jk']) == 1 \
            and len(syn_status[1]['w_jk']) == 2, "Asymmetric resizing failed"

    def test_decay_rpost(self):
        """Decay of r_post without inputs"""
        self.setUp_decay()
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['r_post']

        dt = 0.001
        tau = syn_defaults['tau']
        prop = np.exp(-dt/tau)

        spike0 = 1./tau * prop**80.  # propagate spike 1 by 80ms
        spike1 = 1./tau * prop**60.  # propagate spike 2 by 60ms
        spike3 = 1./tau * prop**40.  # propagate first spike 40ms
        val_exp = spike0 + spike1 + spike3

        self.assertAlmostEqualDetailed(val_exp, val, "Decay of r_post not as expected.")

    def test_decay_Rpost(self):
        """Decay of R_post without inputs"""
        self.setUp_decay()
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['R_post']
        dt = 0.001
        tau = syn_defaults['tau_slow']
        prop = np.exp(-dt/tau)

        spike0 = 1./tau * prop**80.  # propagate spike 1 by 80ms
        spike1 = 1./tau * prop**60.  # propagate spike 2 by 60ms
        spike3 = 1./tau * prop**40.  # propagate first spike 40ms
        val_exp = spike0 + spike1 + spike3

        self.assertAlmostEqualDetailed(val_exp, val, "Decay of R_post not as expected.")

    def test_decay_cjk(self):
        """Decay of c_jk without inputs"""
        self.setUp_decay(params={'p_fail': 1.})  # ensures r_jk = 0, and thus only decay
        nest.SetStatus(self.syn, {'c_jk': [1.]})

        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['c_jk']
        dt = 0.001
        tau = syn_defaults['tau_slow']
        prop = np.exp(-dt/tau)

        val_exp = 1. * prop**200.

        self.assertAlmostEqualDetailed(val_exp, val, "Decay of c_jk not as expected.")

    def test_decay_Rpre_deterministic(self):
        """Decay and passing for p_fail=0"""
        self.setUp_decay(params={'p_fail': 0.})
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['r_jk']
        dt = 0.001
        tau = syn_defaults['tau']
        prop = np.exp(-dt/tau)

        spike0 = 1./tau * prop**100.  # propagate spike 1 by 100ms
        val_exp = spike0 + 1./tau  # second spike increases this value
        self.assertAlmostEqualDetailed(val_exp, val, "Decay of r_jk not as expected (deterministic case).")

    def test_decay_Rpre_allfail(self):
        """No spikes pass for p_fail==1"""
        self.setUp_decay(params={'p_fail': 1.})
        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        val = syn_status[0]['r_jk'][0]
        spike0 = 0.  # no spike passes
        val_exp = spike0
        self.assertAlmostEqualDetailed(val_exp, val, "Decay of r_jk not as expected (no spike case).")

    def test_decay_Rpre_half_fail(self):
        """Stochastic and selective increase of r_jk, plus decay"""
        self.setUp_decay(params={'p_fail': .2, 'n_pot_conns': 2})

        # this seed lets the first spike on syn 0 and the second spike on syn 1 pass
        msd = 4
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus({'rng_seeds': range(msd+N_vp+1, msd+2*N_vp+1)})

        nest.Simulate(210.)

        syn_status = nest.GetStatus(self.syn)
        syn_defaults = nest.GetDefaults('testsyn')
        val = syn_status[0]['r_jk']
        dt = 0.001
        tau = syn_defaults['tau']
        prop = np.exp(-dt/tau)

        spike0 = 1./tau * prop**100.  # propagate spike 1 by 100ms
        val_exp = [spike0, 1./tau]  # no spike increase on 1, only second spike increase on 2
        for k in range(len(val)):
            self.assertAlmostEqualDetailed(val_exp[k], val[k], "Decay of r_jk[%i] not as expected (stochastic case)" % k)

    def test_create_time(self):
        """Weight creation and decay of creation steps"""
        self.setUp_decay(params={
            'p_fail': 1.,
            'n_pot_conns': 1,
            'w_jk': [0.]
        })

        syn_defaults = nest.GetDefaults('testsyn')
        w0 = syn_defaults['w0']

        # this seed lets the first spike on syn 0 and the second spike on syn 1 pass
        msd = 4
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus({'rng_seeds': range(msd+N_vp+1, msd+2*N_vp+1)})

        nest.Simulate(101.)
        syn_status = nest.GetStatus(self.syn)
        nest.SetStatus(self.syn, {'w_create_steps': [100]})

        nest.Simulate(100.)
        syn_status = nest.GetStatus(self.syn)

        val_steps = syn_status[0]['w_create_steps'][0]
        val_w = syn_status[0]['w_jk'][0]
        val_exp_steps = 0
        val_exp_w = w0

        self.assertAlmostEqualDetailed(val_exp_w, val_w, "Weight upon intialization not correct.")
        self.assertAlmostEqualDetailed(val_exp_steps, val_steps, "Creation steps do not decay as intended.")

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
        prop = np.exp(-dt*alpha)

        val = syn_status[0]['w_jk'][0]
        val_exp = 1. * prop**200.

        self.assertAlmostEqualDetailed(val_exp, val, "Weight decay (alpha) not correct.")

    def test_w_cjk_terms(self):
        """Dynamics on w_jk with correlation terms"""
        self.setUp_decay(params={
            'p_fail': 0.,
            'n_pot_conns': 1,
            'A2_corr': 10.,
            'A4_corr': 10.,
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
            r_jk[i] = r_jk[i-1] * np.exp(-dt/tau)
            if i in pre_times:
                r_jk[i] += 1./tau

        r_post = np.zeros(201)
        for i in range(1, 201):
            r_post[i] = r_post[i-1] * np.exp(-dt/tau)
            if i in post_times:
                r_post[i] += 1./tau

        c_jk = np.zeros(201)
        for i in range(1, 201):
            c_jk[i] = c_jk[i-1] * np.exp(-dt/tau_slow)
            c_jk[i] = c_jk[i] + (-np.expm1(-dt/tau_slow)) * (r_post[i-1] * r_jk[i-1])

        w_jk = np.zeros(201)
        w_jk[0] = 1.
        for i in range(1, 201):
            w_jk[i] = w_jk[i-1] + dt * (A2_corr * c_jk[i-1] - A4_corr * c_jk[i-1]**2)

        val = syn_status[0]['w_jk'][0]
        val_exp = w_jk[-1]

        self.assertAlmostEqualDetailed(val_exp, val, "Weight dynamics with correlation terms not correct")

    def test_w_vanish_and_recover(self):
        """Inactivating and recreating connections from the pool of potential connections"""
        self.setUp_decay(params={
            'p_fail': 0.,
            'n_pot_conns': 1,
            'A2_corr': 0.,
            'A4_corr': 1e8,
            'A4_post': 0.,
            'alpha': 0.,
            'lambda': 1./30.*1e3,  # units in [s]
        })

        # this seed lets the first spike on syn 0 and the second spike on syn 1 pass
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus({'rng_seeds': range(msd+N_vp+1, msd+2*N_vp+1)})

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
            r_jk[i] = r_jk[i-1] * np.exp(-dt/tau)
            if i in pre_times:
                r_jk[i] += 1./tau

        r_post = np.zeros(201)
        for i in range(1, 201):
            r_post[i] = r_post[i-1] * np.exp(-dt/tau)
            if i in post_times:
                r_post[i] += 1./tau

        c_jk = np.zeros(201)
        for i in range(1, 201):
            c_jk[i] = c_jk[i-1] * np.exp(-dt/tau_slow)
            c_jk[i] = c_jk[i] + dt/tau_slow * (r_post[i-1] * r_jk[i-1])

        pauses = [43, 85]
        w_jk = np.zeros(201)
        w_jk[0] = 1.
        for i in range(1, 201):
            w_jk[i] = w_jk[i-1] + dt * (A2_corr * c_jk[i-1] - A4_corr * c_jk[i-1]**2)

        idxs = np.where(w_jk <= 0.)[0][0]
        val = syn_status[0]['w_create_steps'][0]

        # all pauses ==
        # time from first zero crossing - 1 timestep where synapse
        # gets set to zero again (due to abnormally large A4_cross)
        val_exp = pauses[0] + pauses[1] - (200 - idxs - 1)

        self.assertAlmostEqualDetailed(val_exp, val, "Zero crossing and synapse creation not correct.")

    def test_w_post_terms(self):
        """Dynamics on w_jk with postsynaptic terms only"""
        self.setUp_decay(params={
            'p_fail': 0.,
            'n_pot_conns': 1,
            'A2_corr': 0.,
            'A4_corr': 0.,
            'A4_post': 10.,
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
            R_post[i] = R_post[i-1] * np.exp(-dt/tau_slow)
            if i in post_times:
                R_post[i] += 1./tau_slow

        w_jk = np.zeros(201)
        w_jk[0] = 1.
        for i in range(1, 201):
            w_jk[i] = w_jk[i-1] - dt * A4_post * R_post[i-1]**4

        val = syn_status[0]['w_jk'][0]
        val_exp = w_jk[-1]

        self.assertAlmostEqualDetailed(val_exp, val, "Weight dynamics on postsynaptic terms incorrect")

    def test_postsynaptic_total_weight(self):
        """Total transmitted weight per spike is stochastic sum of individual weights"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for delays)
        pre_times = [100. - delay, 200. - delay]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})

        # create parrot neurons and connect spike_generators
        pre_parrot = nest.Create("parrot_neuron", 1)
        nest.Connect(pre_spikes, pre_parrot, syn_spec={"delay": delay})

        # create a iaf_psc_delta postsynaptic neuron
        post = nest.Create("iaf_psc_delta", 1)
        rec = nest.Create('multimeter', params={'record_from': ['V_m'], 'withtime': True})
        nest.Connect(rec, post)

        pars = {
            'p_fail': 0.5,
            'n_pot_conns': 10,
        }
        nest.CopyModel('stdp_spl_synapse_hom', 'testsyn', pars)
        syn_spec = {
           "model": "testsyn",
        }
        conn_spec = {
           "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        nest.Connect(pre_parrot, post, syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(source=pre_parrot, synapse_model="testsyn")

        # set seed to something reproducible
        msd = 2
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'grng_seed': msd + N_vp})
        nest.SetKernelStatus({'rng_seeds': range(msd+N_vp+1, msd+2*N_vp+1)})

        nest.Simulate(150.)
        w_jk_0 = nest.GetStatus(syn)[0]['w_jk']
        # set the voltage to resting potential to exclude errors further down
        nest.SetStatus(post, {'V_m': -70.})

        nest.Simulate(150.)
        w_jk_1 = nest.GetStatus(syn)[0]['w_jk']

        events = nest.GetStatus(rec)[0]['events']
        t = events['times']

        # first has spikes in idx [8, 9]
        val_0 = events['V_m'][t == 101.] - events['V_m'][t == 100.]
        val_exp_0 = np.array(w_jk_0)[[8, 9]].sum()

        # first has spikes in idx [0, 3, 6]
        val_1 = events['V_m'][t == 201.] - events['V_m'][t == 200.]
        val_exp_1 = np.array(w_jk_1)[[0, 3, 6]].sum()

        self.assertAlmostEqualDetailed(val_exp_0, val_0, "Transmitted weights not stochastic/correct, first spike.")
        self.assertAlmostEqualDetailed(val_exp_1, val_1, "Transmitted weights not stochastic/correct, second spike.")

    def setUp_decay(self, params={}, n=1):
        """Set up a net with pre and post parrots connected by the SPL synapse"""
        syn_spec = {
           "model": "testsyn",
           "receptor_type": 1,
        }
        conn_spec = {
           "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        self.setUp_net(n, params)
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)
        self.syn = nest.GetConnections(source=self.pre_parrot, synapse_model="testsyn")
        
        # set the initial weight of all contacts to 0.1
        for syn_ in self.syn:
            n_pot_conns_ = nest.GetStatus( [syn_], \
                keys=['n_pot_conns'] )[0][0]
            nest.SetStatus( [syn_], params='w_jk', \
                val=[(np.ones(n_pot_conns_)*0.1)] )
        

    def setUp_net(self, n_post, params={}):
        """Set up a net and parrots"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [120. - delay, 140. - delay, 160. - delay]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {"spike_times": post_times})

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", n_post)

        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay}, conn_spec={"rule": "all_to_all"})

        # create spike detector
        self.spikes = nest.Create("spike_detector")
        nest.Connect(self.pre_parrot, self.spikes, conn_spec={"rule": "all_to_all"})
        nest.Connect(self.post_parrot, self.spikes, conn_spec={"rule": "all_to_all"})

        pars = {
            'tau': 20.,
            'tau_slow': 300.,
            'w0': .1,
            'p_fail': .2
        }

        pars.update(params)
        nest.CopyModel('stdp_spl_synapse_hom', 'testsyn', pars)

    def assertAlmostEqualDetailed(self, expected, given, message):
        """Improve assetAlmostEqual with detailed message. by Teo Stocco."""
        messageWithValues = "%s (expected: `%s` was: `%s`" % (message, str(expected), str(given))
        self.assertAlmostEqual(given, expected, msg=messageWithValues)

    def test_spike_multiplicity_pre(self):
        """Multiplicity of presynpatic spikes is correcly reproduced"""

        """ TODO add here true spike multiplicity. right now this just sends multiple spikes in the same timestep"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [150. - delay]

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", 1)

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {"spike_times": post_times})

        # connect twice
        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})

        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'tau': 10.
        }
        nest.CopyModel('stdp_spl_synapse_hom', 'testsyn', pars)

        syn_spec = {
           "model": "testsyn",
           "receptor_type": 1,
        }

        conn_spec = {
           "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(source=self.pre_parrot, synapse_model="testsyn")

        nest.Simulate(150.)

        syn_defaults = nest.GetDefaults('testsyn')
        val_exp = 1./syn_defaults['tau'] * 2.
        val = nest.GetStatus(syn)[0]['r_jk'][0]
        self.assertAlmostEqualDetailed(val_exp, val, "Multiple presynaptic spikes not treated properly")

    def test_spike_multiplicity_post(self):
        """Multiplicity of postsynaptic spikes is correcly reproduced"""

        """ TODO add here true spike multiplicity. right now this just sends multiple spikes in the same timestep"""
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections

        # set the correct real spike times for generators (correcting for delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [150. - delay]

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", 1)

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {"spike_times": post_times})

        # connect twice
        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})

        pars = {
            'p_fail': 0.,
            'n_pot_conns': 1,
            'tau': 10.
        }
        nest.CopyModel('stdp_spl_synapse_hom', 'testsyn', pars)

        syn_spec = {
           "model": "testsyn",
           "receptor_type": 1,
        }

        conn_spec = {
           "rule": "all_to_all",
        }

        # two syns, different number of potential conns
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)
        syn = nest.GetConnections(source=self.pre_parrot, synapse_model="testsyn")

        nest.Simulate(210.)

        syn_defaults = nest.GetDefaults('testsyn')
        tau = syn_defaults['tau']
        tau_slow = syn_defaults['tau_slow']
        dt = nest.GetKernelStatus()['resolution'] * 0.001

        val_exp_r_post = 1./syn_defaults['tau'] * 2. * np.exp(-dt/tau)**50.
        val_r_post = nest.GetStatus(syn)[0]['r_post']

        val_exp_R_post = 1./syn_defaults['tau_slow'] * 2. * np.exp(-dt/tau_slow)**50.
        val_R_post = nest.GetStatus(syn)[0]['R_post']

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

    suite = unittest.makeSuite(SplSynapseTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
