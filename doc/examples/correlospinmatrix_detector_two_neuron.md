

    
    
correlospinmatrix_detector example
-----------------------------------

This scripts simulates two connected binary neurons, similar
as in Ginzburg & Sompolinsky (1994) PRE, 50(4) p. 3771. Fig. 1.
It measures and plots the auto- and cross covariance funcitons
of the individual neurons and between them, repsectively.


    
    import pylab as pl
    import nest
    import numpy as np
    
    m_x = 0.5
    tau_m = 10.
    h = 0.1
    T = 1000000.
    tau_max = 100.
    
    csd = nest.Create("correlospinmatrix_detector")
    nest.SetStatus(csd, {"N_channels": 2, "tau_max": tau_max, "Tstart": tau_max,
                         "delta_tau": h})
    
    nest.SetDefaults('ginzburg_neuron', {'theta': 0.0, 'tau_m': tau_m,
                                         'c_1': 0.0, 'c_2': 2. * m_x, 'c_3': 1.0})
    n1 = nest.Create("ginzburg_neuron")
    
    nest.SetDefaults("mcculloch_pitts_neuron", {'theta': 0.5, 'tau_m': tau_m})
    n2 = nest.Create("mcculloch_pitts_neuron")
    
    nest.Connect(n1, n2, syn_spec={"weight": 1.0})
    
    nest.Connect(n1, csd, syn_spec={"receptor_type": 0})
    nest.Connect(n2, csd, syn_spec={"receptor_type": 1})
    
    nest.Simulate(T)
    
    stat = nest.GetStatus(csd)[0]
    
    c = stat["count_covariance"]
    
    m = np.zeros(2, dtype=float)
    for i in range(2):
        m[i] = c[i][i][int(tau_max / h)] * (h / T)
    
    print('mean activities =', m)
    
    cmat = np.zeros((2, 2, int(2 * tau_max / h) + 1), dtype=float)
    for i in range(2):
        for j in range(2):
            cmat[i, j] = c[i][j] * (h / T) - m[i] * m[j]
    
    ts = np.arange(-tau_max, tau_max + h, h)
    
    pl.title("auto- and cross covariance functions")
    
    pl.plot(ts, cmat[0, 1], 'r', label=r"$c_{12}$")
    pl.plot(ts, cmat[1, 0], 'b', label=r"$c_{21}$")
    pl.plot(ts, cmat[0, 0], 'g', label=r"$c_{11}$")
    pl.plot(ts, cmat[1, 1], 'y', label=r"$c_{22}$")
    pl.xlabel("time $t \; \mathrm{ms}$")
    pl.ylabel(r"$c$")
    pl.legend()
    
    pl.show()
    
    



