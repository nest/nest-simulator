# -*- coding: utf-8 -*-
#
# test_step_rate_generator.py
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
import unittest
import numpy as np


@nest.ll_api.check_stack
class StepRateGeneratorTestCase(unittest.TestCase):

    '''
    Test whether the step_rate_generator produces and
    communicates the desired rates
    '''

    def test_step_rate_generator(self):

        rates = np.array([400.0, 1000.0, 200.0])

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus(
            {'resolution': 0.1, 'use_wfr': False, 'print_time': False})

        # create nodes
        neuron = nest.Create("lin_rate_ipn", 1, {"sigma": 0.0})
        srg = nest.Create("step_rate_generator", 1)
        mm = nest.Create('multimeter', params={
                         'record_from': ['rate'], 'interval': 100.0})

        # configure srg
        nest.SetStatus(srg, {"amplitude_times": [
                       10.0, 110.0, 210.0], "amplitude_values": rates})

        # connect srg to neuron
        nest.Connect(srg, neuron, "one_to_one",
                     {"synapse_model": "rate_connection_delayed",
                      "weight": 1.0})
        nest.Connect(mm, neuron)
        nest.Connect(mm, srg)

        # simulate
        nest.Simulate(301.)

        # read data from multimeter
        data = nest.GetStatus(mm)[0]['events']
        rates_neuron = np.array(
            data['rate'][np.where(data['senders'] == neuron.get('global_id'))])
        rates_srg = np.array(data['rate'][
            np.where(data['senders'] == srg.get('global_id'))])
        times = np.array(data['times'][
            np.where(data['senders'] == neuron.get('global_id'))])

        # make sure that srg produces the desired rates
        assert(np.array_equal(rates, rates_srg))
        # make sure that communication to the connected neuron works
        assert(np.allclose(rates, rates_neuron, atol=0.2))


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        StepRateGeneratorTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
