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


class StatusTestCase(unittest.TestCase):
    """Tests of data in JSON format"""

    def test_GetDefaults_JSON(self):
        """JSON data of GetDefaults"""

        for model in nest.node_models + nest.synapse_models:
            d_json = nest.GetDefaults(model, output="json")
            self.assertIsInstance(d_json, str)

            d = nest.GetDefaults(model)
            d_json = nest.to_json(d)
            self.assertIsInstance(d_json, str)

    def test_kernel_status_JSON(self):
        """JSON data of KernelStatus"""

        d = nest.kernel_status
        d_json = nest.to_json(d)
        self.assertIsInstance(d_json, str)

    def test_get_JSON(self):
        """JSON data of node status"""

        for model in nest.node_models:
            nest.ResetKernel()
            n = nest.Create(model)
            d_json = n.get(output="json")
            self.assertIsInstance(d_json, str)


def suite():
    suite = unittest.makeSuite(StatusTestCase, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
