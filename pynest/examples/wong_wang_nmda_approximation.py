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

nrn1 = nest.Create("iaf_wang_2002", params_approx)
pg = nest.Create("inhomogeneous_poisson_generator", {"rate_values": [400., 0.],
                                                     "rate_times": [0.1, 10.]})
sr = nest.Create("spike_recorder")
nrn2 = nest.Create("iaf_wang_2002", params_approx)

nrn3 = nest.Create("iaf_wang_2002_exact", params_exact)

mm1 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})
mm2 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})
mm3 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "NMDA_sum", "s_GABA"], "interval": 0.1})

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


times = mm3.get("events", "times")
nest_nmda = mm3.get("events", "NMDA_sum")
nest_nmda_approx = mm2.get("events", "s_NMDA")
times -= times[(nest_nmda > 0).argmax()] - 0.1




from scipy.integrate import cumtrapz
def nmda_integrand(t, x0, tau_decay, tau_rise, alpha):
    a = (1 / tau_decay - 1 / tau_rise)

    # argument of exp
    arg = t * a  + alpha * x0 * tau_rise * (1 - np.exp(- t / tau_rise))
    return np.exp(arg)

def nmda_fn(t, s0, x0, tau_decay, tau_rise, alpha):
    f1 = np.exp(- t / tau_decay - alpha * x0 * tau_rise * (1 - np.exp(-t / tau_rise)))
    
    tvec = np.linspace(0, t, 1001)
    integrand = nmda_integrand(tvec, x0, tau_decay, tau_rise, alpha)
#    f2 = alpha * x0 * np.trapz(integrand, x = tvec) + s0
    f2 = np.trapz(integrand, x = tvec)
    return f1, f2 # f1 * f2

def nmda_fn_approx(t, x0, tau_decay, tau_rise, alpha):
    f1 = np.exp(-t / tau_decay - alpha * x0 * tau_rise * (1 - np.exp(-t / tau_rise)))
    f2 = (np.exp((1 - np.exp(-t / tau_rise)) * alpha * tau_rise) - 1) / alpha

    return f1, f2


t = np.arange(0.1, 1000, 0.1)
s0 = 0.
f1s, f2s = [], []
for t_ in t:
    f1, f2 = nmda_fn(t_, 0., 1., 100., 2., 0.5)
    f1s.append(f1)
    f2s.append(f2)

s_nmda = np.array([f1s[i] * (f2s[i] * 0.5 + s0) for i, _ in enumerate(f1s)])

f1_approx, f2_approx = nmda_fn_approx(t, 1, 100, 2, 0.5)
s_nmda_approx = f1_approx * (f2_approx * 0.5 + s0)

def nmda_approx_exp(t, tau_rise, alpha, tau_decay):
    f1 = np.exp(-alpha * tau_rise * (1 - np.exp(-t / tau_rise)))
    f2 = -(1 - np.exp(alpha * tau_rise * (1 - np.exp(-t / tau_rise))))
    return f1, f2

f1_exp, f2_exp = nmda_approx_exp(t, 2.0, 0.5, 100)

i = 40
s_nmda_exp = (f1_exp[i] * f2_exp[i] + f1_exp[i] * s0) * np.exp(-t / 100)

plt.plot(t, s_nmda, color="C0")
plt.plot(t, s_nmda_approx, color="C1")
plt.plot(t, s_nmda_exp, "--", color="C2")

plt.show()




from scipy.special import expn, gamma
def limfun(tau_rise, tau_decay, alpha):
    f0 = np.exp(-alpha * tau_rise)

    at = alpha * tau_rise
    tr_td = tau_rise / tau_decay
    f1 = -at * expn(tr_td, at) + at ** tr_td * gamma(1 - tr_td)
    
    return f0, f1

f0, f1 = limfun(2, 100, 0.5)




plt.plot(t, s_nmda)
plt.plot(times, nest_nmda)
plt.plot(times, nest_nmda_approx)
plt.plot(t, f1 * np.exp(-t / 100))

plt.show()



