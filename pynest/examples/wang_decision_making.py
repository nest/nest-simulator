"""
docstring
"""

import nest
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
import numpy as np

np.random.seed(1234)
rng = np.random.default_rng()

model = "iaf_wang_2002"

dt = 0.1
nest.set(resolution=dt, print_time=True)

g_AMPA_ex = 0.05
g_AMPA_ext_ex = 2.1
g_NMDA_ex = 0.165
g_GABA_ex = 1.3

g_AMPA_in = 0.04
g_AMPA_ext_in = 1.62
g_NMDA_in = 0.13
g_GABA_in = 1.0


# Parameters from paper
epop_params = {"tau_GABA": 5.0,
               "tau_AMPA": 2.0,
               "tau_decay_NMDA": 100.0,
               "tau_rise_NMDA": 2.0,
               "alpha": 0.5,
               "conc_Mg2": 1.0,
               "g_L": 25.,               # leak conductance
               "E_L": -70.0,            # leak reversal potential
               "E_ex": 0.0,             # excitatory reversal potential
               "E_in": -70.0,           # inhibitory reversal potential
               "V_reset": -55.0,        # reset potential
               "V_th": -50.0,           # threshold
               "C_m": 500.0,            # membrane capacitance
               "t_ref": 2.0             # refreactory period
               }


ipop_params = {"tau_GABA": 5.0,
               "tau_AMPA": 2.0,
               "tau_decay_NMDA": 100.0,
               "tau_rise_NMDA": 2.0,
               "alpha": 0.5,
               "conc_Mg2": 1.0,
               "g_L": 20.,               # leak conductance
               "E_L": -70.0,            # leak reversal potential
               "E_ex": 0.0,             # excitatory reversal potential
               "E_in": -70.0,           # inhibitory reversal potential
               "V_reset": -55.0,        # reset potential
               "V_th": -50.0,           # threshold
               "C_m": 200.0,            # membrane capacitance
               "t_ref": 1.0             # refreactory period
               }

simtime = 4000.
signal_start = 1000.
signal_duration = 2000.
signal_update_interval = 50.
f = 0.15 # proportion of neurons receiving signal inputs
w_plus = 1.7
w_minus = 1 - f * (w_plus - 1) / (1 - f)
delay = 0.5

NE = 1600
NI = 400

selective_pop1 = nest.Create(model, int(0.15 * NE), params=epop_params)
selective_pop2 = nest.Create(model, int(0.15 * NE), params=epop_params)
nonselective_pop = nest.Create(model, int(0.7 * NE), params=epop_params)
inhibitory_pop = nest.Create(model, NI, params=ipop_params)

mu_0 = 40.
rho_a = mu_0 / 100
rho_b = rho_a
c = 0.
sigma = 4.
mu_a = mu_0 + rho_a * c
mu_b = mu_0 - rho_b * c

num_updates = int(signal_duration / signal_update_interval)
update_times = np.arange(0, signal_duration, signal_update_interval)
update_times[0] = 0.1
rates_a = np.random.normal(mu_a, sigma, size=num_updates)
rates_b = np.random.normal(mu_b, sigma, size=num_updates)

poisson_a = nest.Create("inhomogeneous_poisson_generator",
                        params={"origin": signal_start-0.1,
                                "start": 0.,
                                "stop": signal_duration,
                                "rate_times": update_times,
                                "rate_values": rates_a})

poisson_b = nest.Create("inhomogeneous_poisson_generator",
                        params={"origin": signal_start-0.1,
                                "start": 0.,
                                "stop": signal_duration,
                                "rate_times": update_times,
                                "rate_values": rates_b})

poisson_0 = nest.Create("poisson_generator", params={"rate": 2400.})

syn_spec_pot_AMPA = {"synapse_model": "static_synapse", "weight":w_plus * g_AMPA_ex, "delay":delay, "receptor_type": 1}
syn_spec_pot_NMDA = {"synapse_model": "static_synapse", "weight":w_plus * g_NMDA_ex, "delay":delay, "receptor_type": 3}

syn_spec_dep_AMPA = {"synapse_model": "static_synapse", "weight":w_minus * g_AMPA_ex, "delay":delay, "receptor_type": 1}
syn_spec_dep_NMDA = {"synapse_model": "static_synapse", "weight":w_minus * g_NMDA_ex, "delay":delay, "receptor_type": 3}

ie_syn_spec = {"synapse_model": "static_synapse", "weight": -1.0 * g_GABA_ex, "delay":delay, "receptor_type": 2}
ii_syn_spec = {"synapse_model": "static_synapse", "weight": -1.0 * g_GABA_in, "delay":delay, "receptor_type": 2}

ei_syn_spec_AMPA = {"synapse_model": "static_synapse", "weight": 1.0 * g_AMPA_in, "delay":delay, "receptor_type": 1}
ei_syn_spec_NMDA = {"synapse_model": "static_synapse", "weight": 1.0 * g_NMDA_in, "delay":delay, "receptor_type": 3}
ee_syn_spec_AMPA = {"synapse_model": "static_synapse", "weight": 1.0 * g_AMPA_ex, "delay":delay, "receptor_type": 1}
ee_syn_spec_NMDA = {"synapse_model": "static_synapse", "weight": 1.0 * g_NMDA_ex, "delay":delay, "receptor_type": 3}

exte_syn_spec = {"synapse_model": "static_synapse", "weight":g_AMPA_ext_ex, "delay":0.1, "receptor_type": 1}
exti_syn_spec = {"synapse_model": "static_synapse", "weight":g_AMPA_ext_in, "delay":0.1, "receptor_type": 1}

sr_nonselective = nest.Create("spike_recorder")
sr_selective1 = nest.Create("spike_recorder")
sr_selective2 = nest.Create("spike_recorder")
sr_inhibitory = nest.Create("spike_recorder")

mm_selective1 = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})
mm_selective2 = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})
mm_nonselective = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})
mm_inhibitory = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})



# # # Create connections

# from external
nest.Connect(poisson_0, nonselective_pop + selective_pop1 + selective_pop2, conn_spec="all_to_all", syn_spec=exte_syn_spec)
nest.Connect(poisson_0, inhibitory_pop, conn_spec="all_to_all", syn_spec=exti_syn_spec)

nest.Connect(poisson_a, selective_pop1, conn_spec="all_to_all", syn_spec=exte_syn_spec)
nest.Connect(poisson_b, selective_pop2, conn_spec="all_to_all", syn_spec=exte_syn_spec)

# from nonselective pop
nest.Connect(nonselective_pop, selective_pop1 + selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_AMPA)
nest.Connect(nonselective_pop, selective_pop1 + selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_NMDA)

nest.Connect(nonselective_pop, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_AMPA)
nest.Connect(nonselective_pop, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_NMDA)

nest.Connect(nonselective_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_AMPA)
nest.Connect(nonselective_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_NMDA)

nest.Connect(nonselective_pop, sr_nonselective)

# from selective pops
nest.Connect(selective_pop1, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_pot_AMPA)
nest.Connect(selective_pop1, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_pot_NMDA)

nest.Connect(selective_pop2, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_pot_AMPA)
nest.Connect(selective_pop2, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_pot_NMDA)

nest.Connect(selective_pop1, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_AMPA)
nest.Connect(selective_pop1, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_NMDA)

nest.Connect(selective_pop2, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_dep_AMPA)
nest.Connect(selective_pop2, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_dep_NMDA)

nest.Connect(selective_pop1 + selective_pop2, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_AMPA)
nest.Connect(selective_pop1 + selective_pop2, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_NMDA)

nest.Connect(selective_pop1 + selective_pop2, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_AMPA)
nest.Connect(selective_pop1 + selective_pop2, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_NMDA)

nest.Connect(selective_pop1, sr_selective1)
nest.Connect(selective_pop2, sr_selective2)

# from inhibitory pop
nest.Connect(inhibitory_pop, selective_pop1 + selective_pop2 + nonselective_pop, conn_spec="all_to_all", syn_spec=ie_syn_spec)
nest.Connect(inhibitory_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ii_syn_spec)

nest.Connect(inhibitory_pop, sr_inhibitory)


# multimeters
nest.Connect(mm_selective1, selective_pop1[0])
nest.Connect(mm_selective2, selective_pop2[0])
nest.Connect(mm_nonselective, nonselective_pop[0])
nest.Connect(mm_inhibitory, inhibitory_pop[0])

nest.Simulate(5000.)

spikes_nonselective = sr_nonselective.get("events", "times")
spikes_selective1 = sr_selective1.get("events", "times")
spikes_selective2 = sr_selective2.get("events", "times")
spikes_inhibitory = sr_inhibitory.get("events", "times")

vm_nonselective = mm_nonselective.get("events", "V_m")
s_AMPA_nonselective = mm_nonselective.get("events", "s_AMPA")
s_GABA_nonselective = mm_nonselective.get("events", "s_GABA")
s_NMDA_nonselective = mm_nonselective.get("events", "s_NMDA")

vm_selective1 = mm_selective1.get("events", "V_m")
s_AMPA_selective1 = mm_selective1.get("events", "s_AMPA")
s_GABA_selective1 = mm_selective1.get("events", "s_GABA")
s_NMDA_selective1 = mm_selective1.get("events", "s_NMDA")

vm_selective2 = mm_selective2.get("events", "V_m")
s_AMPA_selective2 = mm_selective2.get("events", "s_AMPA")
s_GABA_selective2 = mm_selective2.get("events", "s_GABA")
s_NMDA_selective2 = mm_selective2.get("events", "s_NMDA")

vm_inhibitory = mm_inhibitory.get("events", "V_m")
s_AMPA_inhibitory = mm_inhibitory.get("events", "s_AMPA")
s_GABA_inhibitory = mm_inhibitory.get("events", "s_GABA")
s_NMDA_inhibitory = mm_inhibitory.get("events", "s_NMDA")


res = 1.0
bins = np.arange(0, 4001, res) - 0.001

fig, ax = plt.subplots(ncols=2, nrows=2, sharex=True, sharey=True)
fig.tight_layout()
d = NE * f * (res / 1000)
hist1, _ = np.histogram(spikes_selective1, bins=bins)
hist2, _ = np.histogram(spikes_selective2, bins=bins)

ax[0,0].plot(hist1 / d)
ax[0,0].set_title("Selective pop A")
ax[0,1].plot(hist2 / d)
ax[0,1].set_title("Selective pop B")

d = NE * (1 - 2*f) * res / 1000
hist, _ = np.histogram(spikes_nonselective, bins=bins)
ax[1,0].plot(hist / d)
ax[1,0].set_title("Nonselective pop")

d = NI * res / 1000
hist, _ = np.histogram(spikes_inhibitory, bins=bins)
ax[1,1].plot(hist / d)
ax[1,1].set_title("Inhibitory pop")



fig, ax = plt.subplots(ncols=4, nrows=4, sharex=True, sharey="row")
fig.tight_layout()


ax[0,0].plot(s_AMPA_selective1)
ax[0,1].plot(s_AMPA_selective2)
ax[0,2].plot(s_AMPA_nonselective)
ax[0,3].plot(s_AMPA_inhibitory)

ax[1,0].plot(s_NMDA_selective1)
ax[1,1].plot(s_NMDA_selective2)
ax[1,2].plot(s_NMDA_nonselective)
ax[1,3].plot(s_NMDA_inhibitory)

ax[2,0].plot(s_GABA_selective1)
ax[2,1].plot(s_GABA_selective2)
ax[2,2].plot(s_GABA_nonselective)
ax[2,3].plot(s_GABA_inhibitory)

ax[3,0].plot(vm_selective1)
ax[3,1].plot(vm_selective2)
ax[3,2].plot(vm_nonselective)
ax[3,3].plot(vm_inhibitory)


ax[0,0].set_ylabel("S_AMPA")
ax[1,0].set_ylabel("S_NMDA")
ax[2,0].set_ylabel("S_GABA")
ax[3,0].set_ylabel("V_m")

ax[0,0].set_title("Selective pop1")
ax[0,1].set_title("Selective pop2")
ax[0,2].set_title("Nonselective pop")
ax[0,3].set_title("Inhibitory pop")

ax[0,0].set_title("Selective pop1")
ax[0,1].set_title("Selective pop2")
ax[0,2].set_title("Nonselective pop")
ax[0,3].set_title("Inhibitory pop")


plt.show()

