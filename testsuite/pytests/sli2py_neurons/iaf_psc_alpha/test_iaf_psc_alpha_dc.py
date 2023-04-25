import numpy as np
import pytest
import nest

@pytest.fixture
def setup_simulation(simulation, weight, delay, syn_spec, n1_params, amplitude, resolution):
    def setup_simulation_fixture():
        _syn_spec = {}
        if weight is not None:
            _syn_spec["weight"] = weight
        if delay is not None:
            _syn_spec["delay"] = delay
        _syn_spec.update(syn_spec)
        _n1_params = {}
        _n1_params.update(n1_params)
        n1 = simulation.neuron = nest.Create("iaf_psc_alpha")
        n1.set(_n1_params)
        dc = simulation.dc_generator = nest.Create("dc_generator")
        dc.amplitude = amplitude
        vm = simulation.voltmeter = nest.Create("voltmeter")
        vm.interval = resolution
        nest.Connect(dc, n1, syn_spec=_syn_spec)
        nest.Connect(vm, n1)

    return setup_simulation_fixture


@pytest.fixture
def modify_dc_gen(simulation, amplitude, origin, arrival, resolution):
    simulation.dc_generator.set(
        amplitude=amplitude,
        origin=origin,
        start=arrival - resolution,
    )


@pytest.mark.parametrize("weight", [1.0])
class TestIAFPSCAlphaDC:
    @pytest.mark.parametrize("delay", [0.1])
    def test_dc(self, simulate, testutils):
        _, results = simulate()
        testutils.assert_expected_table(results, expect_default)

    @pytest.mark.parametrize("resolution,delay", [(.1, .1), (.2, .2), (.5, .5), (1., 1.)])
    @pytest.mark.parametrize("post_setup_simulation", [modify_dc_gen])
    def test_dc_aligned(self, simulate, delay, testutils):
        _, results = simulate()
        testutils.assert_expected_table(results, expect_aligned)

    @pytest.fixture(autouse=True)
    def use_utils(self, testutils):
        self._utils = testutils

expect_default = np.array([
    [0.1, -70],  #   <--------   is at rest (initial condition).
    [0.2, -70],  # <-------   |
    [
        0.3,
        -69.602,
    ],  # <-  |   - In the first update step 0ms  -> 0.1 ms, i.e. at
    [
        0.4,
        -69.2079,
    ],  #  | |     the earliest possible time, the current generator
    [
        0.5,
        -68.8178,
    ],  #  | |     is switched on and emits a current event with time
    [0.6, -68.4316],  #  | |     stamp 0.1 ms.
    [0.7, -68.0492],  #  | |
    [
        0.8,
        -67.6706,
    ],  #  |  ---- After the minimal delay of 1 computation time step,
    [
        0.9,
        -67.2958,
    ],  #  |       the current affects the state of the neuron. This is
    [
        1.0,
        -66.9247,
    ],  #  |       reflected in the neuron's state variable y0 (initial
    [
        1.1,
        -66.5572,
    ],  #  |       condition) but has not yet affected the membrane
    [1.2, -66.1935],  #  |       potential.
    [1.3, -65.8334],  #  |
    [
        1.4,
        -65.4768,
    ],  #   ------ The effect of the DC current, influencing the neuron
    [
        1.5,
        -65.1238,
    ],  #          for 0.1 ms now, becomes visible in the membrane potential.
    [1.6, -64.7743],  #
])
expect_aligned = np.array([
     [ 2.5          , -70          ], #
     [ 2.6          , -70          ], #
     [ 2.7          , -70          ], #
     [ 2.8          , -70          ], #
     [ 2.9          , -70          ], #
     [ 3.0          , -70          ], #  %  <---- Current starts to affect
     [ 3.1          , -69.602      ], #   %   neuron (visible in state variable
     [ 3.2          , -69.2079     ], #  %   y0). This is the desired onset of
     [ 3.3          , -68.8178     ], #  %    t= 3.0 ms.
     [ 3.4          , -68.4316     ], #
     [ 3.5          , -68.0492     ], #
     [ 3.6          , -67.6706     ], #
     [ 3.7          , -67.2958     ], #
     [ 3.8          , -66.9247     ], #
     [ 3.9          , -66.5572     ], #
     [ 4.0          , -66.1935     ], #
     [ 4.1          , -65.8334     ], #
     [ 4.2          , -65.4768     ], #
 ])