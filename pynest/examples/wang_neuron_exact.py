import nest
import matplotlib.pyplot as plt
import numpy as np

nest.rng_seed = 12345

nest.ResetKernel()
w_ex = 40.
w_in = -15.
alpha = 0.5
tau_AMPA = 2.0
tau_GABA = 5.0
tau_NMDA = 100.0
nrn1 = nest.Create("iaf_wang_2002_exact", {"tau_AMPA": tau_AMPA,
                                           "tau_GABA": tau_GABA,
                                           "tau_decay_NMDA": tau_NMDA})

pg = nest.Create("poisson_generator", {"rate": 50.})
sr = nest.Create("spike_recorder")
nrn2 = nest.Create("iaf_wang_2002_exact", {"tau_AMPA": tau_AMPA,
                                           "tau_GABA": tau_GABA,
                                           "tau_decay_NMDA": tau_NMDA,
                                           "t_ref": 0.})

mm1 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})
mm2 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})

ex_syn_spec = {"synapse_model": "static_synapse",
               "weight": w_ex,
               "receptor_type": 1}

nmda_syn_spec = {"synapse_model": "static_synapse",
                 "weight": w_ex,
                 "receptor_type": 3}

in_syn_spec = {"synapse_model": "static_synapse",
               "weight": w_in,
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

nest.Connect(pg, nrn1, syn_spec=ex_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, sr)
nest.Connect(nrn1, nrn2, syn_spec=ex_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=in_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(mm1, nrn1)

nest.Connect(mm2, nrn2)

nest.Simulate(1000.)

# get spike times from membrane potential
# cannot use spike_recorder because we abuse exact spike timing
V_m = mm1.get("events", "V_m")
times = mm1.get("events", "times")
diff = np.ediff1d(V_m, to_begin=0.)
spikes = sr.get("events", "times")
spikes = times[diff < -3]

# compute analytical solutimes = mm1.get("events", "times")
ampa_soln = spiketrain_response(times, tau_AMPA, spikes, w_ex)
nmda_soln = spiketrain_response_nmda(times, tau_NMDA, spikes, w_ex, alpha)
gaba_soln = spiketrain_response(times, tau_GABA, spikes, w_in)

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

