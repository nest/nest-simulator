# Reset kernel
nest.ResetKernel()

# Create nodes
pg = nest.Create("poisson_generator", params={"rate": 6500.})
neurons = nest.Create("iaf_psc_alpha", 100)
sd = nest.Create("spike_detector")

# Connect nodes
nest.Connect(pg, neurons, syn_spec={"weight": 10.})
nest.Connect(neurons, sd)

# Simulate
nest.Simulate(1000.0)

# Get events
n_events = sd.get("n_events")
