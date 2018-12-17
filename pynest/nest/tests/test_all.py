# -*- coding: utf-8 -*-
#
# test_all.py
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

import unittest
import nest

from . import compatibility

from . import test_aeif_lsodar
from . import test_connect_all_patterns
from . import test_connect_all_to_all
from . import test_connect_array_fixed_indegree
from . import test_connect_array_fixed_outdegree
from . import test_connect_distributions
from . import test_connect_fixed_indegree
from . import test_connect_fixed_outdegree
from . import test_connect_fixed_total_number
from . import test_connect_one_to_one
from . import test_connect_pairwise_bernoulli
from . import test_connect_parameters
from . import test_connect_symmetric_pairwise_bernoulli
from . import test_create
from . import test_csa
from . import test_current_recording_generators
from . import test_dataconnect
from . import test_erfc_neuron
from . import test_errors
from . import test_events
from . import test_facetshw_stdp
from . import test_getconnections
from . import test_helper_functions
from . import test_labeled_synapses
from . import test_mc_neuron
from . import test_networks
from . import test_onetooneconnect
from . import test_parrot_neuron_ps
from . import test_parrot_neuron
from . import test_pp_psc_delta
from . import test_pp_psc_delta_stdp
from . import test_quantal_stp_synapse
from . import test_rate_copy_model
from . import test_rate_instantaneous_and_delayed
from . import test_rate_neuron
from . import test_rate_neuron_communication
from . import test_refractory
from . import test_siegert_neuron
from . import test_sp
from . import test_split_simulation
from . import test_stack
from . import test_status
from . import test_stdp_multiplicity
from . import test_stdp_triplet_synapse
from . import test_threads
from . import test_use_gid_in_filename
from . import test_vogels_sprekeler_synapse
from . import test_weight_recorder


def suite():

    suite = unittest.TestSuite()

    suite.addTest(test_aeif_lsodar.suite())
    suite.addTest(test_connect_all_patterns.suite())
    suite.addTest(test_connect_all_to_all.suite())
    suite.addTest(test_connect_array_fixed_indegree.suite())
    suite.addTest(test_connect_array_fixed_outdegree.suite())
    suite.addTest(test_connect_distributions.suite())
    suite.addTest(test_connect_fixed_indegree.suite())
    suite.addTest(test_connect_fixed_outdegree.suite())
    suite.addTest(test_connect_fixed_total_number.suite())
    suite.addTest(test_connect_one_to_one.suite())
    suite.addTest(test_connect_pairwise_bernoulli.suite())
    suite.addTest(test_connect_parameters.suite())
    suite.addTest(test_connect_symmetric_pairwise_bernoulli.suite())
    suite.addTest(test_create.suite())
    suite.addTest(test_csa.suite())
    suite.addTest(test_current_recording_generators.suite())
    suite.addTest(test_dataconnect.suite())
    suite.addTest(test_erfc_neuron.suite())
    suite.addTest(test_errors.suite())
    suite.addTest(test_events.suite())
    suite.addTest(test_facetshw_stdp.suite())
    suite.addTest(test_getconnections.suite())
    suite.addTest(test_helper_functions.suite())
    suite.addTest(test_labeled_synapses.suite())
    suite.addTest(test_mc_neuron.suite())
    suite.addTest(test_networks.suite())
    suite.addTest(test_onetooneconnect.suite())
    suite.addTest(test_parrot_neuron_ps.suite())
    suite.addTest(test_parrot_neuron.suite())
    suite.addTest(test_pp_psc_delta.suite())
    suite.addTest(test_pp_psc_delta_stdp.suite())
    suite.addTest(test_quantal_stp_synapse.suite())
    suite.addTest(test_rate_copy_model.suite())
    suite.addTest(test_rate_instantaneous_and_delayed.suite())
    suite.addTest(test_rate_neuron.suite())
    suite.addTest(test_rate_neuron_communication.suite())
    suite.addTest(test_refractory.suite())
    suite.addTest(test_siegert_neuron.suite())
    suite.addTest(test_sp.suite())
    suite.addTest(test_split_simulation.suite())
    suite.addTest(test_stack.suite())
    suite.addTest(test_status.suite())
    suite.addTest(test_stdp_multiplicity.suite())
    suite.addTest(test_stdp_triplet_synapse.suite())
    suite.addTest(test_threads.suite())
    suite.addTest(test_use_gid_in_filename.suite())
    suite.addTest(test_vogels_sprekeler_synapse.suite())
    suite.addTest(test_weight_recorder.suite())

    return suite


if __name__ == "__main__":

    debug = nest.get_debug()
    nest.set_debug(True)

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

    nest.set_debug(debug)
