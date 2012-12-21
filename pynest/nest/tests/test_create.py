#! /usr/bin/env python
#
# test_create.py
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
Creation tests
"""

import unittest
import nest
import sys

class CreateTestCase(unittest.TestCase):


    def test_ModelCreate(self):
        """Model Creation"""       

        nest.ResetKernel()
        for model in nest.Models(mtype='nodes'):
            node = nest.Create(model)


    def test_ModelCreateN(self):
        """Model Creation with N"""       

        nest.ResetKernel()
        for model in nest.Models(mtype='nodes'):
            node = nest.Create(model,10)


    def test_ModelCreateNdict(self):
        """Model Creation with N and dict"""       

        nest.ResetKernel()
        for model in nest.Models(mtype='nodes'):
            node = nest.Create(model,10, {})


    def test_ModelDict(self):
        """IAF Creation with N and dict"""       

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 10, {'V_m':12.0})
        V_m = [12.0, 12.0, 12.0, 12.0, 12.0, 12.0, 12.0, 12.0, 12.0, 12.0]        
        self.assertEqual(nest.GetStatus(n,'V_m'), V_m)
        self.assertEqual([key['V_m'] for key in nest.GetStatus(n)], V_m)

        
    def test_ModelDicts(self):
        """IAF Creation with N and dicts"""       

        nest.ResetKernel()

        V_m = [0.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.]
        n   = nest.Create('iaf_neuron', 10, [{'V_m':v} for v in V_m])
        self.assertEqual(nest.GetStatus(n,'V_m'), V_m)
        self.assertEqual([key['V_m'] for key in nest.GetStatus(n)], V_m)


    def test_CopyModel(self):
        """CopyModel"""

        nest.ResetKernel()        
        nest.CopyModel('iaf_neuron','new_neuron',{'V_m':10.0})
        vm = nest.GetDefaults('new_neuron')['V_m']
        self.assertEqual(vm,10.0)
        
        n = nest.Create('new_neuron',10)
        vm = nest.GetStatus([n[0]])[0]['V_m']
        self.assertEqual(vm,10.0)
        
        nest.CopyModel('static_synapse', 'new_synapse',{'weight':10.})
        nest.Connect([n[0]],[n[1]],model='new_synapse')
        w = nest.GetDefaults('new_synapse')['weight']
        self.assertEqual(w,10.0)
        
        try:
            nest.CopyModel('iaf_neuron','new_neuron') # shouldn't be possible a second time
            self.fail('an error should have risen!')  # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "NewModelNameExists" in info.__str__():
                self.fail('could not pass error message to NEST!')             
        # another error has been thrown, this is wrong
        except: 
          self.fail('wrong error has been thrown')

        
def suite():

    suite = unittest.makeSuite(CreateTestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
