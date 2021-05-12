"""
Network of cells with astrocytes
"""

import numpy as np
import matplotlib.pyplot as plt
import nest
import nest.raster_plot


def main():
    """
    :returns: TODO

    """

    n_exc_neurons = 100
    connectivity = 0.2
    # TODO: Weight is set high to allow a small network to spike. What is realistic?
    synapse_weight = 10.0
    sic_weight = 2.11
    fraction_glu_to_astro = 0.3
    fraction_glu_to_postsyn = 0.7
    neuron_model = "aeif_cond_alpha_astro"
    astrocyte_model = "astrocyte"
    synapse_model = "tsodyks_synapse"
    astro_out_connection = "sic_connection"
    # Input rate to excitatory population that is connected to astrocytes
    # TODO: Is it too large? Some examples show 80 000
    poisson_rate = 20000.0 # Hz
    simulation_duration = 10000 # ms

    # Create a connectivity matrix, could be randomly created but
    # use a simple one for now
    # connections = np.array([[0, 0], [1, 0]])
    connections = np.random.rand(n_exc_neurons, n_exc_neurons) < connectivity
    # convert from booleans
    connections = connections.astype(np.int64)
    # don't want neurons connecting to itself
    np.fill_diagonal(connections, 0)

    # create populations
    # TODO: we assume same number of neurons and astrocytes in this network. Make flexible?
    # (an astrocyte compartment per neuron, and sic to one neuron)
    neuronpop = nest.Create(neuron_model, n_exc_neurons)
    astropop = nest.Create(astrocyte_model, n_exc_neurons)
    print(neuronpop)
    print(astropop)

    # Connect cells so that a row of the connection matrix
    # indicates where this neuron receives connections.
    # Connect the astrocyte population accordingly.
    for i_row in range(connections.shape[0]):
        pre_syn_ids = np.flatnonzero(connections[i_row, :])
        for i in pre_syn_ids:
            # Connect neurons
            nest.Connect(
                neuronpop[i], # pre
                neuronpop[i_row], # post stays constant in this loop
                syn_spec={
                    "synapse_model": synapse_model,
                    "weight": synapse_weight * fraction_glu_to_postsyn,
                },
            )
            # Connect the astrocyte to the same presynaptic neuron
            nest.Connect(
                neuronpop[i],
                astropop[i_row],
                syn_spec={
                    "synapse_model": synapse_model,
                    "weight": synapse_weight * fraction_glu_to_astro,
                },
            )
        # Connect also the SIC
        # TODO: this works as long as there is one astro per neuron
        nest.Connect(
            astropop[i_row],
            neuronpop[i_row], # post synaptic neuron is same as above
            syn_spec={"synapse_model": astro_out_connection, "weight": sic_weight},
        )

    # Add input to neurons and recording devices
    input_gen = nest.Create("poisson_generator", params={"rate": poisson_rate})
    nest.Connect(input_gen, neuronpop)
    spikedetector = nest.Create("spike_recorder")
    nest.Connect(neuronpop, spikedetector)
    # curr_gen = nest.Create("dc_generator", params={"start": 10.0, "stop": 1000.0, "amplitude": 800.0})
    # nest.Connect(curr_gen, neuronpop)
    # Recording for astrocyte
    astro_meter = nest.Create("multimeter", params={"record_from": ["IP3_astro", "Ca_astro"]})
    nest.Connect(astro_meter, astropop)

    # Simulate!
    nest.Simulate(simulation_duration)

    # ..and plot
    nest.raster_plot.from_device(
        spikedetector,
        hist=True,
        hist_binwidth=10.0,
        title="Repeated stimulation by Poisson generator",
    )
    astrodata = nest.GetStatus(astro_meter)[0]['events']
    plt.figure()
    plt.subplot(2, 1, 1)
    for i in range(n_exc_neurons):
        plt.plot(astrodata['times'][i::n_exc_neurons], astrodata['IP3_astro'][i::n_exc_neurons])
    plt.title('IP3')
    plt.subplot(2, 1, 2)
    for i in range(n_exc_neurons):
        plt.plot(astrodata['times'][i::n_exc_neurons], astrodata['Ca_astro'][i::n_exc_neurons])
    plt.title('Ca')
    plt.show()


if __name__ == "__main__":
    main()
