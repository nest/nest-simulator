import nest

nest.rng_seed = 42

model_params = {
    'E_L': 0.0,  # Resting membrane potential(mV)
    'C_m': 250.0,  # Capacity of the membrane(pF)
    'tau_m': 10.0,  # Membrane time constant(ms)
    't_ref': 0.5,  # Duration of refractory period(ms)
    'V_th': 20.0,  # Threshold(mV)
    'V_reset': 0.0,  # Reset Potential(mV)
    # time const. postsynaptic excitatory currents(ms)
    'tau_syn_ex': 0.326,
    # time const. postsynaptic inhibitory currents(ms)
    'tau_syn_in': 0.326,
    'tau_minus': 30.0,  # time constant for STDP(depression)
    # V can be randomly initialized see below
    'V_m': 5.7  # mean value of membrane potential
}

E_neurons = nest.Create('iaf_psc_alpha', 1000, params=model_params)

I_neurons = nest.Create('iaf_psc_alpha', 200, params=model_params)

stimulus = nest.Create('poisson_generator', 1, {'rate': 300})

recorder = nest.Create('spike_recorder')

nest.Connect(stimulus, E_neurons, {'rule': 'all_to_all'},
             {'synapse_model': 'static_synapse_hpc', 'weight': 1000})
nest.Connect(stimulus, I_neurons, {'rule': 'all_to_all'},
             {'synapse_model': 'static_synapse_hpc', 'weight': 1000})

nest.Connect(E_neurons, E_neurons,
             {'rule': 'fixed_indegree', 'indegree': 1000,
              'allow_autapses': False, 'allow_multapses': True},
             {'synapse_model': 'stdp_pl_synapse_hom_hpc', 'weight': 10})

nest.Connect(I_neurons, E_neurons,
             {'rule': 'fixed_indegree', 'indegree': 200,
              'allow_autapses': False, 'allow_multapses': True},
             {'synapse_model': 'static_synapse_hpc', 'weight': 1000})

nest.Connect(E_neurons, I_neurons,
             {'rule': 'fixed_indegree', 'indegree': 1000,
              'allow_autapses': False, 'allow_multapses': True},
             {'synapse_model': 'static_synapse_hpc', 'weight': 1000})

nest.Connect(I_neurons, I_neurons,
             {'rule': 'fixed_indegree', 'indegree': 200,
              'allow_autapses': False, 'allow_multapses': True},
             {'synapse_model': 'static_synapse_hpc', 'weight': 1000})

nest.Connect(E_neurons + I_neurons, recorder, 'all_to_all', 'static_synapse_hpc')
# nest.Connect(stimulus, recorder, 'all_to_all', 'static_synapse_hpc')

nest.Simulate(100)

all_spikes = nest.GetStatus(recorder, keys='events')[0]
print(f"Num: {len(all_spikes['times'])}, Times: {all_spikes['times']}, Senders: {all_spikes['senders']}")
