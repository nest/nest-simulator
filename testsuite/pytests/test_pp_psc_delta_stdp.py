# -*- coding: utf-8 -*-
#
# test_pp_psc_delta_stdp.py
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
import numpy as np


class PpPscDeltaSTDPTestCase(unittest.TestCase):
    """
    Regressiontest to reproduce failure of pp_psc_delta to show spike
    timing dependent plasticity (STDP), as opposed to iaf_psc_delta.
    The problem is probably related to the setting of 'archiver_length'.

    Moritz Deger, moritz.deger@epfl.ch, Aug 14, 2015
    """

    def test_pp_psc_delta_stdp(self):
        Dt = 1.
        nsteps = 100
        w_0 = 100.

        nest.ResetKernel()

        nrn_pre = nest.Create('parrot_neuron')
        nrn_post1 = nest.Create('iaf_psc_delta')
        nrn_post2 = nest.Create('pp_psc_delta')

        nest.Connect(nrn_pre, nrn_post1 + nrn_post2,
                     syn_spec={'synapse_model': 'stdp_synapse', 'weight': w_0})

        conn1 = nest.GetConnections(nrn_pre, nrn_post1)
        conn2 = nest.GetConnections(nrn_pre, nrn_post2)
        print(conn1)
        print(conn2)

        sg_pre = nest.Create('spike_generator')
        nest.SetStatus(sg_pre,
                       {'spike_times': np.arange(Dt, nsteps * Dt, 10. * Dt)})
        nest.Connect(sg_pre, nrn_pre)

        w1 = np.zeros(nsteps+1)
        w2 = np.zeros(nsteps+1)
        w1[0] = nest.GetStatus(conn1, keys=['weight'])[0][0]
        w2[0] = nest.GetStatus(conn2, keys=['weight'])[0][0]

        for i in range(nsteps):
            nest.Simulate(Dt)
            w1[i+1] = nest.GetStatus(conn1, keys=['weight'])[0][0]
            w2[i+1] = nest.GetStatus(conn2, keys=['weight'])[0][0]

        self.assertEqual(list(w1), list(w2))

        archiver_length1 = nrn_post1.get('archiver_length')
        archiver_length2 = nrn_post2.get('archiver_length')
        print(archiver_length1)
        print(archiver_length2)
        self.assertEqual(archiver_length1, archiver_length2)


def suite():

    suite = unittest.makeSuite(PpPscDeltaSTDPTestCase, 'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
