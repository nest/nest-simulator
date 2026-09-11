# -*- coding: utf-8 -*-
#
# test_gap_junction_source_port.py
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
Tests for the generic ``source_port`` secondary-event routing added to NEST.

The tests use ``gap_junction_source_test``, an infrastructure node that exposes
several gap-junction source and target ports: on source port ``p`` it emits a
constant waveform ``source_values[p]`` and, as a target, records the last
delivered ``weight * value`` per receptor in ``received``. This lets us verify
that each target receives the waveform selected by its connection's
``source_port``.

The number of ports a node exposes is taken from the length of ``source_values``.
Because the source-side legality check is delegated to the model prototype on
ranks/threads where the source is represented by a proxy, the prototype must
expose at least as many ports as any connection uses; the tests therefore set a
two-port default and create two-port nodes throughout.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_gsl

MODEL = "gap_junction_source_test"


def build(threads=1, use_wfr=False, resolution=1.0):
    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.SetKernelStatus({"resolution": resolution})
    if use_wfr:
        # wfr_comm_interval may only be set while waveform relaxation is enabled
        nest.SetKernelStatus({"use_wfr": True, "wfr_comm_interval": resolution})
    else:
        nest.SetKernelStatus({"use_wfr": False})
    # Two-port prototype so the proxy-delegated source-port check accepts port 1.
    nest.SetDefaults(MODEL, {"source_values": [0.0, 0.0]})


def node(v0=0.0, v1=0.0, n=1):
    return nest.Create(MODEL, n, params={"source_values": [v0, v1]})


def connect_gap(pre, post, source_port, receptor_type, weight=1.0):
    nest.Connect(
        pre,
        post,
        {"rule": "one_to_one", "make_symmetric": True},
        {
            "synapse_model": "gap_junction",
            "source_port": source_port,
            "receptor_type": receptor_type,
            "weight": weight,
        },
    )


def received(nodes):
    return [np.atleast_1d(n.get("received")) for n in nodes]


def sport_list(conns):
    return list(np.atleast_1d(conns.get("source_port")))


# --------------------------------------------------------------------------
# 1. connection property: defaults, status round-trip, validation
# --------------------------------------------------------------------------


def test_default_source_port_is_zero():
    build()
    assert nest.GetDefaults("gap_junction")["source_port"] == 0


def test_status_round_trip():
    build()
    src = node(1.0, 2.0)
    tgt = node(0.0, 0.0)
    connect_gap(src, tgt, source_port=1, receptor_type=0)
    conns = nest.GetConnections(source=src, synapse_model="gap_junction")
    assert sport_list(conns) == [1]


def connect_invalid(pop, source_port):
    # all_to_all on one population is symmetric, so it uses a single connection
    # pass and surfaces the forward-pass diagnostic directly.
    nest.Connect(
        pop,
        pop,
        {"rule": "all_to_all"},
        {"synapse_model": "gap_junction", "source_port": source_port, "receptor_type": 0},
    )


def test_negative_source_port_rejected():
    build()
    pop = node(1.0, 2.0, n=2)
    with pytest.raises(nest.NESTError, match="source_port cannot be negative"):
        connect_invalid(pop, -1)


def test_noninteger_source_port_rejected():
    build()
    pop = node(1.0, 2.0, n=2)
    with pytest.raises(Exception):
        connect_invalid(pop, 0.5)


def test_out_of_range_source_port_rejected():
    """A source port beyond the node's exposed ports is rejected."""
    build()
    pop = node(1.0, 2.0, n=2)
    with pytest.raises(nest.NESTError, match="Source port 5 does not exist"):
        connect_invalid(pop, 5)


def test_point_neuron_rejects_nonzero_source_port():
    """Existing point neurons accept only source port zero."""
    build()
    a = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.NESTError, match="Source port 1 does not exist"):
        nest.Connect(
            a,
            a,
            {"rule": "all_to_all"},
            {"synapse_model": "gap_junction", "source_port": 1, "receptor_type": 0},
        )


# --------------------------------------------------------------------------
# 2. routing: two waveforms from one node through two source ports
# --------------------------------------------------------------------------


@pytest.mark.parametrize("threads", [1, 4])
@pytest.mark.parametrize("use_wfr", [False, True])
def test_two_source_ports_two_waveforms(threads, use_wfr):
    build(threads=threads, use_wfr=use_wfr)
    src = node(10.0, 20.0)
    t0 = node(0.0, 0.0)
    t1 = node(0.0, 0.0)

    connect_gap(src, t0, source_port=0, receptor_type=0)
    connect_gap(src, t1, source_port=1, receptor_type=0)

    nest.Simulate(5.0)
    r0, r1 = received([t0, t1])
    assert r0[0] == pytest.approx(10.0)
    assert r1[0] == pytest.approx(20.0)


@pytest.mark.parametrize("threads", [1, 4])
def test_multiple_targets_share_one_waveform(threads):
    """Several targets on one (node, synapse model, source port) get the same waveform."""
    build(threads=threads)
    src = node(7.0, 99.0)
    targets = node(0.0, 0.0, n=5)

    for tgt in targets:
        connect_gap(src, tgt, source_port=0, receptor_type=0, weight=2.0)

    nest.Simulate(5.0)
    vals = [r[0] for r in received(targets)]
    assert all(v == pytest.approx(2.0 * 7.0) for v in vals)


@pytest.mark.parametrize("threads", [1, 4])
def test_no_sharing_between_source_ports(threads):
    """Targets on different source ports of one node receive different waveforms."""
    build(threads=threads)
    src = node(3.0, 5.0)
    t0 = node(0.0, 0.0)
    t1 = node(0.0, 0.0)

    connect_gap(src, t0, source_port=0, receptor_type=0)
    connect_gap(src, t1, source_port=1, receptor_type=0)

    nest.Simulate(5.0)
    r0, r1 = received([t0, t1])
    assert r0[0] != r1[0]
    assert r0[0] == pytest.approx(3.0)
    assert r1[0] == pytest.approx(5.0)


# --------------------------------------------------------------------------
# 3. make_symmetric swaps source_port and receptor_type
# --------------------------------------------------------------------------


def test_make_symmetric_swaps_endpoints():
    build()
    a = node(100.0, 200.0)
    b = node(1000.0, 2000.0)

    connect_gap(a, b, source_port=1, receptor_type=0)

    # forward: a[source_port=1] -> b[receptor_type=0]
    fwd = nest.GetConnections(source=a, target=b, synapse_model="gap_junction")
    assert sport_list(fwd) == [1]
    # reverse: b[source_port=0] -> a[receptor_type=1]
    rev = nest.GetConnections(source=b, target=a, synapse_model="gap_junction")
    assert sport_list(rev) == [0]

    nest.Simulate(5.0)
    ra, rb = received([a, b])
    # b receptor 0 receives a's port-1 waveform (200)
    assert rb[0] == pytest.approx(200.0)
    # a receptor 1 receives b's port-0 waveform (1000)
    assert ra[1] == pytest.approx(1000.0)
