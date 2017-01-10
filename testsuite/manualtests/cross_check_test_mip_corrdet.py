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
    # resolution
    h = 0.1
    tau_max = 100.0  # ms correlation window
    t_bin = 10.0  # ms bin size

    # read input from spike detector
    spikes = load('spike_detector-0-0-3.gdf')

    sp1 = spikes[find(spikes[:, 0] == 4), 1]
    sp2 = spikes[find(spikes[:, 0] == 5), 1]

    cross = corr_spikes_sorted(sp1, sp2, t_bin, tau_max, h)

    print(cross)
    print(sum(cross))


main()
