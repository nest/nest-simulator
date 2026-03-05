# -*- coding: utf-8 -*-
#
# test_step_current_generator.py
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
Test basic properties of ``step_current_generator``.

This set of tests checks parameter setting and compares actual simulation
result against expectation for one application.
"""

import nest
import numpy.testing as nptest
import pandas as pd
import pandas.testing as pdtest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_set_params_on_instance_and_model_and():
    """
    Test setting ``step_current_generator`` parameters on instance and model.
    """

    params = {"amplitude_times": [1.0, 2.5, 4.6], "amplitude_values": [100.0, -100.0, 200.0]}

    # Set params on instance
    scg1 = nest.Create("step_current_generator")
    scg1.set(params)

    # Set params on model
    nest.SetDefaults("step_current_generator", params)
    scg2 = nest.Create("step_current_generator")

    # Verify that both ways to set params give same result
    df_scg1 = pd.DataFrame.from_dict(scg1.get(params.keys()))
    df_scg2 = pd.DataFrame.from_dict(scg2.get(params.keys()))
    pdtest.assert_frame_equal(df_scg1, df_scg2)


@pytest.mark.parametrize("params", [{"amplitude_times": [1.0, 2.0]}, {"amplitude_values": [1.0, 2.0]}])
def test_set_amplitude_times_or_values_only_fails(params):
    """
    Ensure failure if setting only one of ``amplitude_times`` or ``amplitude_values``.
    """

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.SetDefaults("step_current_generator", params)


def test_set_different_sized_amplitude_times_and_values_fails():
    """
    Ensure failure if setting different sized ``amplitude_times`` and ``amplitude_values``.
    """

    params = {"amplitude_times": [1.0, 2.0], "amplitude_values": [1.0, 2.0, 3.0]}

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.SetDefaults("step_current_generator", params)


def test_fail_if_amplitude_times_not_strictly_increases():
    """
    Ensure failure if ``amplitude_times`` is not strictly increasing.
    """

    params = {"amplitude_times": [1.0, 2.0, 2.0], "amplitude_values": [1.0, 2.0, 3.0]}

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.SetDefaults("step_current_generator", params)


def test_step_current_generator_simulation(expected_V_m):
    """
    Test
    """

    nrn = nest.Create(
        "iaf_psc_alpha", params={"V_th": 1e10, "C_m": 1.0, "tau_m": 1.0, "E_L": 0.0, "V_reset": 0.0, "V_m": 0.0}
    )
    scg = nest.Create(
        "step_current_generator",
        params={
            "start": 5.0,
            "stop": 15.0,
            "amplitude_times": [1.5, 5.5, 7.5, 10.0, 14.9, 15.0, 20.0],
            "amplitude_values": [1.0, -2.0, 4.0, -8.0, 16.0, -32.0, 64.0],
        },
    )
    vm = nest.Create("voltmeter", params={"interval": 0.1})

    nest.Connect(scg, nrn)
    nest.Connect(vm, nrn)

    nest.Simulate(25.0)

    actual_V_m = vm.events["V_m"]

    nptest.assert_almost_equal(actual_V_m, expected_V_m, decimal=5)


@pytest.fixture(scope="module")
def expected_V_m():
    expected = [
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        9.516260e-02,
        1.812690e-01,
        2.591820e-01,
        3.296800e-01,
        3.934690e-01,
        1.657010e-01,
        -4.039300e-02,
        -2.268740e-01,
        -3.956100e-01,
        -5.482870e-01,
        -6.864360e-01,
        -8.114380e-01,
        -9.245450e-01,
        -1.026890e00,
        -1.119490e00,
        -1.203280e00,
        -1.279100e00,
        -1.347700e00,
        -1.409780e00,
        -1.465940e00,
        -1.516770e00,
        -1.562750e00,
        -1.604360e00,
        -1.642010e00,
        -1.676080e00,
        -1.135930e00,
        -6.471810e-01,
        -2.049430e-01,
        1.952100e-01,
        5.572840e-01,
        8.849020e-01,
        1.181340e00,
        1.449570e00,
        1.692280e00,
        1.911890e00,
        2.110600e00,
        2.290400e00,
        2.453090e00,
        2.600300e00,
        2.733500e00,
        2.854020e00,
        2.963070e00,
        3.061750e00,
        3.151040e00,
        3.231830e00,
        3.304930e00,
        3.371070e00,
        3.430920e00,
        3.485080e00,
        3.534080e00,
        2.436470e00,
        1.443310e00,
        5.446560e-01,
        -2.684760e-01,
        -1.004230e00,
        -1.669960e00,
        -2.272350e00,
        -2.817400e00,
        -3.310590e00,
        -3.756850e00,
        -4.160640e00,
        -4.526000e00,
        -4.856600e00,
        -5.155730e00,
        -5.426400e00,
        -5.671310e00,
        -5.892910e00,
        -6.093430e00,
        -6.274860e00,
        -6.439030e00,
        -6.587580e00,
        -6.721990e00,
        -6.843610e00,
        -6.953650e00,
        -7.053230e00,
        -7.143320e00,
        -7.224850e00,
        -7.298610e00,
        -7.365360e00,
        -7.425750e00,
        -7.480400e00,
        -7.529850e00,
        -7.574590e00,
        -7.615070e00,
        -7.651700e00,
        -7.684850e00,
        -7.714840e00,
        -7.741970e00,
        -7.766530e00,
        -7.788750e00,
        -7.808850e00,
        -7.827040e00,
        -7.843500e00,
        -7.858390e00,
        -7.871870e00,
        -7.884060e00,
        -7.895090e00,
        -7.905080e00,
        -7.914110e00,
        -5.638380e00,
        -5.101820e00,
        -4.616320e00,
        -4.177020e00,
        -3.779520e00,
        -3.419850e00,
        -3.094410e00,
        -2.799940e00,
        -2.533490e00,
        -2.292400e00,
        -2.074240e00,
        -1.876850e00,
        -1.698250e00,
        -1.536640e00,
        -1.390410e00,
        -1.258090e00,
        -1.138370e00,
        -1.030040e00,
        -9.320180e-01,
        -8.433250e-01,
        -7.630720e-01,
        -6.904560e-01,
        -6.247510e-01,
        -5.652980e-01,
        -5.115020e-01,
        -4.628270e-01,
        -4.187830e-01,
        -3.789300e-01,
        -3.428700e-01,
        -3.102420e-01,
        -2.807190e-01,
        -2.540050e-01,
        -2.298330e-01,
        -2.079610e-01,
        -1.881710e-01,
        -1.702640e-01,
        -1.540620e-01,
        -1.394010e-01,
        -1.261350e-01,
        -1.141320e-01,
        -1.032710e-01,
        -9.344310e-02,
        -8.455080e-02,
        -7.650470e-02,
        -6.922430e-02,
        -6.263680e-02,
        -5.667610e-02,
        -5.128260e-02,
        -4.640250e-02,
        -4.198670e-02,
        -3.799110e-02,
        -3.437580e-02,
        -3.110450e-02,
        -2.814450e-02,
        -2.546620e-02,
        -2.304280e-02,
        -2.085000e-02,
        -1.886580e-02,
        -1.707050e-02,
        -1.544600e-02,
        -1.397620e-02,
        -1.264610e-02,
        -1.144270e-02,
        -1.035380e-02,
        -9.368490e-03,
        -8.476960e-03,
        -7.670270e-03,
        -6.940350e-03,
        -6.279890e-03,
        -5.682280e-03,
        -5.141540e-03,
        -4.652260e-03,
        -4.209540e-03,
        -3.808950e-03,
        -3.446480e-03,
        -3.118500e-03,
        -2.821740e-03,
        -2.553210e-03,
        -2.310240e-03,
        -2.090390e-03,
        -1.891470e-03,
    ]
    return expected
