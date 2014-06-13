# -*- coding: utf-8 -*-
#
# plot_connections.py
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

## Python script that creates a set of Mayavi2 graphs that gives
## on overview of the connection profile of a layer.

## Mayavi2 is required to run this script!

# The histogram2d function must be loaded before calling the 
# functions in this file.
#execfile(plotting_folder+'histogram2d.py')

import numpy as np
import enthought.mayavi.mlab as mlab # Load Mayavi2

## Function that checks if a node satisfies certain criterias.
## Returns true if that is the case.
##
## Input:
## gid - node
## params - dictionary with specification of layer and model type
##
def check_node(gid, params):
    if 'layer' in params:
        if nest.GetLayer(gid) != params['layer']:
            return False
    if 'model' in params:
        if nest.GetStatus(gid)[0]['model'] != params['model']:
            return False
    return True

## 
## Creates a Mayavi2 plot of connection data.
## 
## Input:
## data_file - data file created with the PrintLayerConnections command
## min/max - lower left and upper right corner - [x, y]
## bins - number of histogram bins - [x_number, y_number] 
##        should in most cases be quite alot smaller than the number
##        of rows and columns in the layer
## params - restriction on connection type (see check_node(..) above)
## output - output directory
##
## Example: plot_connections('out.txt', [-1.0, -1.0], [1.0, 1.0], [9, 9],
##                           {'model'= 'iaf_neuron'}, output='folder/')
##
def plot_connections(data_file, min, max, bins,
                     params=None, output=''):
    
    print("Creating connection profile graphs.")
    
    # Read data points from file
    f = open(data_file, 'r')

    # Ignore first line
    f.readline()

    data = []

    for line in f:
        temp = line.split(' ')
        if params != None:
            if check_node([int(temp[1])], params):
                data.append([float(temp[4]), float(temp[5])]);
        else:
            data.append([float(temp[4]), float(temp[5])]);

    # Create histogram data based on the retrieved data.
    histogram_data = histogram2d(data, min, max, bins)

    # Open a new Mayavi2 figure
    f = mlab.figure()

    # Convert histogram bin count to relative densities.
    m = np.max(histogram_data[2].max(axis=0))
    histogram_data[2] = histogram_data[2]/float(m)

    # Plot histogram data
    mlab.mesh(histogram_data[0], histogram_data[1], histogram_data[2])
    #surf(histogram_data[0], histogram_data[1], histogram_data[2])

    # Create and save various viewpoints of histogram figure

    mlab.axes(z_axis_visibility=False)

    mlab.view(azimuth=0, elevation=90) # X

    mlab.savefig(output+"xaxis.eps", size=[600,400])

    mlab.view(azimuth=90, elevation=270) # Y

    mlab.savefig(output+"yaxis.eps", size=[600,400])

    mlab.view(azimuth=45, elevation=45) # Perspective

    mlab.savefig(output+"perspective.eps", size=[600,400])

    mlab.colorbar(orientation="vertical")

    mlab.view(azimuth=0, elevation=0) # Z

    mlab.savefig(output+"above.eps", size=[600,400])

