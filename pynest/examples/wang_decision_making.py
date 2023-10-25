import nest
import matplotlib.pyplot as plt
import numpy as np

np.random.seed(123)
rng = np.random.default_rng()

epop_params = {"g_AMPA_ext": 2.1,
               "g_AMPA_rec": 0.05,
               "g_NMDA": 0.165,
               "g_GABA": 1.3,
               "tau_GABA": 5.0,
               "tau_AMPA": 2.0,
               "tau_decay_NMDA": 100.0,
               "alpha": 0.5,
               "conc_Mg2": 1.0,
               "g_L": 25.               # leak conductance
               "E_L": -70.0,            # leak reversal potential
               "E_ex": 0.0,             # excitatory reversal potential
               "E_in": -70.0,           # inhibitory reversal potential
               "V_reset": -55.0,        # reset potential
               "C_m": 500.0,            # membrane capacitance
               "t_ref": 2.0             # refreactory period
               }


ipop_params = {"g_AMPA_ext": 1.62,
               "g_AMPA_rec": 0.04,
               "g_NMDA": 0.13,
               "g_GABA": 1.0,
               "tau_GABA": 5.0,
               "tau_AMPA": 2.0,
               "tau_decay_NMDA": 100.0,
               "alpha": 0.5,
               "conc_Mg2": 1.0,
               "g_L": 20.               # leak conductance
               "E_L": -70.0,            # leak reversal potential
               "E_ex": 0.0,             # excitatory reversal potential
               "E_in": -70.0,           # inhibitory reversal potential
               "V_reset": -55.0,        # reset potential
               "C_m": 200.0,            # membrane capacitance
               "t_ref": 1.0             # refreactory period
               }

NE = 1600
NI = 400



epop1 = nest.Create("iaf_wang_2002", int(0.5 * NE))
epop2 = nest.Create("iaf_wang_2002", int(0.5 * NE))
ipop = nest.Create("iaf_wang_2002", NE)






