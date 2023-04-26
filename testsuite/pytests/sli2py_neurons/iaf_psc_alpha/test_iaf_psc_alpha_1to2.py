import dataclasses

import numpy as np
import pytest
import nest
import testsimulation, testutil


@dataclasses.dataclass
class IAFPSCAlpha1to2Simulation(testsimulation.Simulation):
    weight: float = 100.0
    delay: float = None
    min_delay: float = None

    def __post_init__(self):
        self.syn_spec = {"weight": self.weight}
        if self.delay is not None:
            self.syn_spec["delay"] = self.delay

    def setup(self):
        n1, n2 = self.neurons = nest.Create("iaf_psc_alpha", 2)
        n1.I_e = 1450.0
        vm = self.voltmeter = nest.Create("voltmeter")
        vm.interval = self.resolution
        vm_spec = {}
        if self.delay is not None:
            vm_spec["delay"] = self.delay
        nest.Connect(vm, n2, syn_spec=vm_spec)
        nest.Connect(n1, n2, syn_spec=self.syn_spec)


@pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
@testutil.use_simulation(IAFPSCAlpha1to2Simulation)
class TestIAFPSCAlpha1to2WithMultiRes:
    @pytest.mark.parametrize("delay", [1.0])
    def test_1to2(self, simulation):
        simulation.setup()

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expect_default)
        assert actual == expected

    def test_default_delay(self, simulation):
        nest.SetDefaults("static_synapse", {"delay": 1.0})
        simulation.setup()

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expect_default)
        assert actual == expected


@testutil.use_simulation(IAFPSCAlpha1to2Simulation)
@pytest.mark.parametrize("delay,resolution", [(2.0, 0.1)])
@pytest.mark.parametrize("min_delay", [0.1, 0.5, 2.0])
def test_mindelay_invariance(simulation):
    assert simulation.min_delay <= simulation.delay
    nest.set(min_delay=simulation.min_delay, max_delay=simulation.delay)
    simulation.setup()
    results = simulation.simulate()
    actual, expected = testutil.get_comparable_timesamples(results, expect_inv)
    assert actual == expected


expect_default = np.array(
    [
        [2.5, -70],  #          <-- Voltage trace of the postsynaptic neuron
        [2.6, -70],  #               (neuron2), at rest until a spike arrives.
        [2.7, -70],  #
        [2.8, -70],  #
        [2.9, -70],  #
        [3.0, -70],  #
        [3.1, -70],  #              spike at t=3.0 ms.
        [3.2, -70],  #
        [3.3, -70],  #
        [3.4, -70],  #
        [3.5, -70],  #          <--  Synaptic delay of 1.0 ms.
        [3.6, -70],  #
        [3.7, -70],  #
        [3.8, -70],  #
        [3.9, -70],  #
        [4.0, -70],  #  <------  Spike arrives at the postsynaptic neuron
        [4.1, -69.9974],  #     <-    (neuron2) and changes the state vector of
        [4.2, -69.9899],  #       |   the neuron, not visible in voltage because
        [4.3, -69.9781],  #       |   voltage of PSP initial condition is 0.
        [4.4, -69.9624],  #       |
        [4.5, -69.9434],  #        -  Arbitrarily close to the time of impact
        [4.6, -69.9213],  #           (t=4.0 ms) the effect of the spike (PSP)
        [4.7, -69.8967],  #           is visible in the voltage trace.
        [4.8, -69.8699],  #
        [4.9, -69.8411],  #
        [5.0, -69.8108],  #
        [5.1, -69.779],  #
        [5.2, -69.7463],  #     <---  The voltage trace is independent
        [5.3, -69.7126],  #           of the computation step size h.
        [5.4, -69.6783],  #           Larger step sizes only have fewer
        [5.5, -69.6435],  #           sample points.
        [5.6, -69.6084],  #
        [5.7, -69.5732],  #
    ]
)

expect_inv = np.array(
    [
        [0.1, -70],  #
        [0.2, -70],  #
        [0.3, -70],  #
        [0.4, -70],  #
        [0.5, -70],  #
        [2.8, -70],  #
        [2.9, -70],  #
        [3.0, -70],  #
        [3.1, -70],  #
        [3.2, -70],  #
        [3.3, -70],  #
        [3.4, -70],  #
        [3.5, -70],  #
        [4.8, -70],  #
        [4.9, -70],  #
        [5.0, -70],  #
        [5.1, -69.9974],  #
        [5.2, -69.9899],  #
        [5.3, -69.9781],  #
        [5.4, -69.9624],  #
        [5.5, -69.9434],  #
        [5.6, -69.9213],  #
        [5.7, -69.8967],  #
        [5.8, -69.8699],  #
        [5.9, -69.8411],  #
        [6.0, -69.8108],  #
    ]
)
