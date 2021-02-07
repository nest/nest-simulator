"""
Illustrate explicit dumping and loading of networks.

Network with E and I populations. E have I_e > 0 to 
achieve random activity. E connections are plastic.

Store V_m and weights.
"""

import nest
import nest.raster_plot
import pickle
import matplotlib.pyplot as plt

{}

[]

class EINetwork:

    def __init__(self):
        self.nI = 500
        self.nE = 4 * self.nI
        self.n = self.nE + self.nI

        self.JE = 5.0
        self.JI = -4 * self.JE
        self.indeg_e = 200
        self.indeg_i = 50

        self.neuron_model = 'iaf_psc_exp'
        nest.CopyModel('stdp_synapse_hom', 'e_syn', {'Wmax': 2*self.JE})
        nest.CopyModel('static_synapse', 'i_syn')

        self.nrn_params = {'I_e': nest.random.normal(400., 50.),
                           'V_m': nest.random.normal(-65., 5.)}

    def build(self):
        self.e_neurons = nest.Create(self.neuron_model, n=self.nE, params=self.nrn_params)
        self.i_neurons = nest.Create(self.neuron_model, n=self.nI)
        self.neurons = self.e_neurons + self.i_neurons
        
        self.sr = nest.Create('spike_recorder')
        
        nest.Connect(self.e_neurons, self.neurons, {'rule': 'fixed_indegree', 'indegree': self.indeg_e},
                     {'synapse_model': 'e_syn', 'weight': self.JE})
        nest.Connect(self.i_neurons, self.neurons, {'rule': 'fixed_indegree', 'indegree': self.indeg_i},
                     {'synapse_model': 'i_syn', 'weight': self.JI})

        nest.Connect(self.neurons, self.sr)

    def dump(self, dump_filename):
        assert nest.NumProcesses() == 1, "Cannot dump MPI parallel"
        
        network = {}

        network['n_vp'] = nest.GetKernelStatus('total_num_virtual_procs')
        network['nrns'] = self.neurons.get(('V_m', 'I_e'), output='pandas')

        network['e_syns'] = nest.GetConnections(synapse_model='e_syn').get(('source', 'target', 'weight'), output='pandas')
        network['i_syns'] = nest.GetConnections(synapse_model='i_syn').get(('source', 'target', 'weight'), output='pandas')

        with open(dump_filename, 'wb') as f:
            pickle.dump(network, f, pickle.HIGHEST_PROTOCOL)

    def load(self, dump_filename):
        assert nest.NumProcesses() == 1, "Cannot load MPI parallel"
         
        with open(dump_filename, 'rb') as f:
            network = pickle.load(f)

        assert network['n_vp'] == nest.GetKernelStatus('total_num_virtual_procs'), 'N_VP must match'
            
        n = len(network['nrns'])

        npar = network['nrns']
        self.neurons = nest.Create(self.neuron_model, n=n, params={'V_m': npar.V_m.values,
                                                                   'I_e': npar.I_e.values})
        self.sr = nest.Create('spike_recorder')

        nest.Connect(network['e_syns'].source.values, network['e_syns'].target.values,
                     'one_to_one',
                     {'synapse_model': 'e_syn', 'weight': network['e_syns'].weight.values})
        nest.Connect(network['i_syns'].source.values, network['i_syns'].target.values,
                     'one_to_one',
                     {'synapse_model': 'i_syn', 'weight': network['i_syns'].weight.values})

        nest.Connect(self.neurons, self.sr)

    def show(self):
        nest.raster_plot.from_device(self.sr, hist=True)
        plt.figure()
        w = nest.GetConnections(synapse_model='e_syn').weight
        plt.hist(w, bins=200)
        plt.pause(1)


if __name__ == '__main__':

    T_sim = 1000

    nest.SetKernelStatus({'local_num_threads': 4})
    ein = EINetwork()

    ein.build()
    nest.Simulate(T_sim)
    print(nest.GetKernelStatus())
    ein.show()

    ein.dump('ein_1000.pkl')
    nest.Simulate(T_sim)
    ein.show()

    nest.ResetKernel()
    nest.SetKernelStatus({'local_num_threads': 4})
    ein2 = EINetwork()
    ein2.load('ein_1000.pkl')
    nest.Simulate(T_sim)
    ein2.show()

    plt.show()
    
