# -*- coding: utf-8 -*-
#
# test_facetshw_stdp.py
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

import nest
import numpy as np
import unittest


class FacetsTestCase(unittest.TestCase):
    """
    This script is testing the accumulation of spike pairs and
    the weight update mechanism as implemented in the FACETS hardware.

    Author: Thomas Pfeil
    Date of first version: 21.01.2013
    """

    def test_facetshw_stdp(self):

        nest.ResetKernel()

        modelName = 'stdp_facetshw_synapse_hom'

        # homogeneous parameters for all synapses
        Wmax = 100.0

        # see *.cpp file of synapse model and Pfeil et al. 2012 for LUT
        # configuration
        lut_0 = [2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 15]
        lut_1 = [0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13]
        lut_2 = range(16)  # identity
        config_0 = [0, 0, 1, 0]
        config_1 = [0, 1, 0, 0]
        reset_pattern = 6 * [1]  # reset all

        # individual parameters for each synapse
        # reached every 36 runs (e^(-10/20) = 21.83510375)
        lut_th_causal = 21.835
        lut_th_acausal = lut_th_causal

        # other parameters
        startWeight = 0  # as digital value [0, 1, ..., 15]
        tau = 20.0

        timeBetweenPairs = 100.0
        # frequency_of_pairs = 10Hz => delta_t(+) = 10ms, delta_t(-) = 90ms
        delay = 5.0
        spikesIn = np.arange(10.0, 60000.0, timeBetweenPairs)

        synapseDict = {'tau_plus': tau,
                       'tau_minus_stdp': tau,
                       'Wmax': Wmax,
                       'synapses_per_driver': 50,
                       'driver_readout_time': 15.0,
                       'lookuptable_0': lut_0,
                       'lookuptable_1': lut_1,
                       'lookuptable_2': lut_2,
                       'configbit_0': config_0,
                       'configbit_1': config_1,
                       'reset_pattern': reset_pattern,
                       'a_thresh_th': lut_th_causal,
                       'a_thresh_tl': lut_th_acausal}

        # build network
        stim = nest.Create('spike_generator')
        neuronA = nest.Create('parrot_neuron')
        neuronB = nest.Create('parrot_neuron')
        nest.SetStatus(stim, [{'spike_times': spikesIn}])

        nest.SetDefaults(modelName, synapseDict)

        # check if GetDefaults returns same values as have been set
        synapseDictGet = nest.GetDefaults(modelName)
        for key in synapseDict.keys():
            self.assertTrue(
                all(np.atleast_1d(synapseDictGet[key] == synapseDict[key])))

        nest.Connect(stim, neuronA)
        nest.Connect(neuronA, neuronB, syn_spec={
            'weight': float(startWeight) / 15.0 * Wmax,
            'delay': delay, 'synapse_model': modelName})

        nest.Simulate(50.0)
        weightTrace = []
        for run in range(len(spikesIn)):
            nest.Simulate(timeBetweenPairs)

            connections = nest.GetConnections(neuronA)
            if (connections.get('synapse_model') == modelName):
                weightTrace.append(
                    [run, connections.get('weight'),
                     connections.get('a_causal'),
                     connections.get('a_acausal')])

        # analysis
        weightTrace = np.array(weightTrace)

        # just before theoretical updates
        weightTraceMod36pre = weightTrace[35::36]

        # just after theoretical updates
        weightTraceMod36 = weightTrace[::36]

        weightIndex = int(startWeight)
        for i in range(len(weightTraceMod36pre)):
            # check weight value before update
            # (after spike pair with index 35, 71, ...)
            self.assertTrue(np.allclose(weightTraceMod36pre[i][1],
                                        1.0 / 15.0 * weightIndex * Wmax,
                                        atol=1e-6))
            weightIndex = lut_0[weightIndex]

        weightIndex = int(startWeight)
        for i in range(len(weightTraceMod36)):
            # check weight value after update
            # (after spike pair with index 0, 36, 72, ...)
            self.assertTrue(np.allclose(weightTraceMod36[i][1],
                                        1.0 / 15.0 * weightIndex * Wmax,
                                        atol=1e-6))
            # check charge on causal capacitor
            self.assertTrue(np.allclose(weightTraceMod36[i][2],
                                        np.ones_like(weightTraceMod36[i][2]) *
                                        np.exp(-2 * delay / tau), atol=1e-6))
            weightIndex = lut_0[weightIndex]

        # check charge on anti-causal capacitor after each pair
        for i in range(len(weightTrace) - 1):
            # TODO: global params
            self.assertTrue(np.allclose(weightTrace[i, 3], ((i % 36) + 1) *
                                        np.exp(-(timeBetweenPairs -
                                                 2 * delay) / tau),
                                        atol=1e-6))


def suite():
    suite = unittest.makeSuite(FacetsTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
