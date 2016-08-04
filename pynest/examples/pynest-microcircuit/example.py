# -*- coding: utf-8 -*-
#
# example.py
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

'''
pynest microcicuit example
----------------------------------------------

Example file to run the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
'''

from network_params import *
from sim_params import *
from stimulus_params import *
import time
import network as network


# initialize the network and pass parameters to it
net = network.Network(sim_dict, net_dict, stim_dict)
# connect all nodes
t1 = time.time()
net.connect()
print 'time to create the connections', time.time()-t1
# simulate
t2 = time.time()
net.simulate()
print 'time to simulate', time.time()-t2
