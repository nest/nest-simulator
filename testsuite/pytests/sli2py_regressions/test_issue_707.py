"""
Regression test for Issue #707 (GitHub).

This test ensures that the weight of gap junctions can be recorded with the weight_recorder.
"""

import nest
import pytest


@pytest.mark.skipif_without_gsl
def test_gap_junction_weight_recording():
    """
    Test that the weight of gap junctions can be recorded with the weight_recorder.
    """
    nest.ResetKernel()

    neuron_in = nest.Create("hh_psc_alpha_gap")
    neuron_out = nest.Create("hh_psc_alpha_gap")
    wr = nest.Create("weight_recorder")

    nest.SetDefaults("gap_junction", {"weight_recorder": wr})

    nest.Connect(
        neuron_in,
        neuron_out,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "gap_junction", "weight": 2.0},
    )

    nest.Simulate(10.0)

    events = nest.GetStatus(wr, "events")[0]
    weights = events["weights"]

    assert weights[0] == 2.0
