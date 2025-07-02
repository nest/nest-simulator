import matplotlib.pyplot as plt
import nest
import numpy as np

# create neuron population
num_neurons = 100
sir_neurons = nest.Create("sir_neuron", num_neurons)
sir_neurons.beta_sir = 0.01

# connect sir_neurons all-to-all
for i in range(num_neurons):
    for j in range(num_neurons):
        if i != j:
            nest.Connect(sir_neurons[i], sir_neurons[j])

sir_neurons[0].S = 1  # infect zeroth neuron
sir_neurons[1:].h = 1  # set number of infected neighbors of all other neurons to 1

# create recording device
multimeter = nest.Create("multimeter")
multimeter.record_from = ["S", "h"]
nest.Connect(multimeter, sir_neurons)

# simulate dynamics
simulated_time = 1000
nest.Simulate(simulated_time)

# evaluate result
state_list = multimeter.get("events")["S"]
states = np.reshape(state_list, (simulated_time - 1, num_neurons))

num_infected_over_time = np.sum(states == 1, axis=1)
num_susceptible_over_time = np.sum(states == 0, axis=1)
num_recovered_over_time = np.sum(states == 2, axis=1)

plt.plot(range(simulated_time - 1), num_infected_over_time, label="infected", color="red")
plt.plot(range(simulated_time - 1), num_susceptible_over_time, label="susceptible", color="orange")
plt.plot(range(simulated_time - 1), num_recovered_over_time, label="recovered", color="blue")
plt.legend()
plt.xlabel("time")
plt.ylabel("number of neurons in state")
plt.show()
