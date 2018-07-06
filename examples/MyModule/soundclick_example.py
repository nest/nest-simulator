import nest
nest.Install("mymodule")

population = nest.Create("izhikevich", 1)
spike_detectors = nest.Create("spike_detector", 1)
nest.SetStatus(spike_detectors, {"record_to": ["soundclick"]})
nest.Connect(population, spike_detectors)

# regular spiking
nest.SetStatus(population, {"a": 0.02,
                            "b": 0.2,
                            "c": -65.0,
                            "d": 8.0,
                            "U_m": 0.0,
                            "V_m": -75.0,
                            "I_e": 6.0})
nest.Simulate(4000)
nest.SetStatus(population, {"I_e": 0.0})
nest.Simulate(500)

# fast spiking
nest.SetStatus(population, {"a": 0.1,
                            "b": 0.2,
                            "c": -65.0,
                            "d": 2.0,
                            "U_m": 0.0,
                            "V_m": -75.0,
                            "I_e": 6.0})
nest.Simulate(4000)
nest.SetStatus(population, {"I_e": 0.0})
nest.Simulate(500)

# chattering
nest.SetStatus(population, {"a": 0.02,
                            "b": 0.2,
                            "c": -50.0,
                            "d": 2.0,
                            "U_m": 0.0,
                            "V_m": -75.0,
                            "I_e": 6.0})
nest.Simulate(4000)
