# -*- coding: utf-8 -*-
#
# test_rate_instantaneous_and_delayed.py
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


@nest.check_stack
class RateInstantaneousAndDelayedTestCase(unittest.TestCase):

    '''
    Test whether delayed rate connections have same properties as
    instantaneous connections but with the correct delay
    '''

    def test_rate_instantaneous_and_delayed(self):

        # neuron parameters
        neuron_params = {'tau': 5., 'std': 0.}
        drive = 1.5
        delay = 2.
        weight = 0.5

        # simulation parameters
        simtime = 100.
        dt = 0.001

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus(
            {'resolution': dt, 'use_wfr': True, 'print_time': False})

        # set up rate neuron network
        rate_neuron_drive = nest.Create(
            'lin_rate_ipn', params={'mean': drive, 'std': 0.})

        rate_neuron_1 = nest.Create(
            'lin_rate_ipn', params=neuron_params)
        rate_neuron_2 = nest.Create(
            'lin_rate_ipn', params=neuron_params)

        multimeter = nest.Create(
            'multimeter', params={
                'record_from': ['rate'],
                'precision': 10,
                'interval': dt})

        # record rates and connect neurons
        neurons = rate_neuron_1 + rate_neuron_2

        nest.Connect(
            multimeter, neurons, 'all_to_all', {'delay': 10.})

        nest.Connect(rate_neuron_drive, rate_neuron_1,
                     'all_to_all', {'model': 'rate_connection_instantaneous',
                                    'weight': weight})

        nest.Connect(rate_neuron_drive, rate_neuron_2,
                     'all_to_all', {'model': 'rate_connection_delayed',
                                    'delay': delay,
                                    'weight': weight})

        # simulate
        nest.Simulate(simtime)

        # make sure shifted rates are identical
        events = nest.GetStatus(multimeter)[0]['events']
        senders = events['senders']

        rate_1 = np.array(events['rate'][np.where(senders == rate_neuron_1)])
        times_2 = np.array(events['times'][np.where(senders == rate_neuron_2)])
        rate_2 = np.array(events['rate'][np.where(senders == rate_neuron_2)])

        # get shifted rate_2
        rate_2 = rate_2[times_2 > delay]
        # adjust length of rate_1 to be able to substract
        rate_1 = rate_1[:len(rate_2)]

        assert(np.sum(np.abs(rate_2 - rate_1)) < 1e-12)


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        RateInstantaneousAndDelayedTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
