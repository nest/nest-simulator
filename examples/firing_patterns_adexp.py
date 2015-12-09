#!/usr/bin/env python
#-*- coding:utf-8 -*-

""" NEST scipt to invoke after init_nest_adexp.py """


#
#---
# NEST
#--------------------

nest.ResetKernel()
r_resolution = 0.01
nest.SetKernelStatus({"resolution":r_resolution})
d_step_current = 160.
r_min_voltage = -70.

# create AdExp neurons
di_param = {
    'V_reset': -48.,
    'V_peak': 0.0,
    'V_th': -50.,
    'I_e': 0.0,
    'g_L': 12.,
    'tau_w': 130.,
    'E_L': -60.,
    'Delta_T': 2.,
    'a': -11.,
    'b': 30.,
    'C_m': 100.,
    'V_m': -60.
}

# models
lst_names = [ "aeif_cond_alpha", "aeif_cond_alpha_RK5", "aeif_cond_exp_gridprecise" ]
lst_neurons = [ nest.Create(name,params=di_param) for name in lst_names ]
num_neurons = len(lst_neurons)

#~ di_param_tmp = di_param.copy()
#~ # get nullclines
#~ daa_v_null_init = V_nullcline(r_min_voltage,0,1000,di_param_tmp)
#~ di_param_tmp["I_e"] = d_step_current
#~ daa_v_null = V_nullcline(r_min_voltage,0,1000,di_param_tmp)
#~ daa_w_null = w_nullcline(r_min_voltage,0,1000,di_param_tmp)

# create the step current generator to compare the dynamics of the different neurons
step_gen = nest.Create("step_current_generator",1,{"amplitude_times": [50.,1500.], "amplitude_values":[d_step_current,0.]})
multimeter = nest.Create("multimeter",num_neurons)
nest.SetStatus(multimeter, {"withtime":True, "interval":r_resolution, "record_from":["V_m","w"]})

for i,neuron in enumerate(lst_neurons):
   nest.Connect(step_gen,neuron)
   nest.Connect(multimeter[i],neuron[0])

nest.Simulate(1600.0)


#
#---
# Plotting
#--------------------
plt.close("all")

plt.figure(1)

# get the neuron's membrane potential
for i in range(num_neurons):
   dmm = nest.GetStatus(multimeter)[i]
   da_voltage = dmm["events"]["V_m"]
   da_adapt = dmm["events"]["w"]
   da_time = dmm["events"]["times"]
   plt.plot(da_time,da_voltage,c=cm.hot(i/float(num_neurons)), label=lst_names[i])

plt.legend()
plt.xlabel('Time (ms)')
plt.ylabel('Voltage (mV)')

#~ plt.figure(2)
#~ plt.plot(daa_v_null_init[0],daa_v_null_init[1],'b:')
#~ plt.plot(daa_v_null[0],daa_v_null[1],'b--')
#~ plt.plot(daa_w_null[0],daa_w_null[1],'b--')
#~ # insert nan at spike time to get discontinuous lines
#~ pos = np.where(np.abs(da_voltage-di_param['V_reset'])<0.01)[0]
#~ pos2 = np.where(np.abs(np.diff(da_voltage))>=5)[0]+1
#~ pos = np.intersect1d(pos,pos2)
#~ da_lim = da_adapt[pos-1]
#~ pos += np.arange(0,2*len(pos),2).astype(int)
#~ for i,val in enumerate(pos):
	#~ da_adapt = np.insert(da_adapt, val, [da_lim[i],np.nan])
	#~ da_voltage = np.insert(da_voltage, val, [di_param['V_peak'],np.nan])
#~ plt.plot(da_voltage,da_adapt,'k')
#~ plt.scatter(np.repeat(di_param['V_reset'],len(pos)),da_lim+di_param['b'],edgecolors='k',facecolors='none',marker='s')
#~ plt.scatter(di_param['V_m'],0,edgecolors='k',facecolors='k',marker='o')
#~ plt.scatter(da_voltage[-1],da_adapt[-1],edgecolors='k',facecolors='none',marker='+')
#~ 
#~ plt.xlim(r_min_voltage, -35)
#~ plt.ylim(-150, 200)
#~ plt.xlabel('Voltage (mV)')
#~ plt.ylabel('Current (pA)')
plt.show()
