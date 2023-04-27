import nest
import pytest
import numpy as np
import numpy.testing as nptest

pytestmark = pytest.mark.skipif_missing_gsl


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()
    nest.local_num_threads = 1
    nest.resolution = 0.1


@pytest.fixture()
def reference_params():
    return {"V_m": 10.0, "g_Na": 19000.0, "g_K": 5000.0, "g_L": 9.0,
            "C_m": 110.0,   "E_Na": 40.0,
            "E_K": -80.0, "E_L": -70.0,
            "E_ex": 1.0, "E_in": -90.0,
            "tau_syn_ex": 0.3, "tau_syn_in": 3.0,
            "t_ref":    3.5, "I_e":   1.0}


@pytest.fixture()
def neuron(reference_params):
    n = nest.Create("hh_cond_exp_traub")
    n.set(**reference_params)
    return n


def recording_devices(neuron):
    dc = nest.Create("dc_generator", {"amplitude": 100.0})

    sg = nest.Create("spike_generator", {"precise_times": False, "spike_times": [0.1, 1.2]})

    sr = nest.Create("spike_recorder", {"time_in_steps": True})

    vm = nest.Create("voltmeter", {"time_in_steps": True, "interval": nest.resolution})

    mm = nest.Create("multimeter", {"time_in_steps": True, "interval": nest.resolution, "record_from": ["g_ex", "g_in"]})

    nest.Connect(sg, neuron)
    nest.Connect(dc, neuron)
    nest.Connect(vm, neuron)
    nest.Connect(mm, neuron)
    nest.Connect(neuron, sr)

    nest.Simulate(5)

    return vm, mm, sr


@pytest.fixture()
def reference_data_vm():
    return [[1, 3.919340e+01], [2, 3.850370e+01], [3, 3.315630e+01], [4, 2.217610e+01],
            [5, 7.131720e+00], [6, -8.728460e+00], [7, -2.312210e+01], [8, -3.595260e+01],
            [9, -5.182590e+01], [10, -6.699940e+01], [11, -7.398330e+01], [12, -7.647150e+01],
            [13, -7.748650e+01], [14, -7.793770e+01], [15, -7.813580e+01], [16, -7.820390e+01],
            [17, -7.819780e+01], [18, -7.814580e+01], [19, -7.806380e+01], [20, -7.796150e+01],
            [21, -7.784520e+01], [22, -7.771910e+01], [23, -7.752600e+01], [24, -7.734640e+01],
            [25, -7.717640e+01], [26, -7.701340e+01], [27, -7.685560e+01], [28, -7.670190e+01],
            [29, -7.655140e+01], [30, -7.640350e+01], [31, -7.625770e+01], [32, -7.611370e+01],
            [33, -7.597140e+01], [34, -7.583060e+01], [35, -7.569110e+01], [36, -7.555300e+01],
            [37, -7.541600e+01], [38, -7.528030e+01], [39, -7.514570e+01], [40, -7.501230e+01]]


@pytest.fixture()
def reference_data_mm():
    return [[1, 0, 0], [2, 0, 0], [3, 0, 0], [4, 0, 0], [5, 0, 0], [6, 0, 0], [7, 0, 0], [8, 0, 0], [9, 0, 0],
            [10, 0, 0], [11, 1, 0], [12, 7.165310e-01, 0], [13, 5.134160e-01, 0], [14, 3.678790e-01, 0],
            [15, 2.635960e-01, 0], [16, 1.888750e-01, 0], [17, 1.353340e-01, 0], [18, 9.697120e-02, 0],
            [19, 6.948280e-02, 0], [20, 4.978650e-02, 0], [21, 3.567350e-02, 0], [22, 1.025560e+00, 0],
            [23, 7.348450e-01, 0], [24, 5.265390e-01, 0], [25, 3.772810e-01, 0], [26, 2.703330e-01, 0],
            [27, 1.937020e-01, 0], [28, 1.387930e-01, 0], [29, 9.944960e-02, 0], [30, 7.125860e-02, 0],
            [31, 5.105890e-02, 0], [32, 3.658530e-02, 0], [33, 2.621440e-02, 0], [34, 1.878340e-02, 0],
            [35, 1.345890e-02, 0], [36, 9.643710e-03, 0], [37, 6.910010e-03, 0], [38, 4.951230e-03, 0],
            [39, 3.547710e-03, 0], [40, 2.542040e-03, 0]]


def test_setting_params(neuron, reference_params):
    neuron_status = neuron.get(reference_params.keys())
    assert neuron_status == reference_params


def test_recoding_device_status(neuron, reference_data_vm, reference_data_mm):
    vm, mm, sr = recording_devices(neuron)
    reference_data_vm = np.array(reference_data_vm)
    vm_events = vm.get("events")
    actual_vm_data = np.array(list(zip(vm_events["times"], vm_events["V_m"])))

    nptest.assert_allclose(actual_vm_data, reference_data_vm, rtol=1e-5)

    reference_data_mm = np.array(reference_data_mm)
    mm_events = mm.get("events")
    actual_mm_data = np.array(list(zip(mm_events["times"], mm_events["g_ex"], mm_events["g_in"])))

    nptest.assert_allclose(actual_mm_data, reference_data_mm, rtol=1e-5)

    sr_events_times = sr.get("events")["times"]
    assert sr_events_times == [2]
