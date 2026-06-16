# -*- coding: utf-8 -*-
#
# extract_reference_data.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.
"""
Extract reference data for documentation builds
------------------------------------------------

This script extracts and consolidates reference data from simulation outputs
into a format suitable for NEST-independent figure generation.

Run this script AFTER running the full Snakemake workflow::

    snakemake -c4
    python extract_reference_data.py

This creates a ``reference_data/`` directory containing:

- ``iaf_psc_alpha/``
    - ``parameters.json`` - Model parameters
    - ``spikes.dat`` - Spike data
    - ``connectivity_presim.dat`` - Connectivity before simulation
    - ``connectivity_postsim.dat`` - Connectivity after simulation

- ``ignore_and_fire/``
    - (same structure)

- ``scaling_results.json`` - Pre-computed scaling experiment data

The reference_data/ directory should be committed to the repository so that
documentation builds can generate figures without running NEST simulations.
"""

import json
import os
import shutil
from pathlib import Path

import numpy as np

SCRIPT_DIR = Path(__file__).parent
DATA_DIR = SCRIPT_DIR / "data"
REFERENCE_DATA_DIR = SCRIPT_DIR / "reference_data"

NEURON_MODELS = ["iaf_psc_alpha", "ignore_and_fire"]
NETWORK_SIZES = list(range(1250, 15000, 1250))


def extract_model_reference_data(neuron_model):
    """
    Extract reference data for a single neuron model.

    Parameters
    ----------
    neuron_model : str
        Name of the neuron model
    """
    print(f"\nExtracting reference data for {neuron_model}...")

    source_dir = DATA_DIR / neuron_model
    dest_dir = REFERENCE_DATA_DIR / neuron_model
    dest_dir.mkdir(parents=True, exist_ok=True)

    # Copy parameters
    param_file = source_dir / "model_instance_parameters.json"
    if param_file.exists():
        shutil.copy(param_file, dest_dir / "parameters.json")
        print(f"  Copied parameters.json")
    else:
        print(f"  WARNING: {param_file} not found")

    # Find and copy spike data
    spike_files = list(source_dir.glob("spikes-*.dat"))
    if spike_files:
        # Use the first spike file found
        shutil.copy(spike_files[0], dest_dir / "spikes.dat")
        print(f"  Copied spikes.dat")
    else:
        print(f"  WARNING: No spike files found in {source_dir}")

    # Copy connectivity data
    for label in ["presim", "postsim"]:
        conn_file = source_dir / f"connectivity_{label}.dat"
        if conn_file.exists():
            shutil.copy(conn_file, dest_dir / f"connectivity_{label}.dat")
            print(f"  Copied connectivity_{label}.dat")
        else:
            print(f"  WARNING: {conn_file} not found")


def extract_scaling_data():
    """
    Extract and consolidate scaling experiment results.
    """
    print("\nExtracting scaling experiment data...")

    scaling_data = {
        "neuron_models": NEURON_MODELS,
        "network_sizes": NETWORK_SIZES,
        "sim_time": [],
        "rate": [],
        "weight_mean": [],
        "weight_sd": [],
        "weight_min": [],
        "weight_max": [],
        "weight_median": [],
    }

    for neuron_model in NEURON_MODELS:
        model_sim_time = []
        model_rate = []
        model_weight_mean = []
        model_weight_sd = []
        model_weight_min = []
        model_weight_max = []
        model_weight_median = []

        for N in NETWORK_SIZES:
            data_file = DATA_DIR / f"N{N}" / neuron_model / "data.json"

            if data_file.exists():
                with open(data_file) as f:
                    data = json.load(f)

                model_sim_time.append(data.get("sim_time", np.nan))
                model_rate.append(data.get("rate", np.nan))
                model_weight_mean.append(data.get("weight_mean", np.nan))
                model_weight_sd.append(data.get("weight_sd", np.nan))
                model_weight_min.append(data.get("weight_min", np.nan))
                model_weight_max.append(data.get("weight_max", np.nan))
                model_weight_median.append(data.get("weight_median", np.nan))
            else:
                print(f"  WARNING: {data_file} not found")
                model_sim_time.append(np.nan)
                model_rate.append(np.nan)
                model_weight_mean.append(np.nan)
                model_weight_sd.append(np.nan)
                model_weight_min.append(np.nan)
                model_weight_max.append(np.nan)
                model_weight_median.append(np.nan)

        scaling_data["sim_time"].append(model_sim_time)
        scaling_data["rate"].append(model_rate)
        scaling_data["weight_mean"].append(model_weight_mean)
        scaling_data["weight_sd"].append(model_weight_sd)
        scaling_data["weight_min"].append(model_weight_min)
        scaling_data["weight_max"].append(model_weight_max)
        scaling_data["weight_median"].append(model_weight_median)

    # Save consolidated scaling data
    scaling_file = REFERENCE_DATA_DIR / "scaling_results.json"
    with open(scaling_file, "w") as f:
        json.dump(scaling_data, f, indent=2)
    print(f"  Saved scaling_results.json")


def main():
    """
    Main entry point for reference data extraction.
    """
    print("Extracting reference data for documentation builds")
    print("=" * 50)

    # Check if data directory exists
    if not DATA_DIR.exists():
        print(f"\nERROR: Data directory not found: {DATA_DIR}")
        print("\nRun the Snakemake workflow first:")
        print("    snakemake -c4")
        return 1

    # Create reference data directory
    REFERENCE_DATA_DIR.mkdir(exist_ok=True)

    # Extract data for each neuron model
    for neuron_model in NEURON_MODELS:
        source_dir = DATA_DIR / neuron_model
        if source_dir.exists():
            extract_model_reference_data(neuron_model)
        else:
            print(f"\nWARNING: No data for {neuron_model}")

    # Extract scaling data
    extract_scaling_data()

    print("\n" + "=" * 50)
    print("Reference data extraction complete!")
    print(f"\nReference data saved to: {REFERENCE_DATA_DIR}")
    print("\nNext steps:")
    print("  1. Review the extracted data")
    print("  2. Test figure generation: python generate_figures_for_docs.py")
    print("  3. Commit reference_data/ to the repository")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
