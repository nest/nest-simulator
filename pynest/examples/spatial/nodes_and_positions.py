import nest
from matplotlib import pyplot as plt

# create a spatial population
s_nodes = nest.Create('iaf_psc_alpha',
                      positions=nest.spatial.grid(shape=[11, 11],
                                                  extent=[11., 11.]))
# connectivity specifications with a mask
conndict = {'rule': 'pairwise_bernoulli', 'p': 1.,
            'mask': {'rectangular': {'lower_left' : [-1.0, -1.0],
                                     'upper_right': [1.0, 1.0]},
                     'anchor': [3. , 3.]}
           }

# get center element
center_neuron = nest.FindCenterElement(s_nodes)

# connect population s_nodes with itself according to the given
# specifications
nest.Connect(s_nodes, s_nodes, conndict)

# Plot target neurons of center neuron
fig = nest.PlotLayer(s_nodes, nodesize=80, nodecolor='coral')
nest.PlotTargets(center_neuron, s_nodes, fig=fig)
plt.title('Target neurons of center neuron')
plt.show()

# Plot source neurons of center neuron
fig = nest.PlotLayer(s_nodes, nodesize=80, nodecolor='coral')
nest.PlotSources(s_nodes, center_neuron, fig=fig)
plt.title('Source neurons of center neuron')
plt.show()

print('Global id of target neurons of center neuron')
print(nest.GetTargetPositions(center_neuron, s_nodes))

print('Global id of source neurons of center neuron')
print(nest.GetSourcePositions(s_nodes, center_neuron))
