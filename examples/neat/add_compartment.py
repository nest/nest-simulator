import os
import sys
sys.path.insert(0, os.path.expanduser('~/opt/nest-simulator-neat-dend/install/lib/python3.7/site-packages/'))

import nest
import matplotlib.pyplot as plt

nest.ResetKernel()

soma_params = {
    'C_m': 1.0,
    'g_c': 0.1,
    'g_L': 0.1,
    'E_L': -70.0,
}

den_params = {
    'C_m': 0.1,
    'g_c': 0.1,
    'g_L': 0.01,
    'E_L': -70.0,
}

n_neat = nest.Create('iaf_neat')
nest.AddCompartment(1, 0, -1, soma_params)
# nest.AddCompartment(1, 1, 0)
