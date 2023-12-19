import brian2 as b2
import nest
import numpy as np
import time
import statistics
import matplotlib.pyplot as plt



# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Parameters
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

V_th = -55 * b2.mV
V_reset = -70 * b2.mV
t_ref = 2 * b2.ms

# parameters for the equation of the neuron 
# (Inhibitory and excitatory neurons have different parameters)
g_L = 25. * b2.nS
C_m = 0.5 * b2.nF

g_AMPA_rec = 1.0 * b2.nS
g_AMPA_ext = 100.0 *b2.nS
g_GABA = 1.0 * b2.nS
g_NMDA = 1.0 * b2.nS 

# reversal potentials
E_L = V_reset
E_ex= 0. * b2.mV
E_in = -70. * b2.mV

# time constant of the receptors
tau_AMPA= 2 * b2.ms
tau_GABA= 5 * b2.ms
tau_NMDA_rise = 2. * b2.ms
tau_NMDA_decay = 100. * b2.ms
#parameters we need for nmda receptors
alpha = 0.5 / b2.ms
Mg2 = 1.




# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Brian
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

## Equations
eqsE="""

    dv / dt = (- g_L * (v - E_L) - I_syn) / C_m : volt (unless refractory)
    I_syn = I_AMPA_rec + I_AMPA_ext + I_GABA + I_NMDA: amp 
   
    I_AMPA_ext= g_AMPA_ext * (v - E_ex) * s_AMPA_ext : amp
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

eqs_ampa="""
          s_AMPA_tot_post= w_AMPA * s_AMPA : 1 (summed)  
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

nrn1 = b2.NeuronGroup(1, model=eqsE, threshold="v > V_th", reset="v = V_reset", refractory=t_ref, method="euler")
nrn2 = b2.NeuronGroup(1, model=eqsE, threshold="v > V_th", reset="v = V_reset", refractory=t_ref, method="euler")

nrn1[0].v[0]=V_reset
nrn2[0].v[0]=V_reset

times = np.array([10, 20, 40, 80, 90]) * b2.ms
indices = np.arange(len(times))
spikeGen = b2.SpikeGeneratorGroup(len(times), indices, times)

ext_conn1 = b2.Synapses(spikeGen, nrn1, on_pre="s_AMPA_ext += 1")
ext_conn1.connect()

conn2 = b2.Synapses(nrn1, nrn2, model=eqs_ampa, on_pre="s_AMPA+=1", method="euler")
conn2.connect()
conn2.w_AMPA = 2.0

conn3= b2.Synapses(nrn1,nrn2,model=eqs_nmda,on_pre="x+=1", method="euler")
conn3.connect()
conn3.w_NMDA = 1.0

conn4 = b2.Synapses(nrn1, nrn2, model=eqs_gaba, on_pre="s_GABA+=1", method="euler")
conn4.connect()
conn4.w_GABA = 2.0


vMonitor1 = b2.StateMonitor(nrn1, "v",record=True)
ampaMonitor1 = b2.StateMonitor(nrn1, "I_AMPA_rec",record=True)
gabaMonitor1 = b2.StateMonitor(nrn1, "I_GABA",record=True)
nmdaMonitor1 = b2.StateMonitor(nrn2, "s_NMDA_tot", record=True)

vMonitor2 = b2.StateMonitor(nrn2, "v", record=True)
ampaMonitor2 = b2.StateMonitor(nrn2, "I_AMPA_rec", record=True)
gabaMonitor2 = b2.StateMonitor(nrn2, "I_GABA", record=True)
nmdaMonitor2 = b2.StateMonitor(nrn2, "I_NMDA", record=True)

t_sim = 300
b2.run(t_sim * b2.ms)


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# NEST
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

nest.rng_seed = 12345

nest.ResetKernel()

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
                 "t_ref": np.asarray(t_ref) * 1e3}                   # units ms


nrn1 = nest.Create("iaf_wang_2002_exact", neuron_params)
nrn2 = nest.Create("iaf_wang_2002_exact", neuron_params)

times = np.array([10.0, 20.0, 40.0, 80.0, 90.0])
sg = nest.Create("spike_generator", {"spike_times": times})
sr = nest.Create("spike_recorder")

mm1 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})
mm2 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})

ex_syn_spec = {"synapse_model": "static_synapse",
               "weight": np.asarray(g_AMPA_rec) * 1e9,   # units nS
               "receptor_type": 1}

ex_syn_spec_ext = {"synapse_model": "static_synapse",
               "weight": np.asarray(g_AMPA_ext) * 1e9,   # units nS
               "receptor_type": 1}

nmda_syn_spec = {"synapse_model": "static_synapse",
                 "weight": np.asarray(g_NMDA) * 1e9,   # units nS
                 "receptor_type": 3}

in_syn_spec = {"synapse_model": "static_synapse",
               "weight": np.asarray(g_GABA) * 1e9,   # units nS
               "receptor_type": 2}

conn_spec = {"rule": "all_to_all"}

nest.Connect(sg, nrn1, syn_spec=ex_syn_spec_ext, conn_spec=conn_spec)
nest.Connect(nrn1, sr)
nest.Connect(nrn1, nrn2, syn_spec=ex_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=in_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(mm1, nrn1)
nest.Connect(mm2, nrn2)

nest.Simulate(300.)


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Plotting
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

fig, ax = plt.subplots(4, 2)
fig.set_size_inches([12,10])
ax[0,0].plot(vMonitor1.t / b2.ms, vMonitor1.v[0] / b2.mV, label="brian2")
ax[0,0].plot(mm1.get("events", "times"), mm1.get("events", "V_m"), label="nest")
ax[0,0].set_xlabel("time (ms)")
ax[0,0].set_ylabel("membrane potential V (mV)")
ax[0,0].legend()

ax[1,0].plot(ampaMonitor1.t / b2.ms, ampaMonitor1.I_AMPA_rec[0]/)
ax[2,0].plot(gabaMonitor1.t / b2.ms, gabaMonitor1.I_GABA[0])
ax[3,0].plot(nmdaMonitor1.t / b2.ms, nmdaMonitor1.s_NMDA_tot[0])

ax[0,1].plot(vMonitor2.t / b2.ms, vMonitor2.v[0] / b2.mV)
ax[0,1].plot(mm2.get("events", "times"), mm2.get("events", "V_m"))
ax[0,1].set_xlabel("time (ms)")
ax[0,1].set_ylabel("membrane potential V (mV)")
ax[0,1].legend()

ax[1,1].plot(ampaMonitor2.t/b2.ms, ampaMonitor2.I_AMPA_rec[0])
ax[2,1].plot(gabaMonitor2.t/b2.ms, gabaMonitor2.I_GABA[0])
ax[3,1].plot(nmdaMonitor2.t/b2.ms, nmdaMonitor2.I_NMDA[0])

plt.show()

