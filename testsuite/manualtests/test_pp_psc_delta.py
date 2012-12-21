#! /usr/bin/env python
#
# test_pp_psc_delta.py
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
#
# tests for pp_psc_delta
#

import nest


# 1) check for reasonable firing rate
# 2) check if fixed dead-time is respected

def check_rate_and_fixed_dead_time():

    # test parameters
    d = 25.0
    lam = 10.0
    T = 10000.0

    nest.ResetKernel()
    nrn = nest.Create('pp_psc_delta')

    params = {  'tau_m': 10.0,
                'C_m':   250.0,
                'dead_time': d, 
                'dead_time_random':  False,
                'dead_time_shape':  1,
                'with_reset':    False ,
                'tau_sfa': 34.0,
                'q_sfa': 0.0,  #// mV, reasonable default is 7 mV
                'c_1':  0.0,
                'c_2':   lam, 
                'c_3':   0.25,
                'I_e':    0.0,
                't_ref_remaining':   0.0
                }

    nest.SetStatus(nrn, params)
    
    sd = nest.Create('spike_detector')
    nest.Connect(nrn, sd)
    nest.Simulate(T)
    
    spikes = nest.GetStatus(sd)[0]['events']['times']
    rate_sim = len(spikes) / (T*1e-3) 
    rate_ana = 1./(1./lam + d*1e-3)
    ratio = rate_sim / rate_ana
    
    # This could fail due to bad luck. However, if it passes once, then it should
    # always do so, since the random numbers are reproducible in NEST.
    assert( 0.5<ratio<1.5 )
    
    isi = []
    for i in xrange(1,len(spikes)):
        isi.append(spikes[i]-spikes[i-1])
        
    assert( min(isi)>=d )



# 3) check if random dead-time moments are respected

def check_random_dead_time():

    # test parameters
    d = 50.0
    n = 10
    lam = 1.0e6
    T = 10000.0

    nest.ResetKernel()
    nrn = nest.Create('pp_psc_delta')

    params = {  'tau_m': 10.0,
                'C_m':   250.0,
                'dead_time': d, 
                'dead_time_random':  True,
                'dead_time_shape':  10,
                'with_reset':    False ,
                'tau_sfa': 34.0,
                'q_sfa': 0.0,  #// mV, reasonable default is 7 mV
                'c_1':  0.0,
                'c_2':   lam, 
                'c_3':   0.25,
                'I_e':    0.0,
                't_ref_remaining':   0.0
                }

    nest.SetStatus(nrn, params)
    
    sd = nest.Create('spike_detector')
    nest.Connect(nrn, sd)
    nest.Simulate(T)
    
    spikes = nest.GetStatus(sd)[0]['events']['times']
    rate_sim = len(spikes) / (T*1e-3) 
    rate_ana = 1./(1./lam + d*1e-3)
    ratio = rate_sim / rate_ana
    
    # This could fail due to bad luck. However, if it passes once, then it should
    # always do so, since the random numbers are reproducible in NEST.
    assert( 0.5<ratio<1.5 )
    
    isi = []
    for i in xrange(1,len(spikes)):
        isi.append(spikes[i]-spikes[i-1])
    
    # compute moments of ISI to get mean and variance
    isi_m1 = 0. 
    isi_m2 = 0.
    for t in isi:
        isi_m1 += t
        isi_m2 += t**2
    
    isi_mean = isi_m1 / len(isi)
    isi_var = isi_m2/ len(isi) - isi_mean**2
    ratio_mean = isi_mean / d
    assert( 0.5<=ratio_mean<=1.5 )
    
    isi_var_th = n / (n/d)**2
    ratio_var = isi_var / isi_var_th
    assert( 0.5<=ratio_var<=1.5 )



# 4) check if threshold adaptation works by looking for negative serial correlation of ISI

def check_adapting_threshold():

    # test parameters
    d = 1e-8
    lam = 30.0
    T = 10000.0

    nest.ResetKernel()
    nrn = nest.Create('pp_psc_delta')

    params = {  'tau_m': 10.0,
                'C_m':   250.0,
                'dead_time': d, 
                'dead_time_random':  False,
                'dead_time_shape':  1,
                'with_reset':    False ,
                'tau_sfa': 34.0,
                'q_sfa': 7.0,  #// mV, reasonable default is 7 mV
                'c_1':  0.0,
                'c_2':   lam, 
                'c_3':   0.25,
                'I_e':    0.0,
                't_ref_remaining':   0.0
                }

    nest.SetStatus(nrn, params)
    
    sd = nest.Create('spike_detector')
    nest.Connect(nrn, sd)
    nest.Simulate(T)
    
    spikes = nest.GetStatus(sd)[0]['events']['times']
    rate_sim = len(spikes) / (T*1e-3) 
    rate_ana = 1./(1./lam + d*1e-3)
    ratio = rate_sim / rate_ana
    
    # This could fail due to bad luck. However, if it passes once, then it should
    # always do so, since the random numbers are reproducible in NEST.
    # Adaptive threshold changes rate, thus not asserted here
    # assert( 0.5<ratio<1.5 )
    
    isi = []
    for i in xrange(1,len(spikes)):
        isi.append(spikes[i]-spikes[i-1])
    
    # compute moments of ISI to get mean and variance
    isi_m1 = isi[-1]
    isi_m2 = isi[-1]**2
    isi_12 = 0.
    for t,t1 in zip(isi[:-1],isi[1:]):
        isi_m1 += t
        isi_m2 += t**2
        isi_12 += t*t1
    
    isi_mean = isi_m1 / len(isi)
    isi_var = (isi_m2 - isi_m1)**2 / len(isi)
    isi_corr = (isi_12 / (len(isi)-1) - isi_mean**2) / isi_var
    
    assert( -1.0<isi_corr<0.0 )
    



if __name__ == "__main__":
    check_rate_and_fixed_dead_time()
    check_random_dead_time()
    check_adapting_threshold()

