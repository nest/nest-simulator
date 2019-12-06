# -*- coding: utf-8 -*-
#
# test_aeif_lsodar.py
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

import os
import sys
import unittest

import numpy as np
from scipy.interpolate import interp1d

from collections import defaultdict

import nest

"""
Comparing the new implementations the aeif models to the reference solution
obrained using the LSODAR solver (see
``doc/model_details/aeif_models_implementation.ipynb``). Also asserting that
the ``Delta_T == 0.`` solution works by comparing the non-adaptive aeif to the
iaf model.

The reference solution is stored in ``test_aeif_psc_lsodar.dat`` and was
generated using the same dictionary of parameters, the data is then downsampled
to keep one value every 0.01 ms and compare with the NEST simulation.

The new implementation binds V_m to be smaller than 0.

Rationale of the test:
  - all models should be close to the reference LSODAR when
    submitted to the same excitatory current.
  - for ``Delta_T = 0.``, ``a = 0.`` and ``b = 0.``, starting from ``w = 0.``,
    models should behave as the associated iaf model. This is tested both with
    spike input and direct current input.

Details:
  The models are compared and we assess that the difference between the
  recorded variables and the reference is smaller than a given tolerance.
"""

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")
path = os.path.abspath(os.path.dirname(__file__))

# --------------------------------------------------------------------------- #
#  Tolerances to compare LSODAR and NEST implementations
# -------------------------
#

# higher for the potential because of the divergence at spike times
di_tolerances_lsodar = {
    "aeif_cond_alpha": {"V_m": 5e-4, "w": 1e-4},
    "aeif_cond_exp": {"V_m": 5e-4, "w": 1e-4},
    "aeif_psc_alpha": {"V_m": 5e-4, "w": 1e-4},
    "aeif_psc_exp": {"V_m": 5e-4, "w": 1e-4},
    "aeif_psc_delta": {"V_m": 5e-4, "w": 1e-4},
    "aeif_cond_alpha_multisynapse": {"V_m": 5e-4, "w": 1e-4},
    "aeif_cond_beta_multisynapse": {"V_m": 5e-4, "w": 1e-4}
}

# "high" difference for V_m on aeif_psc_* because of 1 gap in spike-time on
# the 3rd spike
di_tolerances_iaf = {
    "aeif_cond_alpha": {"V_m": 2e-3, "g_ex": 1e-6},
    "aeif_cond_exp": {"V_m": 5e-4, "g_ex": 1e-6},
    "aeif_psc_alpha": {"V_m": 5e-3, "I_syn_ex": 1e-6},
    "aeif_psc_delta": {"V_m": 5e-3},
    "aeif_psc_exp": {"V_m": 5e-3, "I_syn_ex": 1e-6}
}


# --------------------------------------------------------------------------- #
#  Individual dynamics
# -------------------------
#

models = [
    "aeif_cond_alpha",
    "aeif_cond_exp",
    "aeif_psc_alpha",
    "aeif_psc_delta",
    "aeif_psc_exp",
    "aeif_cond_alpha_multisynapse",
    "aeif_cond_beta_multisynapse"
]

num_models = len(models)

# parameters with which the LSODAR reference solution was generated
aeif_param = {
    'V_reset': -58.,
    'V_peak': 0.0,
    'V_th': -50.,
    'I_e': 420.,
    'g_L': 11.,
    'tau_w': 300.,
    'E_L': -70.,
    'Delta_T': 2.,
    'a': 3.,
    'b': 0.,
    'C_m': 200.,
    'V_m': -70.,
    'w': 5.
}

# parameters to compare with iaf_cond_*
aeif_DT0 = {
    'V_reset': -60.,
    'V_peak': 0.0,
    'V_th': -55.,
    'I_e': 200.,
    'g_L': 16.7,
    'E_L': -70.,
    'Delta_T': 0.,
    'a': 0.,
    'b': 0.,
    'C_m': 250.,
    'V_m': -70.,
    'w': 0.,
    't_ref': 2.,
    'tau_syn_ex': 0.2
}

aeif_DT0_delta = aeif_DT0.copy()
aeif_DT0_delta.pop('tau_syn_ex')

DT0_params = defaultdict(lambda: aeif_DT0)
DT0_params["aeif_psc_delta"] = aeif_DT0_delta

iaf_param_base = {
    'V_reset': -60.,
    'V_th': -55.,
    'I_e': 200.,
    'E_L': -70.,
    'C_m': 250.,
    'V_m': -70.,
    't_ref': 2.,
    'tau_syn_ex': 0.2,
}

iaf_param_psc = iaf_param_base.copy()
iaf_param_psc.update({'tau_m': aeif_DT0['C_m'] / aeif_DT0['g_L']})
iaf_param_psc_delta = iaf_param_psc.copy()
iaf_param_psc_delta.pop('tau_syn_ex')
iaf_param_cond = iaf_param_base.copy()
iaf_param_cond.update({'g_L': 16.7})

di_iaf_param = {
    "psc_alpha": iaf_param_psc,
    "psc_exp": iaf_param_psc,
    "psc_delta": iaf_param_psc_delta,
    "cond_alpha": iaf_param_cond,
    "cond_exp": iaf_param_cond
}


# --------------------------------------------------------------------------- #
#  Synaptic types (link the models to their synapse dynamics)
# -------------------------
#

lst_syn_types = ["cond_alpha", "cond_exp", "psc_alpha", "psc_exp", "psc_delta"]

di_syn_types = {
    "aeif_cond_alpha": "cond_alpha",
    "aeif_cond_exp": "cond_exp",
    "aeif_psc_alpha": "psc_alpha",
    "aeif_psc_exp": "psc_exp",
    "aeif_psc_delta": "psc_delta"
}


# --------------------------------------------------------------------------- #
#  Test class
# -------------------------
#

class AEIFTestCase(unittest.TestCase):
    """
    Check the coherence between reference solution and NEST implementation.
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
            Dictionary containing the model name as key and the node ID of the
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

    def assert_pass_tolerance(self, rel_diff, di_tol):
        '''
        Test that relative differences are indeed smaller than the tolerance.
        '''
        for model, di_rel_diff in iter(rel_diff.items()):
            for var, diff in iter(di_rel_diff.items()):
                self.assertLess(diff, di_tol[model][var],
                                "{} failed test for {}: {} > {}.".format(
                                    model, var, diff, di_tol[model][var]))

    @unittest.skipIf(not HAVE_GSL, 'GSL is not available')
    def test_closeness_nest_lsodar(self):
        # Compare models to the LSODAR implementation.

        simtime = 100.

        # get lsodar reference
        lsodar = np.loadtxt(os.path.join(path, 'test_aeif_data_lsodar.dat')).T
        V_interp = interp1d(lsodar[0, :], lsodar[1, :])
        w_interp = interp1d(lsodar[0, :], lsodar[2, :])

        # create the neurons and devices
        neurons = {model: nest.Create(model, params=aeif_param)
                   for model in models}
        multimeters = {model: nest.Create("multimeter") for model in models}
        # connect them and simulate
        for model, mm in iter(multimeters.items()):
            nest.SetStatus(mm, {"interval": self.resol,
                                "record_from": ["V_m", "w"]})
            nest.Connect(mm, neurons[model])
        nest.Simulate(simtime)

        # relative differences: interpolate LSODAR to match NEST times
        mm0 = next(iter(multimeters.values()))
        nest_times = nest.GetStatus(mm0, "events")[0]["times"]
        reference = {'V_m': V_interp(nest_times), 'w': w_interp(nest_times)}

        rel_diff = self.compute_difference(multimeters, aeif_param, reference,
                                           ['V_m', 'w'])
        self.assert_pass_tolerance(rel_diff, di_tolerances_lsodar)

    @unittest.skipIf(not HAVE_GSL, 'GSL is not available')
    def test_iaf_spike_input(self):
        # Test that the models behave as iaf_* if a == 0., b == 0. and
        # Delta_T == 0 due to random spike input.

        simtime = 200.
        # create the neurons and devices
        refs = {syn_type: nest.Create("iaf_" + syn_type,
                params=di_iaf_param[syn_type]) for syn_type in lst_syn_types}
        ref_mm = {syn_type: nest.Create("multimeter") for syn_type in refs}
        neurons = {model: nest.Create(model, params=DT0_params[model])
                   for model in models if "multisynapse" not in model}
        multimeters = {model: nest.Create("multimeter") for model in neurons}
        pg = nest.Create("poisson_generator", params={"rate": 10000.})
        pn = nest.Create("parrot_neuron")

        # connect them and simulate
        recordables = {"cond_alpha": ["V_m", "g_ex"],
                       "cond_exp": ["V_m", "g_ex"],
                       "psc_exp": ["V_m", "I_syn_ex"],
                       "psc_alpha": ["V_m", "I_syn_ex"],
                       "psc_delta": ["V_m"]}
        nest.Connect(pg, pn)
        for model, mm in iter(multimeters.items()):
            syn_type = di_syn_types[model]
            key = syn_type[:syn_type.index('_')]
            nest.SetStatus(mm, {"interval": self.resol,
                                "record_from": recordables[syn_type]})
            nest.Connect(mm, neurons[model])
            weight = 80. if key == "psc" else 1.
            nest.Connect(pn, neurons[model], syn_spec={'weight': weight})
        for syn_type, mm in iter(ref_mm.items()):
            key = syn_type[:syn_type.index('_')]
            nest.SetStatus(mm, {"interval": self.resol,
                                "record_from": recordables[syn_type]})
            nest.Connect(mm, refs[syn_type])
            weight = 80. if key == "psc" else 1.
            nest.Connect(pn, refs[syn_type], syn_spec={'weight': weight})
        nest.Simulate(simtime)

        # compute the relative differences and assert tolerance
        for model in neurons:
            syn_type = di_syn_types[model]
            ref_data = nest.GetStatus(ref_mm[syn_type], "events")[0]
            key = syn_type[:syn_type.index('_')]
            rel_diff = self.compute_difference(
                {model: multimeters[model]},
                aeif_DT0,
                ref_data,
                recordables[syn_type])
            self.assert_pass_tolerance(rel_diff, di_tolerances_iaf)

    @unittest.skipIf(not HAVE_GSL, 'GSL is not available')
    def test_iaf_dc_input(self):
        # Test that the models behave as iaf_* if a == 0., b == 0. and
        # Delta_T == 0 due to direct current input.

        simtime = 200.
        # create the neurons and devices
        refs = {syn_type: nest.Create("iaf_" + syn_type,
                params=di_iaf_param[syn_type]) for syn_type in lst_syn_types}
        ref_mm = {syn_type: nest.Create("multimeter") for syn_type in refs}
        neurons = {model: nest.Create(model, params=DT0_params[model])
                   for model in models if "multisynapse" not in model}
        multimeters = {model: nest.Create("multimeter") for model in neurons}
        dcg = nest.Create("dc_generator", params={"amplitude": 250.0,
                                                  "start": 50.0,
                                                  "stop": 150.0})

        # connect them and simulate
        for model, mm in iter(multimeters.items()):
            syn_type = di_syn_types[model]
            nest.SetStatus(mm, {"interval": self.resol,
                                "record_from": ["V_m"]})
            nest.Connect(mm, neurons[model])
            nest.Connect(dcg, neurons[model])
        for syn_type, mm in iter(ref_mm.items()):
            nest.SetStatus(mm, {"interval": self.resol,
                                "record_from": ["V_m"]})
            nest.Connect(mm, refs[syn_type])
            nest.Connect(dcg, refs[syn_type])
        nest.Simulate(simtime)

        # compute the relative differences and assert tolerance
        for model in neurons:
            syn_type = di_syn_types[model]
            ref_data = nest.GetStatus(ref_mm[syn_type], "events")[0]
            rel_diff = self.compute_difference(
                {model: multimeters[model]},
                aeif_DT0,
                ref_data,
                ["V_m"])
            self.assert_pass_tolerance(rel_diff, di_tolerances_iaf)


# --------------------------------------------------------------------------- #
#  Run the comparisons
# ------------------------
#

@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
def suite():
    return unittest.makeSuite(AEIFTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
