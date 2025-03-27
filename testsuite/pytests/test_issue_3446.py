# -*- coding: utf-8 -*-
#
# test_issue_3446.py
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


def test_issue_3446():
    nest.local_num_threads = 3

    nrns = nest.Create("parrot_neuron", 2)

    music_in = nest.Create("music_event_in_proxy", 1, {"port_name": "in_spikes"})
    music_out = nest.Create("music_event_out_proxy", 1, {"port_name": "out_spikes"})

    nest.Connect(music_in, nrns)
    nest.Connect(nrns, music_out)

    nest.GetConnections().get()
