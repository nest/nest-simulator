

    
    
Two neurons
-----------


    
    import pylab
    
    import nest
    import nest.voltage_trace
    
    weight = 20.0
    delay = 1.0
    stim = 1000.0
    
    neuron1 = nest.Create("iaf_psc_alpha")
    neuron2 = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter")
    
    nest.SetStatus(neuron1, {"I_e": stim})
    nest.Connect(neuron1, neuron2, syn_spec={'weight': weight, 'delay': delay})
    nest.Connect(voltmeter, neuron2)
    
    nest.Simulate(100.0)
    
    nest.voltage_trace.from_device(voltmeter)
    nest.voltage_trace.show()
    
    



