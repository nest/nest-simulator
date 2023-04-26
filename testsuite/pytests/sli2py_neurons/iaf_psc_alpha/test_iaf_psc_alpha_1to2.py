import numpy as np
import pytest
import nest


@pytest.fixture
def setup_simulation(
    simulation, resolution, weight, delay, syn_spec, n1_params, n2_params
):
    def setup_simulation_fixture():
        _syn_spec = {}
        if weight is not None:
            _syn_spec["weight"] = weight
        if delay is not None:
            _syn_spec["delay"] = delay
        _syn_spec.update(syn_spec)
        _n1_params = {"I_e": 1450.0}
        _n1_params.update(n1_params)
        _n2_params = {}
        _n2_params.update(n2_params)

        n1, n2 = simulation.neurons = nest.Create("iaf_psc_alpha", 2)
        n1.set(_n1_params)
        n2.set(_n2_params)
        vm = simulation.voltmeter = nest.Create("voltmeter")
        vm.interval = resolution
        if delay is None:
            nest.Connect(vm, n2)
        else:
            nest.Connect(vm, n2, syn_spec={"delay": delay})
        nest.Connect(n1, n2, syn_spec=_syn_spec)

    return setup_simulation_fixture


@pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
class TestIAFPSCAlpha1to2WithMultiRes:
    def test_1to2(self, simulate):
        _, results = simulate()
        self._utils.get_comparable_timesamples(results, expect_default)

    # Set the `delay` parameter to `None` so that it isn't set by the `iaf_1to2` fixture
    @pytest.mark.parametrize("delay", [None])
    def test_default_delay(self, simulate, delay):
        # Instead, set the default delay here.
        nest.SetDefaults("static_synapse", {"delay": 1.0})
        _, results = simulate()
        self._utils.get_comparable_timesamples(results, expect_default)

    @pytest.fixture(autouse=True)
    def use_utils(self, testutils):
        self._utils = testutils


@pytest.mark.parametrize("delay,resolution", [(2.0, 0.1)])
@pytest.mark.parametrize("min_delay", [0.1, 0.5, 2.0])
def test_mindelay_invariance(simulate, min_delay, delay, testutils):
    assert min_delay <= delay
    nest.set(min_delay=min_delay, max_delay=delay)
    _, results = simulate()
    testutils.get_comparable_timesamples(results, expect_inv)


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
