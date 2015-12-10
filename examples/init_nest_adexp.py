#!/usr/bin/env python
#-*- coding:utf-8 -*-

""" Script to initiate NEST and the functions """

try:
   import nngt
   with_nngt = True
except:
   with_nngt = False
import nest

import time
import numpy as np

import matplotlib
matplotlib.use('QT4Agg') # avoid tk related errors on multiple runs
import matplotlib.cm as cm
import matplotlib.pyplot as plt
from matplotlib import rcParams



#
#---
# Setup parameters
#--------------------

rcParams.update({
        'savefig.directory': '/home/tfardet/CloudStation/NEST/Documents/pic/AdExp',
        'savefig.dpi': 300,
        'savefig.format': 'pdf',
        'path.simplify': True,
        'font.family': 'serif',
        'legend.handletextpad': 0.3})

di_param_default = {
    'V_reset': -60.0,
    'V_peak': 0.0,
    'V_th': -50.4,
    'I_e': 0.0,
    'g_L': 30.0,
    'tau_w': 144.0,
    'E_L': -70.6,
    'Delta_T': 2.0,
    'a': 4.0,
    'b': 80.5,
    'C_m': 281.0
}

di_param = {
    'V_reset': -58.,
    'V_peak': 0.0,
    'V_th': -50.,
    'I_e': 0.0,
    'g_L': 10.,
    'tau_w': 30.0,
    'E_L': -70.,
    'Delta_T': 2.,
    'a': 2.,
    'b': 0.,
    'C_m': 200.0,
    'V_m': -70.
}


#
#---
# Nullcline functions
#--------------------

def V_nullcline(V_min, V_max, num_points, di_param):
    da_voltage = np.linspace(V_min,V_max,num_points)
    da_adapt = np.repeat(di_param["I_e"],num_points)\
        + di_param["g_L"]*(-(da_voltage-np.repeat(di_param["E_L"],num_points))\
        + di_param["Delta_T"]*np.exp(np.divide(da_voltage-np.repeat(di_param["V_th"],num_points),di_param["Delta_T"])))
    return da_voltage,da_adapt

def w_nullcline(V_min, V_max, num_points, di_param):
    da_voltage = np.linspace(V_min,V_max,num_points)
    da_adapt = di_param["a"]*(da_voltage-np.repeat(di_param["E_L"],num_points))
    return da_voltage,da_adapt
