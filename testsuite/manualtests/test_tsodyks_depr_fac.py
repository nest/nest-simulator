# -*- coding: utf-8 -*-
#
# test_tsodyks_depr_fac.py
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

from scipy import *
from matplotlib.pylab import *
from matplotlib.mlab import *


def plot_spikes():
    dt = 0.1  # time resolution
    nbins = 1000
    N = 500  # number of neurons

    vm = load('voltmeter-0-0-4.dat')

    figure(1)
    clf()
    plot(vm[:, 0], vm[:, 1], 'r')
    xlabel('time / ms')
    ylabel('$V_m [mV]$')

    savefig('test_tsodyks_depressing.png')


plot_spikes()
show()
