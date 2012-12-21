#! /usr/bin/env python
#
# test_status.py
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
Test if Set/GetStatus work properly
"""

import unittest
import nest
import sys

class StatusTestCase(unittest.TestCase):
    """Tests of Set/GetStatus"""


    def test_GetKernelStatus(self):
        """GetKernelStatus"""

        

        nest.ResetKernel()
        s = nest.GetKernelStatus()


    def test_SetKernelStatus(self):
        """SetKernelStatus"""

        nest.ResetKernel()
        nest.SetKernelStatus({})
        nest.SetKernelStatus({'print_time':True})

        try:
            nest.SetKernelStatus({'DUMMY':0})
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "DictError" in info.__str__():
                self.fail('wrong error message')                
        # another error is wrong
        except:
            self.fail('wrong error has been thrown')
      

    def test_GetDefaults(self):
        """GetDefaults"""

        models = nest.Models()
        
        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for model in models:
            nest.ResetKernel()
            d = nest.GetDefaults(model)        


    def test_SetDefaults(self):
        """SetDefaults"""

        models = nest.Models()

        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for m in models:        
            if nest.GetDefaults(m).has_key('V_m'):
                nest.ResetKernel()
                v_m = nest.GetDefaults(m)['V_m']
                nest.SetDefaults(m,{'V_m':-1.})
                self.assertEqual(nest.GetDefaults(m)['V_m'], -1.)
                nest.SetDefaults(m,{'V_m':v_m})

                try:
                    nest.SetDefaults(m,{'DUMMY':0})
                except nest.NESTError:
                    info = sys.exc_info()[1]
                    if not "DictError" in info.__str__():
                        self.fail('wrong error message')                
                # any other error is wrongly thrown
                except:
                    self.fail('wrong error has been thrown')


    def test_GetStatus(self):
        """GetStatus"""

        models = nest.Models()

        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for m in models:        
            if nest.GetDefaults(m).has_key('V_m'):
                nest.ResetKernel()
                n  = nest.Create(m)
                d  = nest.GetStatus(n)
                v1 = nest.GetStatus(n)[0]['V_m']
                v2 = nest.GetStatus(n,'V_m')[0]
                self.assertEqual(v1,v2)
                n  = nest.Create(m,10)
                self.assertEqual(len(nest.GetStatus(n,'V_m')), 10)


    def test_SetStatus(self):
        """SetStatus with dict"""

        models = nest.Models()

        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for m in models:        
            if nest.GetDefaults(m).has_key('V_m'):
                nest.ResetKernel()
                n = nest.Create(m)
                nest.SetStatus(n,{'V_m':1.})
                self.assertEqual(nest.GetStatus(n,'V_m')[0], 1.)

        
    def test_SetStatusList(self):
        """SetStatus with list"""

        models = nest.Models()

        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for m in models:        
            if nest.GetDefaults(m).has_key('V_m'):
                nest.ResetKernel()
                n  = nest.Create(m)
                nest.SetStatus(n,[{'V_m':2.}])
                self.assertEqual(nest.GetStatus(n,'V_m')[0], 2.)


    def test_SetStatusParam(self):
        """SetStatus with parameter"""

        models = nest.Models()
        
        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for m in models:        
            if nest.GetDefaults(m).has_key('V_m'):
                nest.ResetKernel()
                n  = nest.Create(m)
                nest.SetStatus(n,'V_m',3.)
                self.assertEqual(nest.GetStatus(n,'V_m')[0], 3.)


    def test_SetStatusVth_E_L(self):
        """SetStatus of reversal and threshold potential """

        models = nest.Models()

        # sli_neuron does not work under PyNEST
        models.remove('sli_neuron')

        for m in models:
            if nest.GetDefaults(m).has_key('V_th') and nest.GetDefaults(m).has_key('E_L') and m != 'a2eif_cond_exp_HW' and m != 'mat2_psc_exp':
                nest.ResetKernel()

                neuron1 = nest.Create(m)
                neuron2 = nest.Create(m)

                # must not depend on the order.
                new_EL = -90.
                new_Vth= -10.
                nest.SetStatus(neuron1,{'E_L': new_EL})
                nest.SetStatus(neuron2,{'V_th': new_Vth})

                nest.SetStatus(neuron1,{'V_th': new_Vth})
                nest.SetStatus(neuron2,{'E_L': new_EL})

                # next three lines for debugging
                vth1, vth2 = nest.GetStatus(neuron1,'V_th'), nest.GetStatus(neuron2,'V_th')
                if vth1 != vth2:
                    print m, vth1, vth2, nest.GetStatus(neuron1,'E_L'), nest.GetStatus(neuron2,'E_L')

                assert(nest.GetStatus(neuron1,'V_th')==nest.GetStatus(neuron2,'V_th'))
    



def suite():

    suite = unittest.makeSuite(StatusTestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
