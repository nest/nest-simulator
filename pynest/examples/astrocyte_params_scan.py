"""
(This file is only for testing, not for pull request)
This script is for a parameter scan for a single astrocyte.
"""
import os
import json
import hashlib

import pandas as pd
import matplotlib.pyplot as plt

import nest

# Values of parameters to be scanned
# Any astrocyte parameter can be added
params_scan = {
    'tau_IP3': [10., 2.],
    'rate_L': [0.0001, 0.00001],
    'alpha_SIC': [True],
    'tau_SIC': [250]
}

# Helper for recording parameters
class NumpyEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return json.JSONEncoder.default(self, obj)

# Create the parameter sets (as a DataFrame object) to be scanned
def get_arg_df(args):
    assert isinstance(args, dict), "Not a dictionary!"
    df = None
    for i, (k, v) in enumerate(args.items()):
        df_tmp = pd.DataFrame(v, columns=[k])
        if df is None:
            df = df_tmp.copy()
        else:
            df = df.merge(df_tmp, how='cross')
    return df

# Simulate and return recordings
def simulate(time, prate, astro_params):
    nest.ResetKernel()
    astrocyte = nest.Create('astrocyte_lr_1994', params=astro_params)
    mm_astro = nest.Create('multimeter', params={'record_from': ['IP3', 'Ca']})
    ps = nest.Create('poisson_generator', params={'rate': prate})
    neuron = nest.Create('aeif_cond_alpha_astro')
    mm_neuro = nest.Create('multimeter', params={'record_from': ['SIC']})
    nest.Connect(mm_astro, astrocyte)
    nest.Connect(ps, astrocyte)
    nest.Connect(astrocyte, neuron, syn_spec={'synapse_model': 'sic_connection'})
    nest.Connect(mm_neuro, neuron)
    nest.Simulate(time)
    return mm_astro.events, mm_neuro.events

# Main function
def run(params, sim_time, poisson_rate, spath):
    # Create save folder and save this script
    os.system(f'mkdir -p {spath}')
    os.system(f'cp astrocyte_params_scan.py {spath}')

    # Save parameter defaults
    default = nest.GetDefaults('astrocyte_lr_1994')
    out = open(os.path.join(spath, 'astrocyte_params_default.json'), 'w')
    json.dump(default, out, indent=4, cls=NumpyEncoder)
    out.close()

    # Create the parameter sets to be scanned and save it
    df_arg = get_arg_df(params)
    df_arg.to_csv(os.path.join(spath, 'astrocyte_params_scanned.csv'), index=False, float_format='%10.5f')

    # Create plot
    fig, axes = plt.subplots(3, 1, sharex=True, figsize=(10, 10))

    # Run simulations and plot results
    for i, row in df_arg.iterrows():
        label = ''
        astro_params = {}
        for k, v in row.items():
            astro_params[k] = v
            label += f'{k}={v},'
        data_astro, data_neuro = simulate(sim_time, poisson_rate, astro_params)
        ts = data_astro["times"]
        ip3s = data_astro["IP3"]
        cas = data_astro["Ca"]
        sics = data_neuro["SIC"]
        axes[0].plot(ts, ip3s, label=label)
        axes[1].plot(ts, cas, label=label)
        axes[2].plot(ts, sics, label=label)

    # Save plot
    axes[0].set_title(f'Poisson rate = {poisson_rate} Hz')
    axes[0].set_ylabel(r"IP$_{3}$ ($\mu$M)")
    axes[1].set_ylabel(r"Ca$^{2+}$ ($\mu$M)")
    axes[2].set_ylabel("SIC (pA)")
    plt.xlabel('Time (ms)')
    plt.legend(bbox_to_anchor=(1., -0.2), fontsize=10)
    plt.tight_layout()
    plt.savefig(os.path.join(spath, 'astrocyte_params_scan.png'))
    plt.close()


if __name__ == "__main__":
    # Set plot font size
    plt.rcParams.update({'font.size': 14})

    # Set save folder name with hash generator
    spath = hashlib.md5(os.urandom(16)).hexdigest()

    # Run the scan
    run(params_scan, 60000, 100., spath)
