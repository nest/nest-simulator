import nest
import matplotlib.pyplot as plt
import numpy as np

nest.rng_seed = 12345

nest.ResetKernel()

alpha = 0.5
tau_AMPA = 2.0
tau_GABA = 5.0
tau_rise_NMDA = 2.0
tau_NMDA = 100.0

Mg2 = 1.0

t_ref = 2.0

# reversal potentials
E_ex = 0.
E_in = -70.0
E_L = -70.0

V_th = -55.0
V_reset = -70.
C_m = 500.0
g_L = 25.0


# set through synaptic weights
g_AMPA_rec = 1.0
g_AMPA_ext = 100.0
g_GABA = 1.0
g_NMDA = 1.0

neuron_params = {"tau_AMPA": tau_AMPA,
                 "tau_GABA": tau_GABA,
                 "tau_rise_NMDA": tau_rise_NMDA,
                 "tau_decay_NMDA": tau_NMDA, 
                 "conc_Mg2": Mg2,
                 "E_ex": E_ex,
                 "E_in": E_in,
                 "E_L": E_L,
                 "V_th": V_th,
                 "C_m": C_m,
                 "g_L": g_L,
                 "t_ref": t_ref}


nrn1 = nest.Create("iaf_wang_2002_exact", neuron_params)
nrn2 = nest.Create("iaf_wang_2002_exact", neuron_params)

times = np.array([10.0, 20.0, 40.0, 80.0, 90.0])
sg = nest.Create("spike_generator", {"spike_times": times})
sr = nest.Create("spike_recorder")

mm1 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})
mm2 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})

ex_syn_spec = {"synapse_model": "static_synapse",
               "weight": g_AMPA_rec,
               "receptor_type": 1}

ex_syn_spec_ext = {"synapse_model": "static_synapse",
               "weight": g_AMPA_ext,
               "receptor_type": 1}

nmda_syn_spec = {"synapse_model": "static_synapse",
                 "weight": g_NMDA,
                 "receptor_type": 3}

in_syn_spec = {"synapse_model": "static_synapse",
               "weight": g_GABA,
               "receptor_type": 2}

conn_spec = {"rule": "all_to_all"}

def s_soln(w, t, tau):
    isyn = np.zeros_like(t)
    useinds = t >= 0.
    isyn[useinds] = w * np.exp(-t[useinds] / tau)
    return isyn

def spiketrain_response(t, tau, spiketrain, w):
    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1. - sp
        zero_arg = t_ == 0.
        response += s_soln(w, t_, tau)
    return response

def spiketrain_response_nmda(t, tau, spiketrain, w, alpha):
    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1. - sp
        zero_arg = t_ == 0.
        w_ = w * alpha * (1 - response[zero_arg])
        w_ = min(w_, 1 - response[zero_arg])
        response += s_soln(w_, t_, tau)
    return response

nest.Connect(sg, nrn1, syn_spec=ex_syn_spec_ext, conn_spec=conn_spec)
nest.Connect(nrn1, sr)
nest.Connect(nrn1, nrn2, syn_spec=ex_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=in_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(mm1, nrn1)

nest.Connect(mm2, nrn2)

nest.Simulate(300.)

# get spike times from membrane potential
# cannot use spike_recorder because we abuse exact spike timing
V_m = mm1.get("events", "V_m")
times = mm1.get("events", "times")
diff = np.ediff1d(V_m, to_begin=0.)
spikes = sr.get("events", "times")
spikes = times[diff < -3]

# compute analytical solutimes = mm1.get("events", "times")
ampa_soln = spiketrain_response(times, tau_AMPA, spikes, g_AMPA_rec)
nmda_soln = spiketrain_response_nmda(times, tau_NMDA, spikes, g_NMDA, alpha)
gaba_soln = spiketrain_response(times, tau_GABA, spikes, g_GABA)

fig, ax = plt.subplots(4,2)
ax[0,0].plot(mm1.events["V_m"])
ax[0,1].plot(mm2.events["V_m"])

ax[1,0].plot(mm1.events["s_AMPA"])
ax[1,1].plot(mm2.events["s_AMPA"])
ax[1,1].plot(ampa_soln, '--')

ax[2,0].plot(mm1.events["s_GABA"])
ax[2,1].plot(mm2.events["s_GABA"])
ax[2,1].plot(gaba_soln, '--')

ax[3,0].plot(mm1.events["NMDA_sum"])
ax[3,1].plot(mm2.events["NMDA_sum"])
ax[3,1].plot(nmda_soln, '--')

plt.show()

