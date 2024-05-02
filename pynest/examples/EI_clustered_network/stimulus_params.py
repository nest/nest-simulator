# -*- coding: utf-8 -*-
#
# stimulus_params.py
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

""" PyNEST EI-clustered network: Stimulus Parameters
-----------------------------------------------

A dictionary with parameters for an optinal stimulation of clusters.

"""

stim_dict = {
    # list of clusters to be stimulated (None: no stimulation, 0-n_clusters-1)
    "stim_clusters": [2, 3, 4],
    # stimulus amplitude (in pA)
    "stim_amp": 0.15,
    # stimulus start times in ms: list (warmup time is added automatically)
    "stim_starts": [2000, 6000],
    # list of stimulus end times in ms (warmup time is added automatically)
    "stim_ends": [3500, 7500],
}
