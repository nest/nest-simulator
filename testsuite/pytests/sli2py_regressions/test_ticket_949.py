# -*- coding: utf-8 -*-
#
# test_ticket_949.py
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


"""TThis test ensures that Connect throws and error if one tries to create a connection with a delay
less than the resolution."""
import unittest

import nest


class TestTicket949(unittest.TestCase):
    def test_delay_less_than_resolution_throws(self):
        nest.ResetKernel()
        nest.set_verbosity("M_ERROR")
        nest.resolution = 0.25

        population = nest.Create("iaf_psc_alpha")
        with self.assertRaises(nest.kernel.NESTError, msg='Delay must be greater than or equal to resolution'):
            nest.Connect(population, population, syn_spec={"delay": 0.1})
