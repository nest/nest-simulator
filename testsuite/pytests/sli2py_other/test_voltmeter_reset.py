# mistral test_voltmeter_reset

import nest
import pytest


def test_voltmeter_reset():
    """
    Test if resetting works on voltmeter.

    The voltmeter records from iaf_psc_alpha to memory, checks if the proper number
    of data points is acquired, deleted on reset, and recorded again on further
    simulation. This test ascertains that also data stored in the derived recorder
    class, not only in RecordingDevice, is reset.
    """

    nest.ResetKernel()

    n = nest.Create("iaf_psc_alpha")
    vm = nest.Create("voltmeter")
    vm.set({"interval": 1.0})

    nest.Connect(vm, n)

    # Simulate and check initial recording
    nest.Simulate(10.5)
    events = vm.events
    assert len(events["times"]) == 10
    assert len(events["V_m"]) == 10

    # Reset voltmeter and check
    vm.set({"n_events": 0})
    events = vm.events
    assert len(events["times"]) == 0
    assert len(events["V_m"]) == 0

    # Simulate more and check recording
    nest.Simulate(5.0)
    events = vm.events
    assert len(events["times"]) == 5
    assert len(events["V_m"]) == 5
