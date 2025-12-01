# -*- coding: utf-8 -*-
#
# test_ticket_386.py
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

import nest
import pytest

RESOLUTION = 0.1


def _models_supporting_recordables():
    models = []
    for model in nest.node_models:
        defaults = nest.GetDefaults(model)
        recordables = defaults.get("recordables")
        if recordables:
            models.append((model, tuple(recordables), "compartments" in defaults))
    return models


@pytest.mark.parametrize("model,recordables,has_compartments", _models_supporting_recordables())
def test_ticket_386(model, recordables, has_compartments):
    """
    Ensure multimeter recording at the kernel resolution works for models with recordables.
    """

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": RESOLUTION})

    neuron = nest.Create(model)

    if has_compartments:
        nest.SetStatus(neuron, {"compartments": {"parent_idx": -1}})

    multimeter = nest.Create(
        "multimeter",
        params={
            "record_from": list(recordables),
            "interval": RESOLUTION,
        },
    )

    nest.Connect(multimeter, neuron)

    nest.Simulate(10.0)
