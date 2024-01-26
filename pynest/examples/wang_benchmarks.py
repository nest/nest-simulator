"""
Check that NEST implementation gives same results as a reference implementation from Brian.
Note that in the NEST model, the constant "g" parameter is baked into the synaptic weight, instead of being
a separate parameter. This is the only difference in parameterization between the two models.
Also, in the NEST model, the weight for the NMDA receptor is applied AFTER computing the s_NMDA values,
so the in the recorded value of NMDA_sum, the weights are not yet applied. The end result (V_m) is still the same.
"""


import brian2 as b2
import nest
import numpy as np
import time, os
import statistics
import matplotlib.pyplot as plt
from pathlib import Path


path = Path(__file__).parent
outfile = os.path.join(path, "wang_benchmark_log.csv")
if not os.path.isfile(outfile):
    with open(outfile, "w") as f:
        f.write("brian_time,nest_time_exact,nest_time_approx,NE,NI\n")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Parameters
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

simtime = 1000
resolution = 0.01
NE = 200
NI = 200

V_th = -55 * b2.mV
V_reset = -70 * b2.mV
t_ref = 2 * b2.ms

# parameters for the equation of the neuron 
# (Inhibitory and excitatory neurons have different parameters)
g_L = 25. * b2.nS
C_m = 0.5 * b2.nF

g_AMPA_rec = 2.0 * b2.nS
g_AMPA_ext = 1.0 *b2.nS
g_GABA = 1. * b2.nS
g_NMDA = 1. * b2.nS 

# reversal potentials
E_L = V_reset
E_ex= 0. * b2.mV
E_in = -70. * b2.mV

# time constant of the receptors
tau_AMPA= 2 * b2.ms
tau_GABA= 5 * b2.ms
tau_NMDA_rise = 2. * b2.ms
tau_NMDA_decay = 100. * b2.ms

# additional NMDA parameters
alpha = 0.5 / b2.ms
Mg2 = 1.

# synaptic weights
weight_AMPA_ext = 1.
weight_AMPA = 1. * 50 / NE
weight_NMDA = 1. * 50 / NE
weight_GABA = 1. * 50 / NI


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Brian simulation
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

## Equations
eqsE="""

    dv / dt = (- g_L * (v - E_L) - I_syn) / C_m : volt (unless refractory)
    I_syn = I_AMPA_rec + I_AMPA_ext + I_GABA + I_NMDA: amp 

    I_AMPA_ext = g_AMPA_ext * (v - E_ex) * s_AMPA_ext : amp
    ds_AMPA_ext / dt = - s_AMPA_ext / tau_AMPA : 1
    #Here I don"t need the summed variable because the neuron receive inputs from only one Poisson generator. Each neuron need only one s.

    I_AMPA_rec = g_AMPA_rec * (v - E_ex) * 1 * s_AMPA_tot : amp
    s_AMPA_tot : 1  #the eqs_ampa solve many s and sum them and give the summed value here
    #Each neuron receives inputs from many neurons. Each of them has his own differential equation s_AMPA (where I have the deltas with the spikes). 
    #I then sum all the solutions s of the differential equations and I obtain s_AMPA_tot_post.
 
    I_GABA= g_GABA * (v - E_in) * s_GABA_tot : amp
    s_GABA_tot :1

    I_NMDA  = g_NMDA * (v - E_ex) / (1 + Mg2 * exp(-0.062 * v / mV) / 3.57) * s_NMDA_tot : amp
    s_NMDA_tot : 1

 """



eqsI="""

    dv / dt = (- g_L * (v - E_L) - I_syn) / C_m : volt (unless refractory)
    I_syn = I_AMPA_rec + I_AMPA_ext + I_GABA + I_NMDA : amp
    
    I_AMPA_ext= g_AMPA_ext * (v - E_ex) * s_AMPA_ext : amp
    ds_AMPA_ext / dt = - s_AMPA_ext / tau_AMPA : 1
    # Here I don"t need the summed variable because the neuron receive inputs from only one Poisson generator. Each neuron need only one s.
        
    
    I_AMPA_rec = g_AMPA_rec * (v - E_ex) * 1 * s_AMPA_tot : amp
    s_AMPA_tot : 1  #the eqs_ampa solve many s and sum them and give the summed value here
    #Each neuron receives inputs from many neurons. Each of them has his own differential equation s_AMPA (where I have the deltas with the spikes). 
    #I then sum all the solutions s of the differential equations and I obtain s_AMPA_tot_post.
 
     I_GABA= g_GABA * (v - E_in) * s_GABA_tot : amp
     s_GABA_tot :1
     
     I_NMDA  = g_NMDA * (v - E_ex) / (1 + Mg2 * exp(-0.062 * v / mV) / 3.57) * s_NMDA_tot : amp
     s_NMDA_tot : 1
    
     """

eqs_ampa="""
          s_AMPA_tot_post = w_AMPA * s_AMPA : 1 (summed)  
          ds_AMPA / dt = - s_AMPA / tau_AMPA : 1 (clock-driven)
          w_AMPA: 1
        """

eqs_gaba="""
        s_GABA_tot_post= w_GABA* s_GABA : 1 (summed)  
        ds_GABA/ dt = - s_GABA/ tau_GABA : 1 (clock-driven)
        w_GABA: 1
        """

eqs_nmda="""s_NMDA_tot_post = w_NMDA * s_NMDA : 1 (summed)
            ds_NMDA / dt = - s_NMDA / tau_NMDA_decay + alpha * x * (1 - s_NMDA) : 1 (clock-driven)
            dx / dt = - x / tau_NMDA_rise : 1 (clock-driven)
            w_NMDA : 1
         """



#Create the two population with the corresponfing equations:
popE = b2.NeuronGroup(NE, model=eqsE, threshold="v > V_th", reset="v = E_L", refractory=t_ref, method="rk4")
popI= b2.NeuronGroup(NI, model=eqsI, threshold="v > V_th", reset="v = E_L", refractory=t_ref, method="rk4")


#Set the initial value of the potential v for all the neurons
for k in range(0,NE):
    popE[k].v[0]=E_L

for k in range(0,NI):
    popI[k].v[0]=E_L


# Connect the neurons of popE with the neurons of popI (for the ampa connections)
conn= b2.Synapses(popE,popI,model=eqs_ampa,on_pre="s_AMPA+=1", method="rk4")
conn.connect()
conn.w_AMPA= weight_AMPA
conn.delay = 1.0 * b2.ms

# Connect the neurons of popE with the neurons of popI (for the NMDA connections)
conn1= b2.Synapses(popE,popI,model=eqs_nmda,on_pre="x+=1", method="rk4")
conn1.connect()
conn1.w_NMDA= weight_NMDA
conn1.delay = 1.0 * b2.ms

# Connect the neurons of popE with the neurons of popE (for the AMPA connections)
conn2= b2.Synapses(popE,popE,model=eqs_ampa,on_pre="s_AMPA+=1", method="rk4")
conn2.connect()
conn2.w_AMPA= weight_AMPA
conn2.delay = 1.0 * b2.ms

# Connect the neurons of popE with the neurons of popE (for the NMDA connections)
conn3= b2.Synapses(popE,popE,model=eqs_nmda,on_pre="x+=1", method="rk4")
conn3.connect()
conn3.w_NMDA= weight_NMDA
conn3.delay = 1.0 * b2.ms

# Connect the neurons of popI with the neurons of popE (for the GABA connections)
conn4= b2.Synapses(popI,popE,model = eqs_gaba,on_pre="s_GABA+=1", method="rk4")
conn4.connect()
conn4.w_GABA= weight_GABA
conn4.delay = 1.0 * b2.ms

# Connect the neurons of popI with the neurons of popI (for the GABA connections)
conn5= b2.Synapses(popI,popI,model = eqs_gaba,on_pre="s_GABA+=1", method="rk4")
conn5.connect()
conn5.w_GABA= weight_GABA
conn5.delay = 1.0 * b2.ms

# To excitatory neurons
rate_E = 4000 * b2.Hz
ext_inputE = b2.PoissonGroup(NE, rates = rate_E)
ext_connE = b2.Synapses(ext_inputE, popE, on_pre="s_AMPA_ext += 1")
ext_connE.connect(j="i")
ext_connE.delay = 1.0 * b2.ms

# To inhibitory neurons
rate_I=4000 * b2.Hz
ext_inputI= b2.PoissonGroup(NI, rates = rate_I)
ext_connI = b2.Synapses(ext_inputI, popI, on_pre="s_AMPA_ext += 1")
ext_connI.connect(j="i")
ext_connI.delay = 1.0 * b2.ms
 
# Recorder to save the spikes of the neurons
S_e = b2.SpikeMonitor(popE[:NE], record=True) 
S_i = b2.SpikeMonitor(popI[:NI], record=True)

b2.defaultclock.dt = resolution * b2.ms
tic = time.time()
b2.run(simtime * b2.ms)
toc = time.time()
brian_time = toc - tic

brian_espikes = S_e.spike_trains()
brian_espikes = np.array(np.concatenate(tuple(brian_espikes.values()))) * 1000.
brian_ispikes = S_i.spike_trains()
brian_ispikes = np.array(np.concatenate(tuple(brian_ispikes.values()))) * 1000.


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# NEST simulation exact
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

nest.rng_seed = 12345

nest.ResetKernel()
nest.resolution = resolution

neuron_params = {"tau_AMPA": np.asarray(tau_AMPA) * 1e3,             # units ms
                 "tau_GABA": np.asarray(tau_GABA) * 1e3,             # units ms
                 "tau_rise_NMDA": np.asarray(tau_NMDA_rise) * 1e3,   # units ms
                 "tau_decay_NMDA": np.asarray(tau_NMDA_decay) * 1e3, # units ms
                 "conc_Mg2": np.asarray(Mg2),                        # dimensionless
                 "E_ex": np.asarray(E_ex) * 1e3,                     # units mV
                 "E_in": np.asarray(E_in) * 1e3,                     # units mV
                 "E_L": np.asarray(E_L) * 1e3,                       # units mV
                 "V_th": np.asarray(V_th) * 1e3,                     # units mV
                 "C_m": np.asarray(C_m) * 1e12,                      # units pF
                 "g_L": np.asarray(g_L) * 1e9,                       # units nS
                 "V_reset": np.asarray(V_reset) * 1e3,               # units nS
                 "alpha": np.asarray(alpha * b2.ms),                 # units nS
                 # DIFFERENCE: subtract 0.1 ms from t_ref                 
                 "t_ref": np.asarray(t_ref) * 1e3 - b2.defaultclock.dt / b2.ms}                   # units ms



poisson = nest.Create("poisson_generator", {"rate": 4000.})
epop = nest.Create("iaf_wang_2002_exact", NE, params=neuron_params)
ipop = nest.Create("iaf_wang_2002_exact", NI, params=neuron_params)

sr_ex = nest.Create("spike_recorder")
sr_in = nest.Create("spike_recorder")


# DIFFERENCE: add 0.1ms to delay
nest_delay = 1. + b2.defaultclock.dt / b2.ms
ex_syn_spec = {"synapse_model": "static_synapse",
               "weight": np.asarray(g_AMPA_rec) * 1e9 * weight_AMPA,   # units nS
               "receptor_type": 1,
               "delay": nest_delay}

ex_syn_spec_ext = {"synapse_model": "static_synapse",
                   "weight": np.asarray(g_AMPA_ext) * 1e9 * weight_AMPA_ext,   # units nS
                   "receptor_type": 1,
                   "delay": nest_delay}

nmda_syn_spec = {"synapse_model": "static_synapse",
                 "weight": np.asarray(g_NMDA) * 1e9 * weight_NMDA,   # units nS
                 "receptor_type": 3,
                 "delay": nest_delay}

in_syn_spec = {"synapse_model": "static_synapse",
               "weight": -np.asarray(g_GABA) * 1e9 * weight_GABA,   # units nS
               "receptor_type": 2,
               "delay": nest_delay}

conn_spec = {"rule": "all_to_all"}

nest.Connect(poisson, epop + ipop, syn_spec=ex_syn_spec_ext, conn_spec="all_to_all")
nest.Connect(epop, sr_ex)
nest.Connect(ipop, sr_in)
nest.Connect(epop, epop + ipop, syn_spec=ex_syn_spec, conn_spec=conn_spec)
nest.Connect(epop, epop + ipop, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(ipop, ipop + epop, syn_spec=in_syn_spec, conn_spec=conn_spec)

tic = time.time()
nest.Simulate(simtime)
toc = time.time()
nest_time_exact = toc - tic

nest_espikes_exact = sr_ex.get("events", "times")
nest_ispikes_exact = sr_in.get("events", "times")
 

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# NEST simulation approximate
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

nest.rng_seed = 12345

nest.ResetKernel()
nest.resolution = resolution

neuron_params = {"tau_AMPA": np.asarray(tau_AMPA) * 1e3,             # units ms
                 "tau_GABA": np.asarray(tau_GABA) * 1e3,             # units ms
                 "tau_rise_NMDA": np.asarray(tau_NMDA_rise) * 1e3,   # units ms
                 "tau_decay_NMDA": np.asarray(tau_NMDA_decay) * 1e3, # units ms
                 "conc_Mg2": np.asarray(Mg2),                        # dimensionless
                 "E_ex": np.asarray(E_ex) * 1e3,                     # units mV
                 "E_in": np.asarray(E_in) * 1e3,                     # units mV
                 "E_L": np.asarray(E_L) * 1e3,                       # units mV
                 "V_th": np.asarray(V_th) * 1e3,                     # units mV
                 "C_m": np.asarray(C_m) * 1e12,                      # units pF
                 "g_L": np.asarray(g_L) * 1e9,                       # units nS
                 "V_reset": np.asarray(V_reset) * 1e3,               # units nS
                 "alpha": np.asarray(alpha * b2.ms),                 # units nS
                 # DIFFERENCE: subtract 0.1 ms from t_ref                 
                 "t_ref": np.asarray(t_ref) * 1e3 - b2.defaultclock.dt / b2.ms}                   # units ms



poisson = nest.Create("poisson_generator", {"rate": 4000.})
epop = nest.Create("iaf_wang_2002", NE, params=neuron_params)
ipop = nest.Create("iaf_wang_2002", NI, params=neuron_params)

sr_ex = nest.Create("spike_recorder")
sr_in = nest.Create("spike_recorder")

# DIFFERENCE: add 0.1ms to delay
nest_delay = 1. + b2.defaultclock.dt / b2.ms
ex_syn_spec = {"synapse_model": "static_synapse",
               "weight": np.asarray(g_AMPA_rec) * 1e9 * weight_AMPA,   # units nS
               "receptor_type": 1,
               "delay": nest_delay}

ex_syn_spec_ext = {"synapse_model": "static_synapse",
                   "weight": np.asarray(g_AMPA_ext) * 1e9 * weight_AMPA_ext,   # units nS
                   "receptor_type": 1,
                   "delay": nest_delay}

nmda_syn_spec = {"synapse_model": "static_synapse",
                 "weight": np.asarray(g_NMDA) * 1e9 * weight_NMDA,   # units nS
                 "receptor_type": 3,
                 "delay": nest_delay}

in_syn_spec = {"synapse_model": "static_synapse",
               "weight": -np.asarray(g_GABA) * 1e9 * weight_GABA,   # units nS
               "receptor_type": 2,
               "delay": nest_delay}

conn_spec = {"rule": "all_to_all"}

nest.Connect(poisson, epop + ipop, syn_spec=ex_syn_spec_ext, conn_spec="all_to_all")
nest.Connect(epop, sr_ex)
nest.Connect(ipop, sr_in)
nest.Connect(epop, epop + ipop, syn_spec=ex_syn_spec, conn_spec=conn_spec)
nest.Connect(epop, epop + ipop, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(ipop, ipop + epop, syn_spec=in_syn_spec, conn_spec=conn_spec)

tic = time.time()
nest.Simulate(simtime)
toc = time.time()
nest_time_approx = toc - tic

with open(outfile, "a") as f:
    f.write(f"{brian_time},{nest_time_exact},{nest_time_approx},{NE},{NI}\n")

nest_espikes_approx = sr_ex.get("events", "times")
nest_ispikes_approx = sr_in.get("events", "times")


# Plotting
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

print(f"Time NEST exact: {nest_time_exact}")
print(f"Time NEST approx: {nest_time_approx}")
print(f"Time Brian2 exact: {brian_time}")

print(f"Total excitatory spikes Brian exact: {len(brian_espikes)}")
print(f"Total inhibitory spikes Brian exact: {len(brian_ispikes)}")

print(f"Total excitatory spikes NEST exact: {len(nest_espikes_exact)}")
print(f"Total inhibitory spikes NEST exact: {len(nest_ispikes_exact)}")

print(f"Total excitatory spikes NEST approx: {len(nest_espikes_approx)}")
print(f"Total inhibitory spikes NEST approx: {len(nest_ispikes_approx)}")

bins = np.arange(0, simtime+1, 1) - 0.001
nest_ehist_exact, _ = np.histogram(nest_espikes_exact, bins=bins)
nest_ihist_exact, _ = np.histogram(nest_ispikes_exact, bins=bins)

nest_ehist_approx, _ = np.histogram(nest_espikes_approx, bins=bins)
nest_ihist_approx, _ = np.histogram(nest_ispikes_approx, bins=bins)

brian_ehist, _ = np.histogram(brian_espikes, bins=bins)
brian_ihist, _ = np.histogram(brian_ispikes, bins=bins)

fig, ax = plt.subplots(ncols=2, nrows=3, sharex=True, sharey=True)
ax[0,0].plot(nest_ehist_exact * 1000 / NE)
ax[1,0].plot(nest_ehist_approx * 1000 / NE)
ax[2,0].plot(brian_ehist * 1000 / NE)

ax[0,1].plot(nest_ihist_exact * 1000 / NI)
ax[1,1].plot(nest_ihist_approx * 1000 / NI)
ax[2,1].plot(brian_ihist * 1000 / NI)

ax[0,0].set_title("excitatory")
ax[0,1].set_title("inhibitory")
ax[2,0].set_xlabel("time (ms)")
ax[2,1].set_xlabel("time (ms)")
ax[0,0].set_ylabel("NEST exact")
ax[1,0].set_ylabel("NEST approx")
ax[2,0].set_ylabel("Brian")
plt.show()

