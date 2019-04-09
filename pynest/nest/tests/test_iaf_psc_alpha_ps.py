# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_ps.py
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

import unittest

import numpy as np

import nest


"""
Comparing the new implementations the precise ``iaf_psc_alpha_ps`` model to the
previous reference implementation given by ``iaf_psc_alpha_canon``.

The new implementation provides two separate buffer for excitatory and
inhibitory spikes, allowing the use of 2 different synaptic timescales.

Rationale of the test:
  - models are identical with respect to currents and spikes when
  ``tau_syn_ex == tau_syn_in``,
  - models differ with respect to spikes if ``tau_syn_ex != tau_syn_in``.

Details:
  The models are compared and we assess that the difference between the
  recorded variables and the reference is smaller than a given tolerance.
"""


class IAFPreciseTestCase(unittest.TestCase):
    """
    Check the coherence between new and previous implementations.
    """

    def setUp(self):
        '''
        Clean up and initialize NEST before each test.
        '''
        msd = 123456
        self.resol = 0.01
        nest.ResetKernel()
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        pyrngs = [np.random.RandomState(s) for s in range(msd, msd + N_vp)]
        nest.SetKernelStatus({
            'resolution': self.resol, 'grng_seed': msd + N_vp,
            'rng_seeds': range(msd + N_vp + 1, msd + 2 * N_vp + 1)})

    def compute_difference(self, multimeters, params, reference, recordables):
        '''
        Compute the relative differences between the values recorded by the
        multimeter and those of the reference (recorded at same times).

        Parameters
        ----------
        multimeters : dict of tuples
            Dictionary containing the model name as key and the GID of the
            associated multimeter as value.
        params : dict
            Parameters used for the models.
        reference : dict
            Reference arrays (one per entry in `recordables`).
        recordables : list of strings
            List of recordables that will be compared.

        Returns
        -------
        rel_diff : dict of dict of doubles
            Relative differences between recorded data and reference (one dict
            per model, containing one value per entry in `recordables`).
        '''
        rel_diff = {model: {} for model in multimeters.keys()}
        V_lim = (params["V_th"] + params["V_peak"]) / 2.

        for model, mm in iter(multimeters.items()):
            dmm = nest.GetStatus(mm, "events")[0]
            for record in recordables:
                # ignore places where a divide by zero would occur
                rds = np.abs(reference[record] - dmm[record])
                nonzero = np.where(~np.isclose(reference[record], 0.))[0]
                if np.any(nonzero):
                    rds = rds[nonzero] / np.abs(reference[record][nonzero])
                # ignore events around spike times for V if it diverges
                if record == "V_m" and params["Delta_T"] > 0.:
                    spiking = (dmm[record] > V_lim)
                    rds = rds[~spiking]
                rel_diff[model][record] = np.average(rds)
        return rel_diff

    def test_closeness_same_timescales(self):
        '''
        Compare models for tau_syn_ex == tau_syn_in (should be equal).
        '''
        Ie = 370.

        ref = nest.Create("iaf_psc_alpha_canon", params={"I_e": Ie})
        new = nest.Create("iaf_psc_alpha_ps", params={"I_e": Ie})

        espikes = nest.Create("spike_generator", params={"spike_times": [
                              10., 100., 500.], "spike_weights": [20.]*3})
        ispikes = nest.Create("spike_generator", params={"spike_times": [
                              15., 99., 200.], "spike_weights": [-20.]*3})

        nest.Connect(espikes, ref)
        nest.Connect(ispikes, ref)
        nest.Connect(espikes, new)
        nest.Connect(ispikes, new)

        mm_ref = nest.Create("multimeter", params={"record_from": ["V_m"]})
        mm_new = nest.Create("multimeter", params={
                             "record_from": ["V_m", "I_syn_ex", "I_syn_in"]})

        nest.Connect(mm_ref, ref)
        nest.Connect(mm_new, new)

        nest.Simulate(1000.)

        data_ref = nest.GetStatus(mm_ref, "events")[0]
        data_new = nest.GetStatus(mm_new, "events")[0]

        self.assertTrue(np.allclose(data_new["V_m"], data_ref["V_m"]))

    def test_unequal_different_timescales(self):
        '''
        Check that the behavior differs if ``tau_syn_in != tau_syn_ex``
        '''
        Ie = 370.

        ref = nest.Create("iaf_psc_alpha_canon", params={"I_e": Ie})
        new = nest.Create("iaf_psc_alpha_ps", params={
                          "I_e": Ie, "tau_syn_in": 5.})

        espikes = nest.Create("spike_generator", params={"spike_times": [
                              10., 100., 500.], "spike_weights": [20.]*3})
        ispikes = nest.Create("spike_generator", params={
                              "spike_times": [500.], "spike_weights": [-20.]})

        nest.Connect(espikes, ref)
        nest.Connect(ispikes, ref)
        nest.Connect(espikes, new)
        nest.Connect(ispikes, new)

        mm_ref = nest.Create("multimeter", params={"record_from": ["V_m"]})
        mm_new = nest.Create("multimeter", params={
                             "record_from": ["V_m", "I_syn_ex", "I_syn_in"]})

        nest.Connect(mm_ref, ref)
        nest.Connect(mm_new, new)

        nest.Simulate(1000.)

        data_ref = nest.GetStatus(mm_ref, "events")[0]
        data_new = nest.GetStatus(mm_new, "events")[0]

        # test before 500.
        keep = data_new["times"] < 500.

        self.assertTrue(np.allclose(
            data_new["V_m"][keep], data_ref["V_m"][keep]))
        self.assertFalse(np.allclose(
            data_new["V_m"][~keep], data_ref["V_m"][~keep]))


def suite():
    return unittest.makeSuite(IAFPreciseTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
