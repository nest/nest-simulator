#! /usr/bin/env python

#author: Thomas Pfeil
#date of 1st version: 21.01.2013

#This script is testing the accumulation of spike pairs and
#the weight update mechanism as implemented in the FACETS hardware

import nest
import numpy as np

modelName      = 'stdp_facetshw_synapse_hom'

##############
# parameters #
# homogen parameters for all synapses #
Wmax           = 100.0
#see *.cpp file of synapse model and Pfeil et al. 2012 for LUT configuration
lut_0          = [2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 15]
lut_1          = [0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13]
lut_2          = range(16) #identity
config_0       = [0, 0, 1, 0]
config_1       = [0, 1, 0, 0]
reset_pattern  = 6 * [1] #reset all

# individual parameters for each synapse #
lut_th_causal  = 21.835 #reached every 36 runs (e^(-10/20)=21.83510375)
lut_th_acausal = lut_th_causal

# other parameters #
startWeight      = 0 #as digital value [0,1,...,15]
tau              = 20.0

timeBetweenPairs = 100.0
delay            = 5.0 #frequency_of_pairs=10Hz => delta_t(+)=10ms, delta_t(-)=90ms
spikesIn         = np.arange(10.0, 60000.0, timeBetweenPairs)

synapseDict    = {'tau_plus'            : tau,
                  'tau_minus_stdp'      : tau,
                  'Wmax'                : Wmax,
                  'synapses_per_driver' : 50,
                  'driver_readout_time' : 15.0,
                  'lookuptable_0'       : lut_0,
                  'lookuptable_1'       : lut_1,
                  'lookuptable_2'       : lut_2,
                  'configbit_0'         : config_0,
                  'configbit_1'         : config_1,
                  'reset_pattern'       : reset_pattern,

                  'a_thresh_th'         : lut_th_causal,
                  'a_thresh_tl'         : lut_th_acausal}

#################
# build network #
stim     = nest.Create('spike_generator')
neuronA  = nest.Create('parrot_neuron')
neuronB  = nest.Create('parrot_neuron')
#recorder = nest.Create('spike_detector')
nest.SetStatus(stim, [{'spike_times': spikesIn}])

nest.SetDefaults(modelName, synapseDict)

#check if Get returns same values as have been Set
synapseDictGet = nest.GetDefaults(modelName)
for key in synapseDict.keys():
  assert(all(np.atleast_1d(synapseDictGet[key] == synapseDict[key])))
  
nest.Connect(stim, neuronA)
nest.Connect(neuronA, neuronB, float(startWeight) / 15.0 * Wmax, delay, model=modelName)
#nest.Connect(neuronA, recorder)
#nest.Connect(neuronB, recorder)

nest.Simulate(50.0)
weightTrace = []
for run in range(len(spikesIn)):
  nest.Simulate(timeBetweenPairs)

  connections = nest.FindConnections(neuronA)
  for i in range(len(connections)):
    if nest.GetStatus([connections[i]])[0]['synapse_model'] == modelName:
      weightTrace.append([run, nest.GetStatus([connections[i]])[0]['weight'],
                               nest.GetStatus([connections[i]])[0]['a_causal'],
                               nest.GetStatus([connections[i]])[0]['a_acausal']])

############
# analysis #
weightTrace = np.array(weightTrace)

weightTraceMod36pre = weightTrace[35::36] #just before theoretical updates
weightTraceMod36 = weightTrace[::36]      #just after theoretical updates

weightIndex = int(startWeight)
for i in range(len(weightTraceMod36pre)):
  #check weight value before update (after spike pair with index 35 , 71                                                          , 107         , ...)
  assert(np.allclose(weightTraceMod36pre[i][1], 1.0 / 15.0 * weightIndex * Wmax, atol=1e-6))
  weightIndex = lut_0[weightIndex]

weightIndex = int(startWeight)
for i in range(len(weightTraceMod36)):
  #check weight value after update (after spike pair with index 0, 36, 72, ...)
  assert(np.allclose(weightTraceMod36[i][1], 1.0 / 15.0 * weightIndex * Wmax, atol=1e-6))
  #check charge on causal capacitor
  assert(np.allclose(weightTraceMod36[i][2], np.ones_like(weightTraceMod36[i][2]) * np.exp(-2 * delay / tau), atol=1e-6))
  weightIndex = lut_0[weightIndex]

#check charge on anti-causal capacitor after each pair
for i in range(len(weightTrace) - 1):
  assert(np.allclose(weightTrace[i, 3], ((i % 36) + 1) * np.exp(-(timeBetweenPairs - 2 * delay) / tau), atol=1e-6)) #TODO: global params

print 'test successful'

#ids = nest.GetStatus(recorder)[0]['events']['senders']
#times = nest.GetStatus(recorder)[0]['events']['times']
#spikes = np.vstack((ids, times)).T
#print spikes[spikes[:,0] == neuronB] - spikes[spikes[:,0] == neuronA]
