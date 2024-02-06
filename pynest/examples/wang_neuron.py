"""
docstring
"""
import nest
import matplotlib.pyplot as plt
import numpy as np

nest.ResetKernel()
nest.rng_seed = 12345

w_ext = 40.
w_ex = 1.
w_in = -15.

params_exact = {"tau_AMPA": 2.0,
                "tau_GABA": 5.0,
                "tau_rise_NMDA": 2.0,
                "tau_decay_NMDA": 100.0,
                "conc_Mg2": 1.0,
                "E_ex": 0.0,
                "E_in": -70.0,
                "E_L": -70.0,
                "V_th": -55.0,
                "C_m": 500.0,
                "g_L": 25.0,
                "V_reset": -70.0,
                "alpha": 0.5,
                "t_ref": 2.0}

params_approx = params_exact.copy()
params_approx["approx_t_exact"] = 200

nrn1 = nest.Create("iaf_wang_2002", params_approx)
pg = nest.Create("poisson_generator", {"rate": 50.})
sr = nest.Create("spike_recorder")
nrn2 = nest.Create("iaf_wang_2002", params_approx)

nrn3 = nest.Create("iaf_wang_2002_exact", params_exact)

mm1 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})
mm2 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})
mm3 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})

ampa_ext_syn_spec = {"synapse_model": "static_synapse",
                     "weight": w_ext,
                     "receptor_type": 1}

ampa_syn_spec = {"synapse_model": "static_synapse",
               "weight": w_ex,
               "receptor_type": 1}

nmda_syn_spec = {"synapse_model": "static_synapse",
               "weight": w_ex,
               "receptor_type": 3}

gaba_syn_spec = {"synapse_model": "static_synapse",
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

nest.Connect(pg, nrn1, syn_spec=ampa_ext_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, sr)
nest.Connect(nrn1, nrn2, syn_spec=ampa_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=gaba_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn3, syn_spec=ampa_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn3, syn_spec=gaba_syn_spec, conn_spec=conn_spec)
nest.Connect(nrn1, nrn3, syn_spec=nmda_syn_spec, conn_spec=conn_spec)
nest.Connect(mm1, nrn1)

nest.Connect(mm2, nrn2)
nest.Connect(mm3, nrn3)

nest.Simulate(1000.)

# get spike times from membrane potential
# cannot use spike_recorder because we abuse exact spike timing
V_m = mm1.get("events", "V_m")
times = mm1.get("events", "times")
diff = np.ediff1d(V_m, to_begin=0.)
spikes = sr.get("events", "times")
spikes = times[diff < -3]

def nmda_integrand(t, x0, tau_decay, tau_rise, alpha):
    a = (1 / tau_decay - 1 / tau_rise)

    # argument of exp
    arg = t * a  + alpha * x0 * tau_rise * (1 - np.exp(- t / tau_rise))
    return np.exp(arg)

def nmda_fn(t, s0, x0, tau_decay, tau_rise, alpha):
    f1 = np.exp(- t / tau_decay + alpha * x0 * tau_rise * (1 - np.exp(-t / tau_rise)))
    
    tvec = np.linspace(0, t, 1001)
    f2 = alpha * x0 * np.trapz(nmda_integrand(tvec, x0, tau_decay, tau_rise, alpha), x = tvec) + s0
    return f1, f2

fig, ax = plt.subplots(4,2)
fig.set_size_inches([12,10])
fig.subplots_adjust(hspace=0.5)

ax[0,0].plot(mm1.events["V_m"])
ax[0,0].set_xlabel("time (ms)")
ax[0,0].set_ylabel("membrane potential V (mV)")
ax[0,0].legend()
ax[0,0].set_title("Presynaptic neuron")

ax[0,1].plot(mm2.events["V_m"])
ax[0,1].plot(mm3.events["V_m"], "--")
ax[0,1].set_xlabel("time (ms)")
ax[0,1].set_ylabel("membrane potential V (mV)")
ax[0,1].set_title("Postsynaptic neuron")


ax[1,1].plot(mm2.events["s_AMPA"])
ax[1,1].plot(mm3.events["s_AMPA"], "--")
ax[1,1].set_xlabel("time (ms)")
ax[1,1].set_ylabel("s_AMPA")


ax[2,1].plot(mm2.events["s_GABA"])
ax[2,1].plot(mm3.events["s_GABA"], "--")
ax[2,1].set_xlabel("time (ms)")
ax[2,1].set_ylabel("s_GABA")


ax[3,1].plot(mm2.events["s_NMDA"])
ax[3,1].plot(mm3.events["s_NMDA"], "--")
ax[3,1].set_xlabel("time (ms)")
ax[3,1].set_ylabel("s_NMDA")

ax[1,0].axis("off")
ax[2,0].axis("off")
ax[3,0].axis("off")

plt.show()

