#!/usr/bin/env python
#-*- coding:utf-8 -*-

""" NEST script to invoke after init_nest_adexp.py """


#
#---
# NEST model
#--------------------

nest.ResetKernel()
nest.SetKernelStatus({"local_num_threads": 10})
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


#
#---
# Time comparison
#--------------------

if with_nngt:   
   # model to use
   models = ["aeif_cond_exp" , "aeif_cond_exp_gridprecise" ]
   
   # time the simulations for each neural model and network size
   sim_time = 20.
   lst_network_sizes = np.arange(100, 4000, 500)
   num_runs = len(lst_network_sizes)
   lst_times = [ np.zeros(num_runs) for _ in range(len(models)) ]
   for i,size in enumerate(lst_network_sizes):
      for j,model in enumerate(models):
         # create the population of neurons and the synaptic strength distribution
         pop = nngt.NeuralPop.ei_population(size, en_model=model, in_model=model)
         # synaptic weight
         weight = 120. if "exp" in model else 33.
         # create the network
         graph = nngt.SpatialNetwork(pop)
         nngt.generation.erdos_renyi(density=0.09, from_graph=graph)
         # in nest
         nest.ResetKernel()
         nest.SetKernelStatus({"local_num_threads": 10})
         subnet, gids = nngt.simulation.make_nest_network(graph)
         dc = nest.Create("dc_generator",params={"amplitude": 800.})
         nest.Connect(dc, gids[::3])
         
         start = time.time()
         nest.Simulate(sim_time)
         lst_times[j][i] = time.time() - start
   
   fig, ax = plt.subplots()
   for i, model in enumerate(models):
      ax.plot(lst_network_sizes, lst_times[i], label=model)
   print("done")
plt.legend()
plt.show()
