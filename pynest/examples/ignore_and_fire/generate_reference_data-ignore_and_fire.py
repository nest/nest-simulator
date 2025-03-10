# -*- coding: utf-8 -*-
#
# generate_reference_data-ignore_and_fire.py
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
Generate reference data
-----------------------
"""
import sys

import model
import psutil


def generate_reference_data(neuron_model="ignore_and_fire"):
    """
    Generate set of reference data and store on disk (spike data and model paramaters).

    Note: Data can be loaded from file using

             parameters = model.get_default_parameters()
             spikes = model.load_spike_data("./")

    """

    parameters = model.get_default_parameters()
    parameters["neuron_model"] = neuron_model

    parameters["record_spikes"] = True
    # parameters["record_weights"] = True

    model_instance = model.Model(parameters)

    print("\nneuron model: %s" % model_instance.pars["neuron_model"])

    model_instance.create()
    model_instance.connect()

    # connectivity at start of simulation
    subset_size = 2000  # number of pre- and post-synaptic neurons weights are extracted from
    pop_pre = model_instance.nodes["pop_E"][:subset_size]
    pop_post = model_instance.nodes["pop_E"][:subset_size]
    C = model_instance.get_connectivity(
        pop_pre, pop_post, model_instance.pars["data_path"] + "/" + "connectivity_presim.dat"
    )

    # simulate
    model_instance.simulate(model_instance.pars["T"])

    # save parameters to file
    model_instance.save_parameters("model_instance_parameters", model_instance.pars["data_path"])

    # connectivity at end of simulation
    C = model_instance.get_connectivity(
        pop_pre, pop_post, model_instance.pars["data_path"] + "/" + "connectivity_postsim.dat"
    )


generate_reference_data(neuron_model=sys.argv[1])
