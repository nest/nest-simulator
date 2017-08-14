# -*- coding: utf-8 -*-
#
# cross_check_test_mip_corrdet.py
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

# Script to check correlation_detector.

# Calculates spike cross correlation function of both spike trains in
# spike_detector-0-0-3.gdf. The file is generated after running the
# testscript testsuite/unittests/test_mip_corrdet.sli
#
# Author: Helias
# Date: 08-04-07
#

from scipy import *
from matplotlib.pylab import *  # for plot
import nest
import unittest


@nest.check_stack
class mipCorrdetTestCase(unittest.TestCase):
    """Testing mip_generator and correlation_detector."""

    def test_mip_generator_correlation_detector(self):
        nest.ResetKernel()
        
        # Cross check generated with cross_check_test_mip_corrdet.py
        exptected_hist = [2453., 2528., 2507., 2439., 2459., 2451., 2441., 2523., 2494.,
                          2445., 4909., 2369., 2410., 2457., 2495., 2484., 2369., 2341.,
                          2452., 2475., 2453.]
        
        h = 0.1             # Computation step size in ms 
        T = 100000.0        # Total duration
        delta_tau = 10.0
        tau_max = 100.0
        pc = 0.5
        nu = 100.0
    
        # grng_seed is 0 because test data was produced for seed = 0
        nest.SetKernelStatus({'local_num_threads': 1, 'resolution': h,
                              'overwrite_files': True, 'grng_seed': 0})
    
        # Check, if we can set another rng
    #    nest.SetDefaults('mip_generator', )
    #/mip_generator << /mother_rng rngdict/MT19937 :: 101 CreateRNG >> SetDefaults
    #/mip_generator << /mother_rng rngdict/knuthlfg :: 101 CreateRNG >> SetDefaults % this seed will be ignored, because explicitly set
    
    
        mg = nest.Create('mip_generator')
        nest.SetStatus(mg, {'rate': nu, 'p_copy': pc})
    
        cd = nest.Create('correlation_detector')
        nest.SetStatus(cd, {'tau_max': tau_max, 'delta_tau': delta_tau})
    
        sd = nest.Create('spike_detector')
        nest.SetStatus(sd, {'withtime': True,
                            'withgid': True, 'time_in_steps': True})
    
        pn1 = nest.Create('parrot_neuron')
        pn2 = nest.Create('parrot_neuron')
    
        nest.Connect(mg, pn1)
        nest.Connect(mg, pn2)
        nest.Connect(pn1, sd)
        nest.Connect(pn2, sd)
        
        nest.SetDefaults('static_synapse', {'weight': 1.0, 'receptor_type': 0})
        nest.Connect(pn1, cd)
        
        nest.SetDefaults('static_synapse', {'weight': 1.0, 'receptor_type': 1})
        nest.Connect(pn2, cd)
    
        nest.Simulate(T)
        
        hist = [x for x in nest.GetStatus(cd)[0]['histogram']]
        n_events = nest.GetStatus(cd)[0]['n_events']
        n1 = n_events[0]
        n2 = n_events[1]
        
        lmbd1 = ( n1 / ( T - tau_max ) ) * 1000.0
        lmbd2 = ( n2 / ( T - tau_max ) ) * 1000.0
        
        print(hist)
        print(lmbd1)
        print(lmbd2)
        
        print(nest.GetStatus(sd))

        self.assertEqual(hist, exptected_hist)
        
        
        
        h = 0.1
        tau_max = 100.0  # ms correlation window
        t_bin = 10.0  # ms bin size
        
        spikes = nest.GetStatus(sd)[0]['events']['senders']
        print(spikes)
        print(find(spikes[:] == 4))
        
        sp1 = find(spikes[:] == 4)
        sp2 = find(spikes[:] == 5)
        
        cross = self.corr_spikes_sorted(sp1, sp2, t_bin, tau_max, h)

        print(cross)
        print(sum(cross))
        
    def corr_spikes_sorted(self, spike1, spike2, tbin, tau_max, h):
        tau_max_i = int(tau_max / h)
        tbin_i = int(tbin / h)
    
        cross = zeros(int(2 * tau_max_i / tbin_i + 1), 'd')
    
        j0 = 0
    
        for spki in spike1:
            j = j0
            while j < len(spike2) and spike2[j] - spki < -tau_max_i - tbin_i / 2.0:
                j += 1
            j0 = j
    
            while j < len(spike2) and spike2[j] - spki < tau_max_i + tbin_i / 2.0:
                cross[int(
                    (spike2[j] - spki + tau_max_i + 0.5 * tbin_i) / tbin_i)] += 1.0
                j += 1
    
        return cross
        
def suite():
    suite = unittest.makeSuite(mipCorrdetTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
    
# Auto- and crosscorrelation functions for spike trains.
#
# A time bin of size tbin is centered around the time difference it
# represents If the correlation function is calculated for tau in
# [-tau_max, tau_max], the pair events contributing to the left-most
# bin are those for which tau in [-tau_max-tbin/2, tau_max+tbin/2) and
# so on.

# correlate two spike trains with each other
# assumes spike times to be ordered in time
# tau > 0 means spike2 is later than spike1
#
# tau_max:     maximum time lag in ms correlation function
# tbin:   bin size
# spike1: first spike train [tspike...]
# spike2: second spike train [tspike...]
#

"""
def corr_spikes_sorted(spike1, spike2, tbin, tau_max, h):
    tau_max_i = int(tau_max / h)
    tbin_i = int(tbin / h)

    cross = zeros(int(2 * tau_max_i / tbin_i + 1), 'd')

    j0 = 0

    for spki in spike1:
        j = j0
        while j < len(spike2) and spike2[j] - spki < -tau_max_i - tbin_i / 2.0:
            j += 1
        j0 = j

        while j < len(spike2) and spike2[j] - spki < tau_max_i + tbin_i / 2.0:
            cross[int(
                (spike2[j] - spki + tau_max_i + 0.5 * tbin_i) / tbin_i)] += 1.0
            j += 1

    return cross


def main():
    
    make_spikes()
    
    # resolution
    h = 0.1
    tau_max = 100.0  # ms correlation window
    t_bin = 10.0  # ms bin size

    # read input from spike detector
    spikes = load('spike_detector-3-0.gdf')

    sp1 = spikes[find(spikes[:, 0] == 4), 1]
    sp2 = spikes[find(spikes[:, 0] == 5), 1]

    cross = corr_spikes_sorted(sp1, sp2, t_bin, tau_max, h)

    print(cross)
    print(sum(cross))


main()
"""