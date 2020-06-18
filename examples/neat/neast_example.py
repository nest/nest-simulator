import nest
import matplotlib.pyplot as plt

from neat import PhysTree, CompartmentFitter, NeuronSimTree
from neat import createReducedNestModel, createReducedNeuronModel

from datarep import paths


nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=.1))


spike_train = [50., 55., 70., 78.]


phys_tree = PhysTree(paths.morph_path + 'N19ttwt.CNG.swc')

phys_tree.setPhysiology(1., 100./1e6)
phys_tree.fitLeakCurrent(-70., 10.)
phys_tree.setCompTree()
phys_tree.treetype = 'computational'

cfit = CompartmentFitter(phys_tree)

loc = (122, 1.)
locs = [(1,.5), loc]

ctree = cfit.fitModel(locs)
# ctree[0].currents['L'][0] = 0.01
# ctree[1].currents['L'][0] = 0.001
# ctree[1].g_c = 0.01
# ctree[0].ca = 0.0001
# ctree[1].ca = 0.00001
n_neat = createReducedNestModel(ctree)
n_neuron = createReducedNeuronModel(ctree)

print(ctree)

clocs = ctree.getEquivalentLocs()
n_neuron.initModel(dt=.1, t_calibrate=100.)

n_neuron.storeLocs(clocs, name='rec locs')
n_neuron.addDoubleExpSynapse(clocs[1], .2, 3., 0.)
n_neuron.setSpikeTrain(0, 0.001, spike_train)
# n_neuron.addIClamp(clocs[1], 0.001, 0., 150.)
res = n_neuron.run(150.)


# n_full = phys_tree.__copy__(new_tree=NeuronSimTree())
# n_full.treetype = 'computational'
# n_full.initModel(dt=.1, t_calibrate=100., factor_lambda=5.)

# n_full.storeLocs(locs, name='rec locs')
# n_full.addDoubleExpSynapse(locs[1], .2, 3., 0.)
# n_full.setSpikeTrain(0, 0.001, spike_train)
# res_full = n_full.run(150.)

sg = nest.Create('spike_generator', 1, {'spike_times': spike_train})
syn_idx_AMPA = nest.AddReceptor(n_neat, 1, "AMPA")
nest.Connect(sg, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .001, 'delay': 0.2, 'receptor_type': syn_idx_AMPA})


m_neat = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1'], 'interval': .1})
nest.Connect(m_neat, n_neat)

nest.Simulate(150.)

events_neat = nest.GetStatus(m_neat, 'events')[0]

print('v0 = %.5f, v1 = %.5f'%(events_neat['V_m_0'][-1], events_neat['V_m_1'][-1]))

# # attenuation analytical
# a_ = ctree[1].g_c / (ctree[0].currents['L'][0] + ctree[1].g_c)
# print('att analytical =', a_)
# # attenuation neuron sim
# print('att neuron sim =', (res['v_m'][0,-1] - res['v_m'][0,0]) / (res['v_m'][1,-1] - res['v_m'][1,0]))
# # attenuation nest sim
# print('att nest sim   =', (events_neat['V_m_0'][-1] - (-70.)) / (events_neat['V_m_1'][-1] - (-70.)))

# print('')
# # input impedance analytical
# z_ = 1. / (ctree[1].currents['L'][0] + ctree[1].g_c * ctree[0].currents['L'][0] / (ctree[0].currents['L'][0] + ctree[1].g_c))
# print('imp analytical =', z_)
# # input impedance neuron sim
# print('imp neuron sim =', (res['v_m'][1,-1] - res['v_m'][1,0]) / 0.001)
# # input impedance nest sim
# print('imp nest sim   =', (events_neat['V_m_1'][-1] - events_neat['V_m_1'][0]) / 0.001)


plt.plot(res['t'], res['v_m'][0], c='DarkGrey', label='V_NEURON_0')
plt.plot(res['t'], res['v_m'][1], c='DarkGrey', label='V_NEURON_1')
# plt.plot(res_full['t'], res_full['v_m'][0], 'k:', label='V_m_0')
# plt.plot(res_full['t'], res_full['v_m'][1], 'k:', label='V_m_1')
plt.plot(events_neat['times'], events_neat['V_m_0'], 'b--', label='V_NEST_0')
plt.plot(events_neat['times'], events_neat['V_m_1'], 'r--', label='V_NEST_1')
plt.legend()

plt.show()

