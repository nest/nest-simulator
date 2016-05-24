# -*- coding: utf-8 -*-
#
# test_connect_all_patterns.py
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

import numpy as np
import unittest
import nest

from . import test_connect_helpers as hf

from .test_connect_parameters import TestParams
from .test_connect_one_to_one import TestOneToOne
from .test_connect_all_to_all import TestAllToAll
from .test_connect_fixed_indegree import TestFixedInDegree
from .test_connect_fixed_outdegree import TestFixedOutDegree
from .test_connect_fixed_total_number import TestFixedTotalNumber
from .test_connect_pairwise_bernoulli import TestPairwiseBernoulli

nest.set_verbosity("M_WARNING")

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestAllToAll)
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestOneToOne))
    suite.addTest(
        unittest.TestLoader().loadTestsFromTestCase(TestFixedInDegree))
    suite.addTest(
        unittest.TestLoader().loadTestsFromTestCase(TestFixedOutDegree))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(
        TestFixedTotalNumber))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(
        TestPairwiseBernoulli))
    unittest.TextTestRunner(verbosity=2).run(suite)
