import dataclasses
import math

import numpy as np
import pytest
import nest
import testsimulation, testutil
from scipy.special import lambertw


@dataclasses.dataclass
class IAFPSCAlphaSimulation(testsimulation.Simulation):
    def setup(self):
        self.neuron = nest.Create("iaf_psc_alpha")
        vm = self.voltmeter = nest.Create("voltmeter")
        vm.interval = self.resolution
        sr = self.spike_recorder = nest.Create("spike_recorder")
        nest.Connect(vm, self.neuron, syn_spec={"weight": 1.0, "delay": self.delay})
        nest.Connect(self.neuron, sr, syn_spec={"weight": 1.0, "delay": self.delay})

    @property
    def spikes(self):
        return np.column_stack(
            (
                self.spike_recorder.events["senders"],
                self.spike_recorder.events["times"],
            )
        )


@dataclasses.dataclass
class MinDelaySimulation(IAFPSCAlphaSimulation):
    amplitude: float = 1000.0
    min_delay: float = 0.0

    def setup(self):
        dc = self.dc_generator = nest.Create("dc_generator")
        dc.amplitude = self.amplitude

        super().setup()

        nest.Connect(
            dc,
            self.neuron,
            syn_spec={"weight": 1.0, "delay": self.delay},
        )


@testutil.use_simulation(IAFPSCAlphaSimulation)
class TestIAFPSCAlpha:
    def test_iaf_psc_alpha(self, simulation):
        dc = simulation.dc_generator = nest.Create("dc_generator")
        dc.amplitude = 1000

        simulation.setup()

        nest.Connect(
            dc,
            simulation.neuron,
            syn_spec={"weight": 1.0, "delay": simulation.resolution},
        )

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(
            results, expected_default
        )
        assert actual == expected

    @pytest.mark.parametrize("duration", [20.0])
    def test_iaf_psc_alpha_fudge(self, simulation):
        simulation.setup()

        tau_m = 20
        tau_syn = 0.5
        C_m = 250.0
        a = tau_m / tau_syn
        b = 1.0 / tau_syn - 1.0 / tau_m
        t_max = 1.0 / b * (-lambertw(-math.exp(-1.0 / a) / a, k=-1) - 1.0 / a).real
        V_max = (
            math.exp(1)
            / (tau_syn * C_m * b)
            * (
                (math.exp(-t_max / tau_m) - math.exp(-t_max / tau_syn)) / b
                - t_max * math.exp(-t_max / tau_syn)
            )
        )
        simulation.neuron.set(
            tau_m=tau_m, tau_syn_ex=tau_syn, tau_syn_in=tau_syn, C_m=C_m
        )
        sg = nest.Create(
            "spike_generator",
            params={"precise_times": False, "spike_times": [simulation.resolution]},
        )
        nest.Connect(
            sg,
            simulation.neuron,
            syn_spec={"weight": float(1.0 / V_max), "delay": simulation.resolution},
        )

        results = simulation.simulate()

        actual_t_max = results[np.argmax(results[:, 1]), 0]
        assert actual_t_max == pytest.approx(t_max + 0.2, abs=0.05)

    def test_iaf_psc_alpha_i0(self, simulation):
        simulation.setup()

        simulation.neuron.I_e = 1000

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_i0)
        assert actual == expected
        assert simulation.spikes == pytest.approx(expected_i0_t)

    @pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
    def test_iaf_psc_alpha_i0_refractory(self, simulation):
        simulation.setup()

        simulation.neuron.I_e = 1450

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(
            results, expected_i0_refr
        )
        assert actual == expected


@pytest.mark.parametrize("min_delay", [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.0, 2.0])
@pytest.mark.parametrize("delay, duration", [(2.0, 10.5)])
@testutil.use_simulation(MinDelaySimulation)
class TestMinDelayUsingIAFPSCAlpha:
    def test_iaf_psc_alpha_mindelay_create(self, simulation, min_delay):
        simulation.setup()

        # Connect 2 throwaway neurons with `min_delay` to force `min_delay`
        nest.Connect(
            *nest.Create("iaf_psc_alpha", 2),
            syn_spec={"delay": min_delay, "weight": 1.0}
        )

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(
            results, expected_mindelay
        )
        assert actual == expected

    def test_iaf_psc_alpha_mindelay_set(self, simulation, min_delay, delay):
        nest.set(min_delay=min_delay, max_delay=delay)
        nest.SetDefaults("static_synapse", {"delay": delay})

        simulation.setup()

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(
            results, expected_mindelay
        )
        assert actual == expected

    def test_iaf_psc_alpha_mindelay_simblocks(self, simulation, min_delay, delay):
        nest.set(min_delay=min_delay, max_delay=delay)
        nest.SetDefaults("static_synapse", {"delay": delay})

        simulation.setup()

        for _ in range(22):
            nest.Simulate(0.5)
        # duration=0 so that `simulation.simulate` is noop but
        # still extracts results for us.
        simulation.duration = 0
        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(
            results, expected_mindelay
        )
        assert actual == expected


expected_default = np.array(
    [
        [0.1, -70],  # <----- The earliest time dc_gen can be switched on.
        [0.2, -70],  # <----- The DC current arrives at the neuron, it is
        [0.3, -69.602],  # <-     reflected in the neuron's state variable y0,
        [0.4, -69.2079],  #   |    (initial condition) but has not yet affected
        [0.5, -68.8178],  #   |    the membrane potential.
        [0.6, -68.4316],  #   |
        [0.7, -68.0492],  #    --- the effect of the DC current is visible in the
        [0.8, -67.6706],  #        membrane potential
        [0.9, -67.2958],  #
        [1.0, -66.9247],  #
        [4.5, -56.0204],  #
        [4.6, -55.7615],  #
        [4.7, -55.5051],  #
        [4.8, -55.2513],  #
        [4.9, -55.0001],  #
        [5.0, -70],  #  <---- The membrane potential crossed threshold in the
        [5.1, -70],  #        step 4.9 ms -> 5.0 ms. The membrane potential is
        [5.2, -70],  #        reset (no super-threshold values can be observed).
        [5.3, -70],  #        The spike is reported at 5.0 ms
        [5.4, -70],  #
        [5.5, -70],  #
        [5.6, -70],  #
        [5.7, -70],  #
        [5.8, -70],  #
        [5.9, -70],  #
        [6.0, -70],  #
        [6.1, -70],  #
        [6.2, -70],  #
        [6.3, -70],  #
        [6.4, -70],  #
        [6.5, -70],  #
        [6.6, -70],  #
        [6.7, -70],  #
        [6.8, -70],  #
        [6.9, -70],  #
        [7.0, -70],  #  <---- The last point in time at which the membrane potential
        [7.1, -69.602],  #  <-    is clamped. The fact that the neuron is not refractory
        [7.2, -69.2079],  #    |   anymore is reflected in the state variable r==0.
        [7.3, -68.8178],  #    |   The neuron was refractory for 2.0 ms.
        [7.4, -68.4316],  #    |
        [7.5, -68.0492],  #    --- The membrane potential starts to increase
        [7.6, -67.6706],  #        immediately afterwards and the neuron can generate
        [7.7, -67.2958],  #        spikes again (at this resolution reported with time
        [7.8, -66.9247],  #        stamp 7.1 ms on the grid)
        [7.9, -66.5572],  #  <--
        #     |
        #     |
        #     - The simulation was run for 8.0 ms.However, in the step
        #       7.9 ms -> 8.0 ms the voltmeter necessarily receives the
        #       voltages that occurred at time 7.9 ms (delay h).This
        #       results in different end times of the recorded voltage
        #       traces at different resolutions.In the current
        #       simulation kernel there is no general cure for this
        #       problem.One workaround is to end the simulation script
        #       with "h Simulate", thereby making the script resolution
        #       dependent.
    ]
)


sli = np.array(
    [
        -7.000000e01,
        -7.000000e01,
        -6.998033e01,
        -6.993104e01,
        -6.986370e01,
        -6.978665e01,
        -6.970581e01,
        -6.962526e01,
        -6.954774e01,
        -6.947497e01,
        -6.940795e01,
        -6.934717e01,
        -6.929274e01,
        -6.924454e01,
        -6.920227e01,
        -6.916553e01,
        -6.913389e01,
        -6.910686e01,
        -6.908400e01,
        -6.906485e01,
        -6.904898e01,
        -6.903600e01,
        -6.902556e01,
        -6.901732e01,
        -6.901100e01,
        -6.900634e01,
        -6.900310e01,
        -6.900109e01,
        -6.900014e01,
        -6.900008e01,
        -6.900078e01,
        -6.900214e01,
        -6.900404e01,
        -6.900641e01,
        -6.900917e01,
        -6.901226e01,
        -6.901563e01,
        -6.901922e01,
        -6.902300e01,
        -6.902695e01,
        -6.903102e01,
        -6.903519e01,
        -6.903945e01,
        -6.904378e01,
        -6.904816e01,
        -6.905258e01,
        -6.905703e01,
        -6.906151e01,
        -6.906600e01,
        -6.907050e01,
        -6.907500e01,
        -6.907950e01,
        -6.908400e01,
        -6.908849e01,
        -6.909297e01,
        -6.909744e01,
        -6.910190e01,
        -6.910634e01,
        -6.911076e01,
        -6.911517e01,
        -6.911956e01,
        -6.912394e01,
        -6.912829e01,
        -6.913263e01,
        -6.913694e01,
        -6.914124e01,
        -6.914551e01,
        -6.914977e01,
        -6.915401e01,
        -6.915822e01,
        -6.916242e01,
        -6.916659e01,
        -6.917074e01,
        -6.917488e01,
        -6.917899e01,
        -6.918309e01,
        -6.918716e01,
        -6.919121e01,
        -6.919525e01,
        -6.919926e01,
        -6.920325e01,
        -6.920722e01,
        -6.921118e01,
        -6.921511e01,
        -6.921903e01,
        -6.922292e01,
        -6.922680e01,
        -6.923065e01,
        -6.923449e01,
        -6.923831e01,
        -6.924211e01,
        -6.924589e01,
        -6.924965e01,
        -6.925339e01,
        -6.925711e01,
        -6.926082e01,
        -6.926451e01,
        -6.926817e01,
        -6.927182e01,
        -6.927546e01,
        -6.927907e01,
        -6.928267e01,
        -6.928624e01,
        -6.928980e01,
        -6.929335e01,
        -6.929687e01,
        -6.930038e01,
        -6.930387e01,
        -6.930734e01,
        -6.931079e01,
        -6.931423e01,
        -6.931765e01,
        -6.932105e01,
        -6.932444e01,
        -6.932781e01,
        -6.933116e01,
        -6.933450e01,
        -6.933782e01,
        -6.934112e01,
        -6.934441e01,
        -6.934768e01,
        -6.935093e01,
        -6.935417e01,
        -6.935739e01,
        -6.936059e01,
        -6.936378e01,
        -6.936695e01,
        -6.937011e01,
        -6.937325e01,
        -6.937638e01,
        -6.937949e01,
        -6.938258e01,
        -6.938566e01,
        -6.938873e01,
        -6.939178e01,
        -6.939481e01,
        -6.939783e01,
        -6.940083e01,
        -6.940382e01,
        -6.940679e01,
        -6.940975e01,
        -6.941270e01,
        -6.941563e01,
        -6.941854e01,
        -6.942144e01,
        -6.942433e01,
        -6.942720e01,
        -6.943005e01,
        -6.943290e01,
        -6.943572e01,
        -6.943854e01,
        -6.944134e01,
        -6.944413e01,
        -6.944690e01,
        -6.944966e01,
        -6.945240e01,
        -6.945513e01,
        -6.945785e01,
        -6.946055e01,
        -6.946324e01,
        -6.946592e01,
        -6.946859e01,
        -6.947124e01,
        -6.947387e01,
        -6.947650e01,
        -6.947911e01,
        -6.948171e01,
        -6.948429e01,
        -6.948686e01,
        -6.948942e01,
        -6.949197e01,
        -6.949450e01,
        -6.949702e01,
        -6.949953e01,
        -6.950203e01,
        -6.950451e01,
        -6.950698e01,
        -6.950944e01,
        -6.951189e01,
        -6.951432e01,
        -6.951675e01,
        -6.951916e01,
        -6.952155e01,
        -6.952394e01,
        -6.952632e01,
        -6.952868e01,
        -6.953103e01,
        -6.953337e01,
        -6.953569e01,
        -6.953801e01,
        -6.954031e01,
        -6.954261e01,
        -6.954489e01,
        -6.954716e01,
        -6.954942e01,
        -6.955166e01,
        -6.955390e01,
        -6.955613e01,
        -6.955834e01,
    ]
)

expected_i0 = np.array(
    [
        [0.1, -69.602],  #  % <-      because the current has not yet had the
        [0.2, -69.2079],  #  %   |     chance to influence the membrane potential.
        [0.3, -68.8178],  #  %   |     However, the current is reflected in state
        [0.4, -68.4316],  #  %   |     variable I0 in this initial condition of the
        [0.5, -68.0492],  #  %   |     system.
        [4.3, -56.0204],  #  %         affected the membrane potential.
        [4.4, -55.7615],  #  %
        [4.5, -55.5051],  #  %
        [4.6, -55.2513],  #  %
        [4.7, -55.0001],  #  %
        [4.8, -70],  #  %
        [4.9, -70],  #  %
        [5.0, -70],  #  %
    ]
)

expected_i0_t = np.array(
    [
        [1, 4.8],  # %       <-- the neuron emits a spike with time
    ]
)

expected_i0_refr = np.array(
    [
        [0.1, -69.4229],  #  %              because the current has not yet had the
        [0.2, -68.8515],  #  %              chance to influence the membrane
        [0.3, -68.2858],  #  %              potential (initial conditions).
        [0.4, -67.7258],  #
        [0.5, -67.1713],  #
        [0.6, -66.6223],  #
        [0.7, -66.0788],  #
        [0.8, -65.5407],  #
        [0.9, -65.008],  #
        [1.0, -64.4806],  #
        [1.1, -63.9584],  #
        [1.2, -63.4414],  #
        [1.3, -62.9295],  #
        [1.4, -62.4228],  #
        [1.5, -61.9211],  #
        [1.6, -61.4243],  #
        [1.7, -60.9326],  #
        [1.8, -60.4457],  #
        [1.9, -59.9636],  #
        [2.0, -59.4864],  #
        [2.1, -59.0139],  #
        [2.2, -58.5461],  #
        [2.3, -58.0829],  #
        [2.4, -57.6244],  #
        [2.5, -57.1704],  #
        [2.6, -56.721],  #
        [2.7, -56.276],  #
        [2.8, -55.8355],  #
        [2.9, -55.3993],  #
        [3.0, -70],  #  %       <- The membrane potential crossed threshold in the
        [3.1, -70],  #  %  |       reset (no super-threshold values can be observed).
        [3.2, -70],  #  %  |
        [3.3, -70],  #  %   ------ The spike is reported at 3.0 ms
        [3.4, -70],  #
        [3.5, -70],  #
        [3.6, -70],  #
        [3.7, -70],  #
        [3.8, -70],  #
        [3.9, -70],  #
        [4.0, -70],  #
        [4.1, -70],  #
        [4.2, -70],  #
        [4.3, -70],  #
        [4.4, -70],  #
        [4.5, -70],  #
        [4.6, -70],  #
        [4.7, -70],  #
        [4.8, -70],  #
        [4.9, -70],  #
        [5.0, -70],  #  %   <- The last point in time at which the membrane potential
        [
            5.1,
            -69.4229,
        ],  #  % <-   is clamped. The fact that the neuron is not refractory
        [5.2, -68.8515],  #  %   |  anymore is reflected in the state variable r==0.
        [5.3, -68.2858],  #  %   |  The neuron was refractory for 2.0 ms.
        [5.4, -67.7258],  #  %   |
        [5.5, -67.1713],  #  %    -- The membrane potential starts to increase
        [5.6, -66.6223],  #  %       immediately afterwards (step 5ms -> 5ms + h),
        [5.7, -66.0788],  #  %       and the neuron can generate spikes again
        [5.8, -65.5407],  #  %       (with time stamp 5ms + h).
        [5.9, -65.008],  #  %       The membrane trace is independent of the resolution.
        [6.0, -64.4806],  #
        [6.1, -63.9584],  #
        [6.2, -63.4414],  #
        [6.3, -62.9295],  #
        [6.4, -62.4228],  #
        [6.5, -61.9211],  #
        [6.6, -61.4243],  #
        [6.7, -60.9326],  #
        [6.8, -60.4457],  #
        [6.9, -59.9636],  #
    ]
)

expected_mindelay = np.array(
    [
        [1.000000e00, -7.000000e01],
        [2.000000e00, -7.000000e01],
        [3.000000e00, -6.655725e01],
        [4.000000e00, -6.307837e01],
        [5.000000e00, -5.993054e01],
        [6.000000e00, -5.708227e01],
        [7.000000e00, -7.000000e01],
        [8.000000e00, -7.000000e01],
        [9.000000e00, -6.960199e01],
        [1.000000e01, -6.583337e01],
    ]
)
