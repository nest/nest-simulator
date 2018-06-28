

    
    
Test of the adapting exponential integrate and fire model in NEST
-----------------------------------------------------------------

This example tests the adaptive integrate and fire model (AdEx) according to
Brette and Gerstner (2005) J. Neurophysiology and
reproduces figure 2.C of the paper.

Note that Brette&Gerstner give the value for b in nA.
To be consistent with the other parameters in the equations, b must be
converted to pA (pico Ampere).

    
    import nest
    import nest.voltage_trace
    import pylab
    
    nest.ResetKernel()
    
First we make sure that the resolution of the simulation is 0.1 ms.
This is important, since the slop of the action potential is very steep.

    
    res = 0.1
    nest.SetKernelStatus({"resolution": res})
    neuron = nest.Create("aeif_cond_alpha")
    
a and b are parameters of the adex model.
Their values come from the publication.

    
    nest.SetStatus(neuron, {"a": 4.0, "b": 80.5})
    
Next we define the stimulus protocol. There are two DC generators,
producing stimulus currents during two time-intervals.

    
    dc = nest.Create("dc_generator", 2)
    
    nest.SetStatus(dc, [{"amplitude": 500.0, "start": 0.0, "stop": 200.0},
                        {"amplitude": 800.0, "start": 500.0, "stop": 1000.0}])
    
We connect the DC generators.

    nest.Connect(dc, neuron, 'all_to_all')
    
And add a voltmeter to record the membrane potentials.

    
    voltmeter = nest.Create("voltmeter")
    
We set the voltmeter to record in small intervals of 0.1 ms and
connect the voltmeter to the neuron.

    nest.SetStatus(voltmeter, {'interval': 0.1, "withgid": True, "withtime": True})
    
    nest.Connect(voltmeter, neuron)
    
Finally, we simulate for 1000 ms and plot a voltage trace
to produce the figure.

    nest.Simulate(1000.0)
    
    nest.voltage_trace.from_device(voltmeter)
    pylab.axis([0, 1000, -80, -20])
    
    



