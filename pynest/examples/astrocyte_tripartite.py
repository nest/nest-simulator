import os
import json
import hashlib

import matplotlib.pyplot as plt

import nest

# Simulation parameters
sim_time = 60000
poisson_rate_neuro = 1500.0
poisson_rate_astro = 0.0

# Neuron parameters
params_neuro = {
    'tau_syn_ex': 2.0,
}

# Astrocyte parameters
params_astro = {
    'IP3_0': 0.16,
}

# Connection parameters
w_pre2astro = 1.0
w_pre2post = 1.0
w_astro2post = 1.0

# Simulate and return recordings
def simulate():
    nest.ResetKernel()
    # Create astrocyte and its devices
    astrocyte = nest.Create('astrocyte_lr_1994', params=params_astro)
    ps_astro = nest.Create('poisson_generator', params={'rate': poisson_rate_astro})
    mm_astro = nest.Create('multimeter', params={'record_from': ['IP3', 'Ca']})
    nest.Connect(ps_astro, astrocyte)
    nest.Connect(mm_astro, astrocyte)
    # Create neurons and their devices
    pre_neuron = nest.Create('aeif_cond_alpha_astro', params=params_neuro)
    post_neuron = nest.Create('aeif_cond_alpha_astro', params=params_neuro)
    ps_pre = nest.Create('poisson_generator', params={'rate': poisson_rate_neuro})
    mm_pre = nest.Create('multimeter', params={'record_from': ['V_m']})
    mm_post = nest.Create('multimeter', params={'record_from': ['SIC']})
    nest.Connect(ps_pre, pre_neuron)
    nest.Connect(mm_pre, pre_neuron)
    nest.Connect(mm_post, post_neuron)
    # Create tripartite connectivity
    nest.Connect(pre_neuron, post_neuron, syn_spec={'weight': w_pre2post})
    nest.Connect(pre_neuron, astrocyte, syn_spec={'weight': w_pre2astro})
    nest.Connect(astrocyte, post_neuron, syn_spec={'synapse_model': 'sic_connection', 'weight': w_astro2post})
    # Simulate
    nest.Simulate(sim_time)
    return mm_astro.events, mm_pre.events, mm_post.events

# Main function
def run(spath):
    # Create save folder and save this script
    os.system(f'mkdir -p {spath}')
    os.system(f'cp astrocyte_tripartite.py {spath}')

    # Save parameter defaults
    default = nest.GetDefaults('astrocyte_lr_1994')
    default.update(params_astro)
    out = open(os.path.join(spath, 'astrocyte_params.json'), 'w')
    json.dump(default, out, indent=4)
    out.close()

    # Create plot
    fig, axes = plt.subplots(2, 2, sharex=True, figsize=(10, 8))

    # Run simulation and plot results
    data_astro, data_pre, data_post = simulate()
    ts = data_astro["times"]
    ip3s = data_astro["IP3"]
    cas = data_astro["Ca"]
    vms = data_pre['V_m']
    sics = data_post["SIC"]
    axes[0, 0].plot(ts, vms)
    axes[1, 0].plot(ts, sics)
    axes[0, 1].plot(ts, ip3s)
    axes[1, 1].plot(ts, cas)

    # Set and save plot
    axes[0, 0].set_title(f'Presynaptic neuron\n(Poisson rate = {poisson_rate_neuro} Hz)')
    axes[0, 0].set_ylabel(r"V$_{m}$ (mV)")
    axes[1, 0].set_title(f'Postsynaptic neuron')
    axes[1, 0].set_ylabel("SIC (pA)")
    axes[1, 0].set_xlabel('Time (ms)')
    axes[0, 1].set_title(f'Astrocyte\n(Poisson rate = {poisson_rate_astro} Hz)')
    axes[0, 1].set_ylabel(r"IP$_{3}$ ($\mu$M)")
    axes[1, 1].set_ylabel(r"Ca$^{2+}$ ($\mu$M)")
    axes[1, 1].set_xlabel('Time (ms)')
    plt.tight_layout()
    plt.savefig(os.path.join(spath, 'astrocyte_tripartite.png'))
    plt.show()
    plt.close()


if __name__ == "__main__":
    # Set plot font size
    plt.rcParams.update({'font.size': 14})

    # Set save folder name with hash generator
    spath = os.path.join('astrocyte_tripartite', hashlib.md5(os.urandom(16)).hexdigest())

    # Run main function
    run(spath)
