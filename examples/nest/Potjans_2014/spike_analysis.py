# -*- coding: utf-8 -*-
#
# spike_analysis.py
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

# Merges spike files, produces raster plots, calculates and plots firing rates

import numpy as np
import glob
import matplotlib.pyplot as plt
import os

datapath = '../data'

# get simulation time and numbers of neurons recorded from sim_params.sli

f = open(os.path.join(datapath, 'sim_params.sli'), 'r')
for line in f:
    if 't_sim' in line:
        T = float(line.split()[1])
    if '/record_fraction_neurons_spikes' in line:
        record_frac = line.split()[1]
f.close()

f = open(os.path.join(datapath, 'sim_params.sli'), 'r')
for line in f:
    if record_frac == 'true':
        if 'frac_rec_spikes' in line:
            frac_rec = float(line.split()[1])
    else:
        if 'n_rec_spikes' in line:
            n_rec = int(line.split()[1])
f.close()

T_start = 200.  # starting point of analysis (to avoid transients)

# load node IDs

node_idfile = open(os.path.join(datapath, 'population_nodeids.dat'), 'r')
node_ids = []
for l in node_idfile:
    a = l.split()
    node_ids.append([int(a[0]), int(a[1])])
print('Global IDs:')
print(node_ids)
print()

# number of populations

num_pops = len(node_ids)
print('Number of populations:')
print(num_pops)
print()

# first node ID in each population

raw_first_node_ids = [node_ids[i][0] for i in np.arange(len(node_ids))]

# population sizes

pop_sizes = [node_ids[i][1] - node_ids[i][0] + 1 for i in np.arange(len(node_ids))]

# numbers of neurons for which spikes were recorded

if record_frac == 'true':
    rec_sizes = [int(pop_sizes[i] * frac_rec)
                 for i in xrange(len(pop_sizes))]
else:
    rec_sizes = [n_rec] * len(pop_sizes)

# first node ID of each population once device node IDs are dropped

first_node_ids = [int(1 + np.sum(pop_sizes[:i]))
                  for i in np.arange(len(pop_sizes))]

# last node ID of each population once device node IDs are dropped

last_node_ids = [int(np.sum(pop_sizes[:i + 1]))
                 for i in np.arange(len(pop_sizes))]

# convert lists to a nicer format, i.e. [[2/3e, 2/3i], []....]

Pop_sizes = [pop_sizes[i:i + 2]
             for i in xrange(0, len(pop_sizes), 2)]
print('Population sizes:')
print(Pop_sizes)
print()

Raw_first_node_ids = [raw_first_node_ids[i:i + 2] for i in
                      xrange(0, len(raw_first_node_ids), 2)]

First_node_ids = [first_node_ids[i:i + 2] for i in xrange(0, len(first_node_ids), 2)]

Last_node_ids = [last_node_ids[i:i + 2] for i in xrange(0, len(last_node_ids), 2)]

# total number of neurons in the simulation

num_neurons = last_node_ids[len(last_node_ids) - 1]
print('Total number of neurons:')
print(num_neurons)
print()

# load spikes from gdf files, correct node IDs and merge them in population files,
# and store spike trains

# will contain neuron id resolved spike trains
neuron_spikes = [[] for i in np.arange(num_neurons + 1)]
# container for population-resolved spike data
spike_data = [[[], []], [[], []], [[], []], [[], []], [[], []], [[], []],
              [[], []], [[], []]]

counter = 0

for layer in ['0', '1', '2', '3']:
    for population in ['0', '1']:
        output = os.path.join(datapath,
                              'population_spikes-{}-{}.gdf'.format(layer,
                                                                   population))
        file_pattern = os.path.join(datapath,
                                    'spikes_{}_{}*'.format(layer, population))
        files = glob.glob(file_pattern)
        print('Merge ' + str(
            len(files)) + ' spike files from L' + layer + 'P' + population)
        if files:
            merged_file = open(output, 'w')
            for f in files:
                data = open(f, 'r')
                for l in data:
                    a = l.split()
                    a[0] = int(a[0])
                    a[1] = float(a[1])
                    raw_first_node_id = Raw_first_node_ids[int(layer)][int(population)]
                    first_node_id = First_node_ids[int(layer)][int(population)]
                    a[0] = a[0] - raw_first_node_id + first_node_id

                    if (a[1] > T_start):  # discard data in the start-up phase
                        spike_data[counter][0].append(num_neurons - a[0])
                        spike_data[counter][1].append(a[1] - T_start)
                        neuron_spikes[a[0]].append(a[1] - T_start)

                    converted_line = str(a[0]) + '\t' + str(a[1]) + '\n'
                    merged_file.write(converted_line)
                data.close()
            merged_file.close()
            counter += 1

clrs = ['0', '0.5', '0', '0.5', '0', '0.5', '0', '0.5']
plt.ion()

# raster plot

plt.figure(1)
counter = 1
for j in np.arange(num_pops):
    for i in np.arange(first_node_ids[j], first_node_ids[j] + rec_sizes[j]):
        plt.plot(neuron_spikes[i],
                 np.ones_like(neuron_spikes[i]) + sum(rec_sizes) - counter,
                 'k o', ms=1, mfc=clrs[j], mec=clrs[j])
        counter += 1
plt.xlim(0, T - T_start)
plt.ylim(0, sum(rec_sizes))
plt.xlabel(r'time (ms)')
plt.ylabel(r'neuron id')
plt.savefig(os.path.join(datapath, 'rasterplot.png'))

# firing rates

rates = []
temp = 0

for i in np.arange(num_pops):
    for j in np.arange(first_node_ids[i], last_node_ids[i]):
        temp += len(neuron_spikes[j])
    rates.append(temp / (rec_sizes[i] * (T - T_start)) * 1e3)
    temp = 0

print()
print('Firing rates:')
print(rates)

plt.figure(2)
ticks = np.arange(num_pops)
plt.bar(ticks, rates, width=0.9, color='k')
xticklabels = ['L2/3e', 'L2/3i', 'L4e', 'L4i', 'L5e', 'L5i', 'L6e', 'L6i']
plt.setp(plt.gca(), xticks=ticks + 0.5, xticklabels=xticklabels)
plt.xlabel(r'subpopulation')
plt.ylabel(r'firing rate (spikes/s)')
plt.savefig(os.path.join(datapath, 'firing_rates.png'))

plt.show()
