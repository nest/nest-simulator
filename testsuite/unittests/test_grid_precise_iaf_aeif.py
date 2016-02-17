#!/usr/bin/env python
#-*- coding:utf-8 -*-
#
# test_grid_precise.py
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

import numpy as np
import nest


"""
Comparing the new implementations of gridprecise and precise-spiking
models in NEST with previous ones and with their regular counterparts.
These hybrid models implement:
  - a linear interpolation to find the spike time more precisely (all),
  - precise-spiking implementation for the "*_ps" models.

Rationale:
  - the new "iaf_psc_alpha_ps" should give the exact same results as the 
    elder "iaf_psc_alpha_canon",
  - the implementation of the precise-spiking and gridprecise models should be
    very close to the RK5 implementation of "aeif_cond_alpha_RK5" as long as
    only DC currents are involved.
  - "aeif_cond_alpha_ps" and "aeif_cond_alpha_gridprecise" can even be compared
    to "aeif_cond_alpha_RK5" when non-precise-spikes are involved.

Details:
  The models are compared and we assess that the difference is smaller than a
  given tolerance.
  In addition, for the aeif models, we make sure that the difference is lower
  between "aeif_cond_alpha_RK5" and the new models than between the old
  "aeif_cond_alpha" and "aeif_cond_alpha_RK5".
"""


#-----------------------------------------------------------------------------#
# Tolerances
#-------------------------
#

# almost identical for iaf
tol_iaf = 1e-4

# higher for aeif since the resolution methods are quite different; note
# however that this is a rather good result since we are working very close
# to the chaotic regime of the aeif model.
tol_rk5 = 0.01

# still higher for the aeif_cond_alpha since we test them with poisson noise
# near their chaotic regime
tol_aeif_cond_alpha = 0.05


#-----------------------------------------------------------------------------#
# Individual dynamics
#-------------------------
#

param_aeif = {
    'V_reset': -48.,
    'V_peak': 0.0,
    'V_th': -50.,
    'I_e': 0.0,
    'g_L': 12.,
    'tau_w': 130.,
    'E_L': -60.,
    'Delta_T': 2.,
    'a': -11.,
    'b': 30.,
    'C_m': 100.,
    'V_m': -60.,
    'tau_syn_in': 1.,
    'tau_syn_ex': 0.2
}

di_iaf = {
    "iaf_psc_alpha_canon":
        {
          'recordables': ["V_m", "I_syn"],
          'param': { 'V_reset': -65., 'V_th': -50., 'I_e': 0.0, 'E_L': -60., 
                     'V_m': -60., 'C_m':96., 'tau_syn': 0.2, 'tau_m': 8. }
        },
    "iaf_psc_alpha_ps":
        {
          'recordables': ["V_m", "I_ex"],
          'param': { 'V_reset': -65., 'V_th': -50., 'I_e': 0.0, 'E_L': -60.,
                     'V_m': -60., 'tau_syn_in': 0.2, 'tau_syn_ex': 0.2,
                     'g_L': 12., "C_m": 96. }
        }
}

di_aeif_rk5 = {
    "aeif_cond_alpha_RK5": { 'recordables': ["V_m"], 'param': param_aeif },
    "aeif_cond_alpha_ps": { 'recordables': ["V_m"], 'param': param_aeif },
    "aeif_cond_exp_ps": { 'recordables': ["V_m"], 'param': param_aeif },
    "aeif_psc_alpha_ps": { 'recordables': ["V_m"], 'param': param_aeif },
    "aeif_psc_exp_ps": { 'recordables': ["V_m"], 'param': param_aeif },
    "aeif_cond_alpha_gridprecise": {'recordables': ["V_m"],'param':param_aeif},
    "aeif_cond_exp_gridprecise": { 'recordables':["V_m"], 'param':param_aeif },
    "aeif_psc_alpha_gridprecise": {'recordables': ["V_m"],'param':param_aeif},
    "aeif_psc_exp_gridprecise": { 'recordables': ["V_m"], 'param': param_aeif }
}

di_aeif_rk5_comp = {
    "aeif_cond_alpha_RK5": { 'recordables': ["V_m"], 'param': param_aeif },
    "aeif_cond_alpha": { 'recordables': ["V_m"], 'param': param_aeif }
}

di_aeif_cond_alpha = {
    "aeif_cond_alpha_RK5":
        { 'recordables': ["V_m", "g_ex"], 'param': param_aeif },
    "aeif_cond_alpha_ps":
        { 'recordables': ["V_m", "g_ex"], 'param': param_aeif },
    "aeif_cond_alpha_gridprecise":
        {'recordables': ["V_m", "g_ex"], 'param': param_aeif }
}

di_aeif_cond_alpha_comp = {
    "aeif_cond_alpha_RK5": {'recordables':["V_m"], 'param':param_aeif},
    "aeif_cond_alpha": { 'recordables': ["V_m"], 'param': param_aeif }
}


#-----------------------------------------------------------------------------#
# Comparison function
#-------------------------
#

def simulate_models(models, I_dc=160., noise=False, precise=False, rate=100.):
    '''
    Simulate models in given conditions and record their variables.
    '''
    msd = 123456
    resol = 0.01
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution":resol})
    N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
    pyrngs = [np.random.RandomState(s) for s in range(msd, msd+N_vp)]
    nest.SetKernelStatus({'grng_seed': msd+N_vp})
    nest.SetKernelStatus({'rng_seeds': range(msd+N_vp+1, msd+2*N_vp+1)})
    num_models = len(models)
    # create the neurons and devices
    print(models)
    lst_neurons = [ nest.Create(model,params=di_model["param"])
                    for model,di_model in models.iteritems() ]
    step_gen = nest.Create("step_current_generator",
                           params= {"amplitude_times": [50.,1500.],
                                    "amplitude_values":[I_dc,0.] })
    pg_model = "poisson_generator_ps" if precise else "poisson_generator"
    pn_model = "parrot_neuron_ps" if precise else "parrot_neuron"
    pg = nest.Create(pg_model, params={"rate": 100.})
    pn = nest.Create(pn_model)
    nest.Connect(pg,pn)
    multimeter = nest.Create("multimeter",num_models)
    # connect them
    for i,di_model in enumerate(models.values()):
        nest.Connect(step_gen,lst_neurons[i])
        nest.SetStatus((multimeter[i],), {"withtime":True, "interval":resol,
                       "record_from": di_model['recordables']})
        nest.Connect((multimeter[i],),lst_neurons[i])
        if noise:
          nest.Connect(pn,lst_neurons[i])
    nest.Simulate(1600.)
    return multimeter
  
def compute_difference(multimeter, models, avg=1.):
    '''
    Compute the relative differences between the values recorded by the
    multimeter.
    '''
    da_time = None
    first = models.keys()[0]
    recordables = models[first]['recordables']
    di_val = { model: [] for model in models.iterkeys() }

    for i,(model,di_model) in enumerate(models.items()):
        dmm = nest.GetStatus(multimeter)[i]
        for j,record in enumerate(recordables):
          di_val[model].append(dmm["events"][di_model['recordables'][j]])
        da_time = dmm["events"]["times"]
    di_diff = { rec:np.zeros(len(da_time)) for rec in recordables }
    di_rel_diff = { rec:0. for rec in recordables }
    
    for i,record in enumerate(recordables):
        first_val = np.array(di_val[first][i])
        for model in models.iterkeys():
            if model != first:
                di_diff[record] += np.abs(first_val - di_val[model][i])
        rel_diff = np.sum(np.abs(di_diff[record]))/(avg*np.sum(np.abs(first_val)))
        di_rel_diff[record] = 0. if np.isnan(rel_diff) else rel_diff
    return di_rel_diff

def compare_iaf_psc_alpha(di_models):
    '''
    Compare "iaf_psc_alpha_ps" and "iaf_psc_alpha_canon".
    These two models should deliver the exact same results within a given
    tolerance as "iaf_psc_alpha_ps" uses a solver and not the exact formula.
    '''
    multimeter = simulate_models(di_models, noise=True, precise=True)
    di_rel_diff = compute_difference(multimeter, di_models)
    for rel_diff in di_rel_diff.itervalues():
        assert(rel_diff < tol_iaf)
    return di_rel_diff

def compare_aeif_RK5(models, compare):
    '''
    Compare the new aeif models with the old "aeif_cond_alpha_RK5"
    implementation; they should stay whithin a reasonnable precision.
    '''
    multimeter = simulate_models(models)
    di_rel_diff = compute_difference(multimeter, models, avg=len(models)-1)
    mm_comp = simulate_models(compare)
    di_rel_diff_comp = compute_difference(mm_comp, compare)
    for rd, rd_comp in zip(di_rel_diff.values(), di_rel_diff_comp.values()):
        assert(rd < tol_rk5)
        assert(rd < rd_comp)
    return di_rel_diff

def compare_aeif_cond_alpha(models, compare):
    '''
    Compare more precisely (with poisson noise) the new aeif_cond_alpha models 
    with the old "aeif_cond_alpha_RK5" implementation; they should stay whithin
    a closer range than "aeif_cond_alpha".
    '''
    multimeter = simulate_models(models, noise=True)
    di_rel_diff = compute_difference(multimeter, models, avg=len(models)-1)
    mm_comp = simulate_models(compare)
    di_rel_diff_comp = compute_difference(mm_comp, compare)
    for rd, rd_comp in zip(di_rel_diff.values(), di_rel_diff_comp.values()):
        assert(rd < tol_aeif_cond_alpha)
        assert(rd < rd_comp)
    return di_rel_diff


#-----------------------------------------------------------------------------#
# Run the comparisons
#-------------------------
#

lst_results = []
# for the precise iaf with precise poisson noise
lst_results.append(compare_iaf_psc_alpha(di_iaf))
# for all aeif without noise
lst_results.append(compare_aeif_RK5(di_aeif_rk5, di_aeif_rk5_comp))
# for the aeif_cond_alpha with non-precise poisson noise
lst_results.append(compare_aeif_cond_alpha( di_aeif_cond_alpha,
                                            di_aeif_cond_alpha_comp) )
