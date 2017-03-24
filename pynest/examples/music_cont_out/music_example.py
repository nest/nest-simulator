#!/usr/bin/env python
# -*- coding: utf-8 -*-
import nest


# encoding: utf-8

import numpy


import nest

g = nest.Create('sinusoidal_poisson_generator', n=1, params=[{'rate': 1000.0,
                                                              'amplitude': 5000.0,
                                                              'frequency': 100.0,
                                                              'phase': 0.0}])

f = nest.Create('iaf_cond_exp', 20)
# gen = nest.Create('poisson_generator')
# nest.SetStatus(gen, {'rate': 10000.})
nest.Connect(g, f, 'all_to_all', {'weight': 500.})

multi = nest.Create('multimeter')
nest.SetStatus(multi, {'to_music': True,
                       'to_memory': False,
                       'port_name': 'cont_out',
                       'record_from': ['V_m', 'g_ex'],
                       'interval': 1.
                       })

for i, neuron in enumerate(f):
    nest.Connect(multi, [neuron], 'one_to_one', {'model': 'music_synapse', 'music_channel': i})


# We create an event port just to close the MUSIC loop
# event_in = nest.Create('music_event_in_proxy', 20)
# nest.SetAcceptableLatency('event_in', 20.)
# nest.SetStatus(event_in, {'port_name': 'event_in'})
# for i, proxy in enumerate(event_in):
#     nest.SetStatus([proxy], 'music_channel', i)

nest.Simulate(2000.0)

