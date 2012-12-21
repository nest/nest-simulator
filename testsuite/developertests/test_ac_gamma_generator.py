'''
Created on Feb 7, 2012

@author: thhe
'''
import numpy as np
import matplotlib.pyplot as plt
import nest

def step(t, n, initial, after, seed=1, dt=0.05):
    """Simulates for n generators for t ms. Step at T/2."""

    ## prepare/reset nest
    nest.ResetKernel()
    ## initialize simulation
    #np.random.seed(256 * seed)
    nest.SetStatus([0],[{"resolution": dt}])
    nest.SetStatus([0],[{"grng_seed": 256 * seed + 1}])
    nest.SetStatus([0],[{"rng_seeds": [256 * seed + 2]}])    


    model = 'ac_gamma_generator'

    g = nest.Create(model, n, params=initial)
    sd = nest.Create('spike_detector')
    nest.ConvergentConnect(g, sd)
    nest.Simulate(t/2)
    nest.SetStatus(g, after)
    nest.Simulate(t/2)

    return nest.GetStatus(sd, 'events')[0]

def plot_hist(spikes):
    plt.hist(spikes['times'], bins=np.arange(0.,max(spikes['times'])+1.5,1.), histtype='step')

if __name__ == '__main__':
    t = 1000
    n = 1000
    dt = 1.0
    steps = t/dt
    offset = t/1000.*2*np.pi
    
    plt.ion()
    grid = (2,3)
    fig = plt.figure(figsize=(15,10))

    ## Defaults for everything but dc. Step up
    plt.subplot(grid[0], grid[1], 1)
    spikes = step(t, n,
                  {'dc': 20.0},
                  {'dc': 50.0,},
                  seed=123, dt=dt)
    plot_hist(spikes)
    plt.xlabel('Step up 20->50')
    exp = np.ones(steps)
    
    exp[:steps/2] *= 20
    exp[steps/2:] *= 50
    plt.plot(exp, 'r')

    ## Set all parameters. Step down
    plt.subplot(grid[0], grid[1], 2)
    spikes = step(t, n,
                  {'order': 6.0, 'dc': 80.0, 'ac': 0., 'freq': 0., 'phi': 0.},
                  {'order': 6.0, 'dc': 40.0, 'ac': 0., 'freq': 0., 'phi': 0.},
                  seed=123, dt=dt)
    plot_hist(spikes)
    plt.xlabel('Step down 80->40')
    exp = np.ones(steps)
    exp[:steps/2] *= 80
    exp[steps/2:] *= 40
    plt.plot(exp, 'r')


    ## Set all parameters. ac change
    plt.subplot(grid[0], grid[1], 3)
    spikes = step(t, n,
                  {'order': 3.0, 'dc': 40.0, 'ac': 40., 'freq': 10., 'phi': 0.},
                  {'order': 3.0, 'dc': 40.0, 'ac': 20., 'freq': 10., 'phi': 0.},
                  seed=123, dt=dt)
    plot_hist(spikes)
    plt.xlabel('AC 40->20')
    exp = np.zeros(steps)
    exp[:steps/2] = 40. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2))) + 40.
    exp[steps/2:] = 20. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2)) + offset) + 40.
    plt.plot(exp, 'r')

    ## dc change with non-zero ac
    plt.subplot(grid[0], grid[1], 4)
    spikes = step(t, n,
                  {'order': 6.0, 'dc': 20.0, 'ac': 20., 'freq': 10., 'phi': 0.},
                  {'order': 6.0, 'dc': 50.0, 'ac': 50., 'freq': 10., 'phi': 0.},
                  seed=123, dt=dt)
    plot_hist(spikes)
    plt.xlabel('AC/DC 20->50')
    exp = np.zeros(steps)
    exp[:steps/2] = 20. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2))) + 20.
    exp[steps/2:] = 50. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2)) + offset) + 50.
    plt.plot(exp, 'r')

    
    ## Mostly defaults. ac up
    plt.subplot(grid[0], grid[1], 5)
    spikes = step(t, n,
                  {'dc': 40.0,},
                  {'ac': 40.0, 'freq': 20.},
                  seed=123, dt=1.)
    plot_hist(spikes)
    plt.xlabel('Step up 20->50')
    exp = np.zeros(steps)
    exp[:steps/2] = np.ones(steps/2) * 40.
    exp[steps/2:] = 40. * np.sin(np.arange(0, t/1000.*np.pi*20, t/1000.*np.pi*20./(steps/2))) + 40.
    plt.plot(exp, 'r')

    
    #Phase shift    
    plt.subplot(grid[0], grid[1], 6)
    spikes = step(t, n,
                  {'order': 6.0, 'dc': 60.0, 'ac': 60., 'freq': 10., 'phi': 0.},
                  {'order': 6.0, 'dc': 60.0, 'ac': 60., 'freq': 10., 'phi': np.pi},
                  seed=123, dt=1.)
    plot_hist(spikes)
    plt.xlabel('Phi 0->3.14')
    exp = np.zeros(steps)
    exp[:steps/2] = 60. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2))) + 60.
    exp[steps/2:] = 60. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2)) + offset + np.pi)  + 60.
    plt.plot(exp, 'r')
    
