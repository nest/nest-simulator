# -*- coding: utf-8 -*-
#
# test_kinetics_multi_cmpt_neu.py
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

# This script tests the iaf_cond_alpha_mc_kinetics_neuron in NEST.


import numpy as np
import pylab as pl
import matplotlib.cm as cm
import math
import nest
from scipy import *
from scipy.integrate import ode
import unittest

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")

@unittest.skipIf(not HAVE_GSL, 'GSL is not available')

class mc_neuron_ode():


	def fun_u(self,t,y):
		
		f = [1./self.C_m[0,0]*(-self.G_l[0,0]*(y[0]-self.E_l[0,0]) - y[12]*(y[0]-self.E_ex[0,0]) - y[14]*(y[0]-self.E_in[0,0]) - self.G_conn[0]*((y[0] - self.E_l[0,0])-(y[1] - self.E_l[1,0])) + self.I_curr[0,0]),\
                     1./self.C_m[1,0]*(-self.G_l[1,0]*(y[1]-self.E_l[1,0]) - y[8]*(y[1]-self.E_ex[1,0]) - y[10]*(y[1]-self.E_in[1,0]) - self.G_conn[0]*((y[1] - self.E_l[1,0])-(y[0] - self.E_l[0,0])) - self.G_conn[1]*((y[1] - self.E_l[1,0])-(y[2] - self.E_l[2,0])) + self.I_curr[1,0]),\
                     1./self.C_m[2,0]*(-self.G_l[2,0]*(y[2]-self.E_l[2,0]) - y[4]*(y[2]-self.E_ex[2,0]) - y[6]*(y[2]-self.E_in[2,0]) - self.G_conn[1]*((y[2] - self.E_l[2,0])-(y[1] - self.E_l[1,0])) + self.I_curr[2,0] + y[15]*y[16]*self.gca*(self.eca-y[2])),\
		     -y[3]/self.Tau_synE[2,0],\
		     y[3] - y[4]/self.Tau_synE[2,0],\
		     -y[5]/self.Tau_synI[2,0],\
		     y[5] - y[6]/self.Tau_synI[2,0],\
		     -y[7]/self.Tau_synE[1,0],\
		     y[7] - y[8]/self.Tau_synE[1,0],\
		     -y[9]/self.Tau_synI[1,0],\
		     y[9] - y[10]/self.Tau_synI[1,0],\
		     -y[11]/self.Tau_synE[0,0],\
		     y[11] - y[12]/self.Tau_synE[0,0],\
		     -y[13]/self.Tau_synI[0,0],\
		     y[13] - y[14]/self.Tau_synI[0,0],\
		     (1.0/(1.0+np.exp((y[2]-self.halfm)*-1.0*self.slopem)) - y[15]) / self.taum,\
		     (1.0/(1.0+np.exp((y[2]-self.halfh)*-1.0*self.slopeh)) - y[16]) / self.tauh]

		
		return f
	

		

	def __init__(self,active,resolution):
		
		self.num_cmpt = 3
		self.cmpt_cnt = 0
		self.active_flag = active
		self.resolution = resolution
		self.V_th = -45.0         		   #mV
		self.dist_V_th = -20.0
		self.V_T = self.V_th
		self.dist_VT = self.dist_V_th
		self.V_reset = -60.0                    #mV
	    	self.T_ref = 2.0                        #ms
		self.G_conn = [25.,25.]		   #nS


		self.tau_VT = 7.
		self.VT_jump = 25.
		self.reset_time = int(self.T_ref/self.resolution)
		self.tau_distVT = 20.
		self.dist_VT_jump = 20.
		self.counter = 0
		self.ref = 0
	    	self.E_l = np.array([[-70.0],[-65.0],[-60.0]])          #mV
		self.Conn_mat = np.array([[self.G_conn[0],-self.G_conn[0],0.],[-self.G_conn[0],self.G_conn[0]+self.G_conn[1]+self.E_l[1],-self.G_conn[1]],[0.,-self.G_conn[1],self.G_conn[1]+self.E_l[2]]])
		self.G_l = np.array([[5.0],[5.0],[5.0]])                #nS
	    	self.C_m = np.array([[43.0],[18.0],[13.0]])           #pF

	    	self.E_ex = np.array([[0.0],[0.0],[0.0]])               #mV
	    	self.E_in = np.array([[-85.0],[-85.0],[-85.0]])         #mV

	    	self.Tau_synE = np.array([[1.0],[1.0],[1.0]])           #ms
	    	self.Tau_synI = np.array([[2.0],[2.0],[2.0]])           #ms
		self.u = np.array([-70.0,-65.0,-60.0,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.]) 
		self.G_exc = np.array([[0.0],[0.0],[0.0]])
		self.G_inh = np.array([[0.0],[0.0],[0.0]])
		self.G_ca = np.array([[0.0],[0.0],[0.0]])
		self.G_exc1 = np.array([[0.0],[0.0],[0.0]])
		self.G_inh1 = np.array([[0.0],[0.0],[0.0]])
		self.G_ca1 = np.array([[0.0],[0.0],[0.0]])
		self.I_conn = 0.0
		self.I_curr = np.array([[0.0],[0.0],[0.0]])

		self.y_exc = np.array([[[0.],[0.]],[[0.],[0.]],[[0.],[0.]]])
		self.y_inh = np.array([[[0.],[0.]],[[0.],[0.]],[[0.],[0.]]])
		self.y_ca  = np.array([[[0.],[0.]],[[0.],[0.]],[[0.],[0.]]])

		# distal_spike parameters

		self.eca    = 50.0 	
		self.gca    = 70.0    
		self.taum   = 15.0    
		self.tauh   = 80.0    
		self.halfm  = -9.0    
		self.halfh  = -21.0   
		self.slopem = 0.5     
		self.slopeh = -0.5  

		self.u[15] = 1.0/(1.0+np.exp((self.u[2]-self.halfm)*-1.0*self.slopem))
		self.u[16] = 1.0/(1.0+np.exp((self.u[2]-self.halfh)*-1.0*self.slopeh))


		self.r = ode(self.fun_u).set_integrator('dop853',rtol=1e-3, nsteps=500)
		self.r.set_initial_value(self.u,0.)
	
	def compute_u(self,G_exc,G_inh,I_curr):



		self.counter = 0
		self.distal_cnt = 0

		self.r.y[11] += (G_exc[0,0]*np.exp(1)/self.Tau_synE[0,0])
		self.r.y[13] += (G_inh[0,0]*np.exp(1)/self.Tau_synI[0,0])
		self.r.y[7] += (G_exc[1,0]*np.exp(1)/self.Tau_synE[1,0])
		self.r.y[9] += (G_inh[1,0]*np.exp(1)/self.Tau_synI[1,0])
		self.r.y[3] += (G_exc[2,0]*np.exp(1)/self.Tau_synE[2,0])
		self.r.y[5] += (G_inh[2,0]*np.exp(1)/self.Tau_synI[2,0])

		self.r.integrate(self.r.t+self.resolution)
		self.u = self.r.y


class KineticsMultiCmptNeuTestCase(unittest.TestCase):		

    def test_KineticsMultiCmptNeu(self):

	trial_num = 1

	active_flag = 1
	resolution = 0.1
	sim_time = 1500
	l = int(sim_time/resolution)
	t2 = np.linspace(0.1,1500.,15000)
	rate = 0.0008
	mother_rate = 0.005
	neur = mc_neuron_ode(active_flag,resolution)

	soma_flag = 0
	dist_flag = 0
	input_frac = 1.
	copy_frac = 0.2

	dist_inh_ratio = 1.0
	amp_exc = 200.
	amp_inh = 100.
	tot_exc = 3
	tot_inh = 3
	E_L = -70.
	I_s = np.zeros((3,int(sim_time/resolution)))
	exc_wt = np.ones((tot_exc,1))
	inh_wt = np.ones((tot_inh,1))
	u_arr = np.ones((3,l))
	u_arr[0,:] = u_arr[0,:]*-70.
	u_arr[1,:] = u_arr[1,:]*-65.
	u_arr[2,:] = u_arr[2,:]*-60.


	exc_g_arr = np.zeros((3,l))
	inh_g_arr = np.zeros((3,l))
	ca_arr = np.zeros(l)
	VT_arr = np.ones(l)*-45.
	dist_VT_arr = np.ones(l)*-20.


	leak_g_arr = np.zeros((3,l))




	tmp_exc_cmpt = np.reshape(np.zeros(3),(3,1))
	tmp_inh_cmpt = np.reshape(np.zeros(3),(3,1))
	spike_cnt = 0

	exc_input_arr = np.zeros([tot_exc,l])
	inh_input_arr = np.zeros([tot_inh,l])



	exc_input_arr[2,1000] = 1


	for t in range(0,l):


		tmp_exc = amp_exc*exc_wt*np.reshape(exc_input_arr[:,t],(tot_exc,1))
		tmp_inh = amp_inh*inh_wt*np.reshape(inh_input_arr[:,t],(tot_inh,1))
		tmp_curr = np.reshape(I_s[:,t],(3,1))

		tmp_exc_cmpt[0,0] = (tmp_exc[0,0])
		tmp_exc_cmpt[1,0] = (tmp_exc[1,0])
		tmp_exc_cmpt[2,0] = (tmp_exc[2,0])
		tmp_inh_cmpt[0,0] = tmp_inh[0,0]
		tmp_inh_cmpt[1,0] = tmp_inh[1,0]
		tmp_inh_cmpt[2,0] = tmp_inh[2,0]
	
	
		neur.compute_u(tmp_exc_cmpt,tmp_inh_cmpt,tmp_curr)
		u_arr[:,t] = neur.u[0:3]
		exc_g_arr[2,t] = neur.u[4]
		inh_g_arr[2,t] = neur.u[6]
		exc_g_arr[1,t] = neur.u[8]
		inh_g_arr[1,t] = neur.u[10]
		exc_g_arr[0,t] = neur.u[12]
		inh_g_arr[0,t] = neur.u[14]

		leak_g_arr[:,t] = np.reshape(neur.G_l,(1,3))
		ca_arr[t] = neur.u[15]*neur.u[16]*neur.gca*(neur.eca-neur.u[2])


	c_p = 18.
	glp = 5.
	gpd = 25.
	gsp = 25.

	gld = 5.
	gls = 5.
	c_d = 13.
	c_s = 43.
	eld = -60.
	elp = -65.
	els = -70.
	leak = -60.000004268845181
	leak = -60.00000001

	eca    = 50.0 	
	gca    = 70.0    
	taum   = 15.0    
	tauh   = 80.0    
	halfm  = -9.0    
	halfh  = -21.0   
	slopem = 0.5     
	slopeh = -0.5   


	nest.ResetKernel()

	# Obtain receptor dictionary
	syns = nest.GetDefaults('iaf_cond_alpha_mc_kinetics')['receptor_types']
	#print "iaf_cond_alpha_mc receptor_types: ", syns

	# Obtain list of recordable quantities
	rqs = nest.GetDefaults('iaf_cond_alpha_mc_kinetics')['recordables']
	#print "iaf_cond_alpha_mc recordables   : ", rqs

	# Change some default values:
	#  - threshold potential
	#  - reset potential
	#  - refractory period
	#  - somato-proximal coupling conductance
	#  - somatic leak conductance
	#  - proximal synaptic time constants
	#  - distal capacitance


	nest.SetDefaults('iaf_cond_alpha_mc_kinetics',
		 { 
		   'g_sp' : gsp,
		   'g_pd' : gpd,
		   'V_th' : 955.0,
		   'act_flag' : 1.0,
		   'jump_Th' : 0.,
		   'tau_Th' : 1.,
		   'distal'  : { 
				 't_L' : gld,
				 'nt_L' : gld,
				 'E_L' : eld,
				 'tau_syn_ex': 1.0,
				 'tau_syn_in': 2.0,
				 'g_L': gld,
				 'C_m': c_d,
				 'amp_cur_AP': 0.,
				 'tau_cur_AP': 1. },
		   'proximal'  : { 
	  		           't_L' : glp,
				   'nt_L' : glp,
				   'E_L' : elp,
				   'tau_syn_ex': 1.0,
				   'tau_syn_in': 2.0,
				   'g_L': glp, 
				   'C_m': c_p,
				   'amp_cur_AP': 0.,
				   'tau_cur_AP': 1.  },
		   'soma'  : { 
			       't_L' : 150.,
	  		       'nt_L' : gls,
			       'E_L' : els,
			       'tau_syn_ex': 1.0,
			       'tau_syn_in': 2.0,
			       'g_L': gls, 
			       'C_m': c_s,
			       'amp_cur_AP': 0.,
			       'tau_cur_AP': 1. },
		 }) 



	# Create neuron
	n = nest.Create('iaf_cond_alpha_mc_kinetics')

	# Create multimeter recording everything, connect
	mm = nest.Create('multimeter', 
			 params = {'record_from': rqs, 
			           'interval': 0.1})
	nest.Connect(mm, n)


	
	sgs = nest.Create('spike_generator', 6)
	nest.SetStatus([sgs[0]],[{'spike_times': [99.0]}]) # distal

	# Connect generators to correct compartments
	nest.Connect([sgs[0]], n, syn_spec = {'receptor_type': syns['distal_exc'],"weight":amp_exc, "delay":1.0})


	sd = nest.Create('spike_detector', 1)
	nest.Connect(n,sd)

	# Simulate 
	nest.Simulate(1500)

	rec = nest.GetStatus(mm)[0]['events']
	t1 = rec['times']

	error = np.sqrt((u_arr[2,:14990]-rec['V_m.d'])**2)
        self.assertTrue(np.max(error) < 4.0e-4)


def suite():

    suite = unittest.makeSuite(KineticsMultiCmptNeuTestCase,'test')
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()


