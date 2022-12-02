import nest
from pprint import pprint
import matplotlib.pyplot as plt
import numpy as np

# TODO: This is just a test script, remove from repository before PR

nest.ResetKernel()

weight = 250.0
delay = 0.1

nest.SetDefaults('iaf_tsodyks', {'I_e': 0.})

spikegen = nest.Create('spike_generator', 1)
nest.SetStatus(spikegen, {"spike_times": [1.]})

# iaf_tsodyks with static synapse
#nrn_pre = nest.Create("iaf_psc_exp_ps", 1)
nrn_pre = nest.Create("iaf_tsodyks", 1)
nrn_post = nest.Create("iaf_tsodyks", 1)


# Set huge synaptic weight, control spike releases by refractory period
nest.SetStatus(nrn_pre, {'t_ref': 10000.})

#m = nest.Create('multimeter', 1)
#nest.SetStatus(m, {"record_from": ["V_m", "I_syn_ex"]})

nest.CopyModel("static_synapse", "syn_static", {"weight": weight, "delay": delay})
nest.CopyModel("static_synapse", "spike_forcing_syn", {"weight": 10000000., "delay": delay})

nest.Connect(spikegen, nrn_pre, syn_spec="spike_forcing_syn")
nest.Connect(nrn_pre, nrn_post, syn_spec="syn_static")

#nest.Connect(m, nrn_post)

nest.Simulate(10)
