# -*- coding: utf-8 -*-
#
# test_current_recording_generators.py
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
Test if currents from generators are being recorded properly
"""

import numpy
import unittest
import nest


@nest.ll_api.check_stack
class CurrentRecordingGeneratorTestCase(unittest.TestCase):
    """
    Test if currents from generators are recorded properly. Specifically:
    1) Length of current vector should be equal to length of membrane
       potential vector
    2) Value of current should be equal to zero when device is inactive
    3) Check if value of current recorded as expected during active period
    """

    def setUp(self):
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        # setting up the neuron and the generators
        self.neuron = nest.Create('iaf_psc_alpha', params={'V_reset': -65.0})

        self.t_origin = 5.0
        self.t_start = 2.5
        self.t_stop = 40.0
        self.t_next = 25.0
        self.i_amp = 500.0
        self.i_off = 50.0

        self.ac = nest.Create('ac_generator', 1,
                              params={'amplitude': self.i_amp,
                                      'offset': self.i_off,
                                      'frequency': 50.0, 'phase': 45.0,
                                      'origin': self.t_origin,
                                      'start': self.t_start,
                                      'stop': self.t_stop})
        nest.Connect(self.ac, self.neuron)

        self.dc = nest.Create('dc_generator', 1,
                              params={'amplitude': self.i_amp,
                                      'origin': self.t_origin,
                                      'start': self.t_start,
                                      'stop': self.t_stop})
        nest.Connect(self.dc, self.neuron)

        times = [self.t_start, self.t_next]
        currents = [self.i_amp / 4, self.i_amp / 2]
        params = {'amplitude_times': times, 'amplitude_values': currents,
                  'origin': self.t_origin, 'start': self.t_start,
                  'stop': self.t_stop}
        self.step = nest.Create("step_current_generator", 1, params)
        nest.Connect(self.step, self.neuron)

        self.noise = nest.Create('noise_generator', 1,
                                 params={'mean': self.i_amp, 'std': 0.0,
                                         'dt': 0.1, 'std_mod': 0.0,
                                         'phase': 45.0, 'frequency': 50.0,
                                         'origin': self.t_origin,
                                         'start': self.t_start,
                                         'stop': self.t_stop})
        nest.Connect(self.noise, self.neuron)

    def test_GetRecordables(self):
        """Check get recordables"""

        val = nest.GetDefaults('step_current_generator')['recordables'][0]
        self.assertEqual(val, 'I', 'Incorrect recordables ({}) /'
                         'for step current generator'.format(val))

        val = nest.GetDefaults('ac_generator')['recordables'][0]
        self.assertEqual(val, 'I', 'Incorrect recordables ({}) /'
                         'for ac generator'.format(val))

        val = nest.GetDefaults('dc_generator')['recordables'][0]
        self.assertEqual(val, 'I', 'Incorrect recordables ({}) /'
                         'for dc generator'.format(val))

        val = nest.GetDefaults('noise_generator')['recordables'][0]
        self.assertEqual(val, 'I', 'Incorrect recordables ({}) /'
                         'for noise generator'.format(val))

    def test_RecordedCurrentVectors(self):
        """Check the length and contents of recorded current vectors"""

        # setting up multimeters
        m_Vm = nest.Create('multimeter',
                           params={'record_from': ['V_m'], 'interval': 0.1})
        nest.Connect(m_Vm, self.neuron)

        m_ac = nest.Create('multimeter',
                           params={'record_from': ['I'], 'interval': 0.1})
        nest.Connect(m_ac, self.ac)

        m_dc = nest.Create('multimeter',
                           params={'record_from': ['I'], 'interval': 0.1})
        nest.Connect(m_dc, self.dc)

        m_step = nest.Create('multimeter',
                             params={'record_from': ['I'], 'interval': 0.1})
        nest.Connect(m_step, self.step)

        m_noise = nest.Create('multimeter',
                              params={'record_from': ['I'], 'interval': 0.1})
        nest.Connect(m_noise, self.noise)

        # run simulation
        nest.Simulate(50)

        # retrieve vectors
        events_Vm = nest.GetStatus(m_Vm)[0]['events']
        t_Vm = events_Vm['times']
        v_Vm = events_Vm['V_m']

        events_ac = nest.GetStatus(m_ac)[0]['events']
        t_ac = events_ac['times']
        i_ac = events_ac['I']

        events_dc = nest.GetStatus(m_dc)[0]['events']
        t_dc = events_dc['times']
        i_dc = events_dc['I']

        events_step = nest.GetStatus(m_step)[0]['events']
        t_step = events_step['times']
        i_step = events_step['I']

        events_noise = nest.GetStatus(m_noise)[0]['events']
        t_noise = events_noise['times']
        i_noise = events_noise['I']

        # test the length of current vectors
        assert len(i_ac) == len(v_Vm), \
            "Incorrect current vector length for AC generator"
        assert len(i_dc) == len(v_Vm), \
            "Incorrect current vector length for DC generator"
        assert len(i_step) == len(v_Vm), \
            "Incorrect current vector length for step current generator"
        assert len(i_noise) == len(v_Vm), \
            "Incorrect current vector length for noise generator"

        # test to ensure current = 0 when device is inactive
        # and also to check current recorded when device is active
        t_start_ind = numpy.where(t_ac == self.t_start + self.t_origin)[0][0]
        t_stop_ind = numpy.where(t_ac == self.t_stop + self.t_origin)[0][0]
        assert (numpy.all(i_ac[:t_start_ind]) == 0 and
                numpy.all(i_ac[t_stop_ind:]) == 0), \
            "Current not zero when AC generator inactive"
        self.assertAlmostEqual(numpy.amax(i_ac[t_start_ind:t_stop_ind]),
                               self.i_amp + self.i_off,
                               msg=("Current not correct "
                                    "when AC generator active"))
        self.assertAlmostEqual(numpy.amin(i_ac[t_start_ind:t_stop_ind]),
                               -self.i_amp + self.i_off,
                               msg=("Current not correct "
                                    "when AC generator active"))

        t_start_ind = numpy.where(t_dc == self.t_start + self.t_origin)[0][0]
        t_stop_ind = numpy.where(t_dc == self.t_stop + self.t_origin)[0][0]
        assert (numpy.all(i_dc[:t_start_ind]) == 0 and
                numpy.all(i_dc[t_stop_ind:]) == 0), \
            "Current not zero when DC generator inactive"
        assert (numpy.allclose(i_dc[t_start_ind:t_stop_ind], self.i_amp)), \
            "Current not correct when DC generator active"

        t_start_ind = numpy.where(t_step == self.t_start + self.t_origin)[0][0]
        t_stop_ind = numpy.where(t_step == self.t_stop + self.t_origin)[0][0]
        t_next_ind = numpy.where(t_step == self.t_next)[0][0]
        assert (numpy.all(i_step[:t_start_ind]) == 0 and
                numpy.all(i_step[t_stop_ind:]) == 0), \
            "Current not zero when step current generator inactive"
        assert (numpy.allclose(i_step[t_start_ind:t_next_ind],
                               self.i_amp / 4) and
                numpy.allclose(i_step[t_next_ind:t_stop_ind],
                               self.i_amp / 2)), \
            "Current not correct when step current generator active"

        t_start_ind = numpy.where(
            t_noise == self.t_start + self.t_origin)[0][0]
        t_stop_ind = numpy.where(t_noise == self.t_stop + self.t_origin)[0][0]
        assert (numpy.all(i_noise[:t_start_ind]) == 0 and
                numpy.all(i_noise[t_stop_ind:]) == 0), \
            "Current not zero when noise generator inactive"
        assert (numpy.allclose(i_noise[t_start_ind:t_stop_ind],
                               self.i_amp)), \
            "Current not correct when noise generator active"


def suite():
    return unittest.makeSuite(CurrentRecordingGeneratorTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
