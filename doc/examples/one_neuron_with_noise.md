

    
    
One neuron with noise
---------------------


    
    import nest
    import nest.voltage_trace
    
    nest.ResetKernel()
    
    neuron = nest.Create("iaf_psc_alpha")
    
    noise = nest.Create("poisson_generator", 2)
    nest.SetStatus(noise, [{"rate": 80000.0}, {"rate": 15000.0}])
    
    voltmeter = nest.Create("voltmeter")
    nest.SetStatus(voltmeter, {"withgid": True, "withtime": True})
    
    nest.Connect(noise, neuron, syn_spec={'weight': [[1.2, -1.0]], 'delay': 1.0})
    nest.Connect(voltmeter, neuron)
    
    nest.Simulate(1000.0)
    
    nest.voltage_trace.from_device(voltmeter)
    
    



