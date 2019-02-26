# -*- coding: utf-8 -*-
#
# test_json.py
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
Test if json output work properly
"""

import unittest
import nest


@nest.ll_api.check_stack
class StatusTestCase(unittest.TestCase):
    """Tests of data in JSON format"""

    def test_GetDefaults_JSON(self):
        """JSON data of GetDefaults"""

        # sli_neuron does not work under PyNEST
        models = (m for m in nest.Models() if m != 'sli_neuron')

        for m in models:
            d_json = nest.GetDefaults(m, output='json')
            self.assertIsInstance(d_json, str)

            d = nest.GetDefaults(m)
            d_json = nest.hl_api.to_json(d)
            self.assertIsInstance(d_json, str)

    def test_GetKernelStatus_JSON(self):
        """JSON data of KernelStatus"""

        d = nest.GetKernelStatus()
        d_json = nest.hl_api.to_json(d)
        self.assertIsInstance(d_json, str)

    def test_GetStatus_JSON(self):
        """JSON data of GetStatus"""

        # sli_neuron does not work under PyNEST
        models = (m for m in nest.Models('nodes') if m != 'sli_neuron')

        for m in models:
            nest.ResetKernel()
            n = nest.Create(m)
            d_json = nest.GetStatus(n, output='json')
            self.assertIsInstance(d_json, str)


def suite():
    suite = unittest.makeSuite(StatusTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
