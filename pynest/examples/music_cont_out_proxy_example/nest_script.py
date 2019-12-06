#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# nest_script.py
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

"""
Music example
--------------

This example runs 2 NEST instances and one receiver instance. Neurons on
the NEST instances are observed by the music_cont_out_proxy and their
values are forwarded through MUSIC to the receiver.

"""
import nest
import music
import numpy

proxy = nest.Create('music_cont_out_proxy', 1)
nest.SetStatus(proxy, {'port_name': 'out'})
nest.SetStatus(proxy, {'record_from': ["V_m"], 'interval': 0.1})

neuron_grp = nest.Create('iaf_cond_exp', 2)
nest.SetStatus(proxy, {'targets': neuron_grp})
nest.SetStatus(neuron_grp[0], "I_e", 300.)
nest.SetStatus(neuron_grp[1], "I_e", 600.)

nest.Simulate(200)
