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

from nest.tests import compatibility

from nest.tests import test_errors
from nest.tests import test_stack
from nest.tests import test_create
from nest.tests import test_status
from nest.tests import test_onetooneconnect
from nest.tests import test_convergent_divergent_connect
from nest.tests import test_connect_all_to_all
from nest.tests import test_connect_fixed_indegree
from nest.tests import test_connect_fixed_outdegree
from nest.tests import test_connect_fixed_total_number
from nest.tests import test_connect_one_to_one
from nest.tests import test_connect_pairwise_bernoulli
from nest.tests import test_findconnections
from nest.tests import test_getconnections
from nest.tests import test_dataconnect
from nest.tests import test_events
from nest.tests import test_networks
from nest.tests import test_threads
from nest.tests import test_csa
from nest.tests import test_quantal_stp_synapse
from nest.tests import test_sp
from nest.tests import test_parrot_neuron
from nest.tests import test_stdp_triplet_synapse


def suite():

    suite = unittest.TestSuite()

    suite.addTest(test_errors.suite())
    suite.addTest(test_stack.suite())
    suite.addTest(test_create.suite())
    suite.addTest(test_status.suite())
    suite.addTest(test_onetooneconnect.suite())
    suite.addTest(test_convergent_divergent_connect.suite())
    suite.addTest(test_connect_all_to_all.suite())
    suite.addTest(test_connect_fixed_indegree.suite())
    suite.addTest(test_connect_fixed_outdegree.suite())
    suite.addTest(test_connect_fixed_total_number.suite())
    suite.addTest(test_connect_one_to_one.suite())
    suite.addTest(test_connect_pairwise_bernoulli.suite())
    suite.addTest(test_findconnections.suite())
    suite.addTest(test_getconnections.suite())
    suite.addTest(test_dataconnect.suite())
    suite.addTest(test_events.suite())
    suite.addTest(test_networks.suite())
    suite.addTest(test_threads.suite())
    suite.addTest(test_csa.suite())
    suite.addTest(test_quantal_stp_synapse.suite())
    suite.addTest(test_sp.suite())
    suite.addTest(test_parrot_neuron.suite())
    suite.addTest(test_stdp_triplet_synapse.suite())

    return suite


if __name__ == "__main__":

    debug = nest.get_debug()
    nest.set_debug(True)

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

    nest.set_debug(debug)
