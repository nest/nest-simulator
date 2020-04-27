import matplotlib.pyplot as plt
import nest

nest.ResetKernel()

sg = nest.Create('spike_generator', 1, {'spike_times': [1., 5., 50., 500.]})
n = nest.Create('iaf_psc_delta')
n_neat = nest.Create('iaf_neat')
m = nest.Create('multimeter', 1, {'record_from': ['V_m']})
m_neat = nest.Create('multimeter', 1, {'record_from': ['V_m']})

nest.Connect(sg, n)
nest.Connect(sg, n_neat)

nest.Connect(m, n)
nest.Connect(m_neat, n_neat)

nest.Simulate(1000)

events = nest.GetStatus(m, 'events')[0]
events_neat = nest.GetStatus(m_neat, 'events')[0]

plt.plot(events['times'], events['V_m'], label='iaf_psc_delta')
plt.plot(events_neat['times'], events_neat['V_m'], label='iaf_neat')
plt.legend()
plt.savefig('simple_psp_recording.pdf')
