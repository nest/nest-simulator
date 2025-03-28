# -*- coding: utf-8 -*-
#
# test_issue_1974.py
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


from mpi_test_wrapper import MPITestAssertEqual


@MPITestAssertEqual([1, 2, 4])
def test_issue_1974():
    """
    For each of the two parrot neurons, exactly one connection from/to the in/out MUSIC proxy must be created,
    specifically to the proxy on the rank on which the parrot neuron exists. Therefore, the pooled connections across
    ranks must be identical independent of number of ranks.
    """

    import nest

    if not nest.ll_api.sli_func("statusdict/have_music ::"):
        return

    nrns = nest.Create("parrot_neuron", 2)

    music_in = nest.Create("music_event_in_proxy", 1, {"port_name": "in_spikes"})
    music_out = nest.Create("music_event_out_proxy", 1, {"port_name": "out_spikes"})

    nest.Connect(music_in, nrns)
    nest.Connect(nrns, music_out)

    conns = nest.GetConnections().get(output="pandas").drop(labels=["port"], axis=1, errors="ignore")
    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False)  # noqa: F821
