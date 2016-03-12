#!/usr/bin/env python
import nest

proxy = nest.Create('music_cont_out_proxy', 1)
nest.SetStatus(proxy, {'port_name': 'out'})
nest.SetStatus(proxy, {'record_from': ["V_m"], 'interval': 0.1})

neuron_grp = nest.Create('iaf_cond_exp', 2)
nest.SetStatus(proxy, {'index_map': neuron_grp })
nest.SetStatus([neuron_grp[0]], "I_e", 300.)
nest.SetStatus([neuron_grp[1]], "I_e", 600.)



nest.Simulate(6000)
print( nest.GetStatus(neuron_grp) )


