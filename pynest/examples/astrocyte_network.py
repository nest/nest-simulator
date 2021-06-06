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
    connectivity_n2n, connectivity_a2n = 0.2, 0.001
    # TODO: Weight is set high to allow a small network to spike. What is realistic?
    synapse_weight, sic_weight = 10.0, 2.11
    fraction_glu_to_astro, fraction_glu_to_postsyn = 0.3, 0.7
    neuron_model = "aeif_cond_alpha_astro"
    astrocyte_model = "astrocyte"
    synapse_model = "tsodyks_synapse"
    astro_out_connection = "sic_connection"
    # Input rate to excitatory population that is connected to astrocytes
    # TODO: Is it too large? Some examples show 80 000
    poisson_rate = 20000.0 # Hz
    simulation_duration = 10000 # ms
    one_to_many = True

    # connection specs
    syn_spec_n2n = {
        "synapse_model": synapse_model,
        "weight": synapse_weight * fraction_glu_to_postsyn
    }
    syn_spec_a2n = {
        "synapse_model": astro_out_connection,
        "weight": sic_weight
    }

    # create populations
    # TODO: we assume same number of neurons and astrocytes in this network. Make flexible?
    # (an astrocyte compartment per neuron, and sic to one neuron)
    neuronpop = nest.Create(neuron_model, n_exc_neurons)
    astropop = nest.Create(astrocyte_model, n_exc_neurons)
    print(neuronpop)
    print(astropop)

    # connect neurons
    nest.Connect(
        neuronpop, neuronpop,
        syn_spec=syn_spec_n2n,
        conn_spec={
            "rule": "pairwise_bernoulli",
            "p": connectivity_n2n,
            "allow_autapses": False
        }
    )

    # connect astrocyte->neuron
    nest.Connect(
        astropop, neuronpop,
        syn_spec=syn_spec_a2n,
        conn_spec={'rule': 'one_to_one'}
    )
    if one_to_many: # multiple astrocyte->neuron connections
        nest.Connect(
            astropop, neuronpop,
            syn_spec=syn_spec_a2n,
            conn_spec={'rule': 'pairwise_bernoulli', 'p': connectivity_a2n}
        )

    # connect neuron->astrocyte
    conns = nest.GetConnections(neuronpop, neuronpop)   # read the neuron-neuron connections
    neuron_list = neuronpop.tolist()    # read the neuron ids
    for i, conn in enumerate(conns): # connect according to neuron-neuron connections
        nest.Connect(
            [conn.source], astropop[neuron_list.index(conn.target)],
            syn_spec={
                "synapse_model": synapse_model,
                "weight": synapse_weight * fraction_glu_to_astro,
                }
        )

    # verify connections
    conns_n2n = nest.GetConnections(neuronpop, neuronpop)
    conns_n2a = nest.GetConnections(neuronpop, astropop)
    conns_arr = np.array([
        conns_n2n.source, conns_n2n.target,
        conns_n2a.source, conns_n2a.target]).T
    np.savetxt('conns.csv', conns_arr, fmt='%d', delimiter=',',
        header='pre,post,pre,astro', comments='')

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
