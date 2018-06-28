

    
    
One neuron example
------------------

This script simulates a neuron driven by a constant external current
and records its membrane potential.

    
    import nest
    import nest.voltage_trace
    nest.set_verbosity("M_WARNING")
    nest.ResetKernel()
    
    neuron = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter")
    
    nest.SetStatus(neuron, "I_e", 376.0)
    nest.SetStatus(voltmeter, [{"withgid": True}])
    
    nest.Connect(voltmeter, neuron)
    
    nest.Simulate(1000.0)
    
    nest.voltage_trace.from_device(voltmeter)
    
    



