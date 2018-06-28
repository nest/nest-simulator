

    
    
Compare Tsodyks-Markram Synapse models.
---------------------------------------

This script compares the two variants of the Tsodyks/Markram synapse in NEST.

    
    import nest
    import nest.voltage_trace
    import numpy
    import pylab
    
    nest.ResetKernel()
    n_syn = 12.0  # number of synapses in a connection
    n_trials = 30  # number of measurement trials
    
    fac_params = {"U": 0.03, "u": 0.03, "tau_fac": 500.,
                  "tau_rec": 200., "weight": 1.}
    dep_params = {"U": 0.5, "u": 0.5, "tau_fac": 15.,
                  "tau_rec": 670., "weight": 1.}
    lin_params = {"U": 0.3, "u": 0.3, "tau_fac": 330.,
                  "tau_rec": 330., "weight": 1.}
    
    t1_params = fac_params  # for tsodyks2_synapse
    t2_params = t1_params.copy()  # for furhmann_synapse
    
    t2_params['n'] = n_syn
    t2_params['weight'] = 1. / n_syn
    
    nest.SetDefaults("tsodyks2_synapse", t1_params)
    nest.SetDefaults("quantal_stp_synapse", t2_params)
    nest.SetDefaults("iaf_psc_exp", {"tau_syn_ex": 3., 'tau_m': 70.})
    
    source = nest.Create('spike_generator')
    nest.SetStatus(source, {'spike_times':
                            [30., 60., 90., 120., 150., 180., 210., 240., 270.,
                             300., 330., 360., 390., 900.]})
    
    parrot = nest.Create('parrot_neuron')
    neuron = nest.Create("iaf_psc_exp", 2)
    
    nest.Connect(source, parrot)
    nest.Connect(parrot, neuron[:1], syn_spec="tsodyks2_synapse")
    nest.Connect(parrot, neuron[1:], syn_spec="quantal_stp_synapse")
    
    voltmeter = nest.Create("voltmeter", 2)
    nest.SetStatus(voltmeter, {"withgid": False, "withtime": True})
    t_plot = 1000.
    t_tot = 1500.
    
the following is a dry run trial so that the synapse dynamics is
idential in all subsequent trials.

    
    nest.Simulate(t_tot)
    
Now we connect the voltmeters

    nest.Connect([voltmeter[0]], [neuron[0]])
    nest.Connect([voltmeter[1]], [neuron[1]])
    
WE now run the specified number of trials in a loop.

    
    for t in range(n_trials):
        t_net = nest.GetKernelStatus('time')
        nest.SetStatus(source, {'origin': t_net})
        nest.Simulate(t_tot)
    
    nest.Simulate(.1)  # flush the last voltmeter events from the queue
    
    vm = numpy.array(nest.GetStatus([voltmeter[1]], 'events')[0]['V_m'])
    vm_reference = numpy.array(nest.GetStatus([voltmeter[0]], 'events')[0]['V_m'])
    
    t_tot = int(t_tot)
    t_plot = int(t_plot)
    
    vm.shape = (n_trials, t_tot)
    vm_reference.shape = (n_trials, t_tot)
    
    vm_mean = numpy.array([numpy.mean(vm[:, i]) for i in range(t_tot)])
    vm_ref_mean = numpy.array([numpy.mean(vm_reference[:, i])
                               for i in range(t_tot)])
    
    for t in range(n_trials):
        pylab.plot(vm[t][:t_plot], color='gray', lw=0.5)
    pylab.plot(vm_mean[:t_plot], color='black', lw=2.)
    pylab.plot(vm_reference[0][:t_plot], color='red', lw=2.)
    
To display the results, you need to execute
pylab.show()

    
    



