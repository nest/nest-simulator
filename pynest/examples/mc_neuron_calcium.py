# -*- coding: utf-8 -*-
#
# mc_neuron.py
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

'''
Multi-compartment neuron with calcium spike (both kinetics and fixed
wavform) example
--------------------------------

Simple example of how to use the three-compartment iaf_cond_alpha_mc_kinetics
and iaf_cond_alpha_mc_fixedca neuron models with calcium spike.

A stimulation paradigm with beta waveform current is illustrated:

The simulation reproduces results shown in Fig.1b and Fig.8b in [1].

Voltage traces are shown for all compartments.

[1] Chua, Y., Morrison, A., & Helias, M. (2015).
Modeling the calcium spike as a threshold triggered fixed waveform for
synchronous inputs in the fluctuation regime. Frontiers in Computational
Neuroscience., 9(00091).
'''

import nest
import pylab
import numpy as np

nest.ResetKernel()

# Obtain receptor dictionary

ksyns = nest.GetDefaults('iaf_cond_alpha_mc_kinetics')['receptor_types']
fsyns = nest.GetDefaults('iaf_cond_alpha_mc_fixedca')['receptor_types']

# Obtain list of recordable quantities

krqs = nest.GetDefaults('iaf_cond_alpha_mc_kinetics')['recordables']
frqs = nest.GetDefaults('iaf_cond_alpha_mc_fixedca')['recordables']

# Change some default values:

el_s = -70.
el_p = -65.
el_d = -60.

gld = 20.
glp = 10.
gls = 10.
gpd = 10.
gsp = 30.
c_d = 60.
c_p = 80.
c_s = 150.
h_m = -21.
h_h = -24.
eca = 30.
gca = 70.
taum = 5.
tauh = 50.
slopem = 0.5
slopeh = -0.5
ca_th = -25.
fprox_jump = 400.0
fdist_jump = 310.0
fjump_th = 6.0
ftau_th = 7.0
kprox_jump = 400.0
kdist_jump = 310.0
kjump_th = 6.0
ktau_th = 7.0

nest.SetDefaults('iaf_cond_alpha_mc_kinetics', {
    'g_sp': gsp,
    'g_pd': gpd,
    'V_th': -55.0,
    'act_flag': 1.,
    'jump_Th': kjump_th,
    'tau_Th': ktau_th,
    'half_m': h_m,
    'half_h': h_h,
    'E_Ca': eca,
    'G_Ca': gca,
    'tau_m': taum,
    'tau_h': tauh,
    'slope_m': slopem,
    'slope_h': slopeh,
    'distal': {
        't_L': gld,
        'nt_L': gld,
        'E_L': el_d,
        'tau_syn_ex': 1.,
        'tau_syn_in': 2.0,
        'C_m': c_d,
        'amp_cur_AP': kdist_jump,
        'tau_cur_AP': 1.,
        },
    'proximal': {
        't_L': glp,
        'nt_L': glp,
        'E_L': el_p,
        'tau_syn_ex': 1.,
        'tau_syn_in': 2.0,
        'C_m': c_p,
        'amp_cur_AP': kprox_jump,
        'tau_cur_AP': 1.,
        },
    'soma': {
        't_L': 150.,
        'nt_L': gls,
        'E_L': el_s,
        'tau_syn_ex': 1.,
        'tau_syn_in': 2.0,
        'C_m': c_s,
        'amp_cur_AP': 0.,
        'tau_cur_AP': 1.,
        },
    })

nest.SetDefaults('iaf_cond_alpha_mc_fixedca', {
    'g_sp': gsp,
    'g_pd': gpd,
    'V_th': -55.0,
    'act_flag': 1.,
    'V_thCa': ca_th,
    'jump_Th': fjump_th,
    'tau_Th': ftau_th,
    'distal': {
        't_L': gld,
        'nt_L': gld,
        'E_L': el_d,
        'tau_syn_ex': 1.,
        'tau_syn_in': 2.0,
        'C_m': c_d,
        'amp_cur_AP': fdist_jump,
        'tau_cur_AP': 1.,
        },
    'proximal': {
        't_L': glp,
        'nt_L': glp,
        'E_L': el_p,
        'tau_syn_ex': 1.,
        'tau_syn_in': 2.0,
        'C_m': c_p,
        'amp_cur_AP': fprox_jump,
        'tau_cur_AP': 1.,
        },
    'soma': {
        't_L': 150.,
        'nt_L': gls,
        'E_L': el_s,
        'tau_syn_ex': 1.,
        'tau_syn_in': 2.0,
        'C_m': c_s,
        'amp_cur_AP': 0.,
        'tau_cur_AP': 1.,
        },
    })

# Create neuron

kn = nest.Create('iaf_cond_alpha_mc_kinetics')

# Create multimeter recording everything, connect

kmm = nest.Create('multimeter', params={'record_from': krqs,
                  'interval': 0.1})
nest.Connect(kmm, kn)

# Create neuron

fn = nest.Create('iaf_cond_alpha_mc_fixedca')

# Create multimeter recording everything, connect

fmm = nest.Create('multimeter', params={'record_from': frqs,
                  'interval': 0.1})
nest.Connect(fmm, fn)

# Create one current generator for distal compartment

t = np.linspace(0.1, 299., 2990)
ca_curr = np.array([
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    309.901243356,
    584.175028479,
    826.333266247,
    1039.55125897,
    1226.69978059,
    1390.37410306,
    1532.92025962,
    1656.45880784,
    1762.90633049,
    1853.99488937,
    1931.2896271,
    1996.20469297,
    2050.01765246,
    2093.88252472,
    2128.84157852,
    2155.83600501,
    2175.71557393,
    2189.2473703,
    2197.1236989,
    2199.96923581,
    2198.34749881,
    2192.76670117,
    2183.6850479,
    2171.5155272,
    2156.63024535,
    2139.36434844,
    2120.01957029,
    2098.86744214,
    2076.15219625,
    2052.09339266,
    2026.88829538,
    2000.7140219,
    1973.72948763,
    1946.07716468,
    1917.88467283,
    1889.26621853,
    1860.32389648,
    1831.14886685,
    1801.82241992,
    1772.41693907,
    1742.99677149,
    1713.6190157,
    1684.33423365,
    1655.18709453,
    1626.21695704,
    1597.45839564,
    1568.94167646,
    1540.69318744,
    1512.73582715,
    1485.08935619,
    1457.77071483,
    1430.79431,
    1404.17227461,
    1377.91470183,
    1352.02985671,
    1326.52436737,
    1301.40339757,
    1276.67080259,
    1252.32926993,
    1228.38044628,
    1204.82505214,
    1181.66298517,
    1158.89341344,
    1136.51485949,
    1114.52527613,
    1092.9221147,
    1071.70238661,
    1050.86271866,
    1030.39940296,
    1010.30844172,
    990.585587557,
    971.226379761,
    952.226176793,
    933.580185487,
    915.28348723,
    897.331061417,
    879.717806439,
    862.438558463,
    845.488108193,
    828.861215832,
    812.552624399,
    796.557071584,
    780.869300263,
    765.484067818,
    750.396154376,
    735.600370074,
    721.091561445,
    706.864617011,
    692.914472169,
    679.236113433,
    665.824582101,
    652.674977403,
    639.782459187,
    627.142250183,
    614.749637895,
    602.599976156,
    590.688686382,
    579.01125856,
    567.56325199,
    556.340295815,
    545.338089359,
    534.552402299,
    523.979074674,
    513.614016774,
    503.453208898,
    493.492701014,
    483.728612323,
    474.157130744,
    464.774512325,
    455.577080598,
    446.561225869,
    437.723404475,
    429.060137992,
    420.568012413,
    412.243677297,
    404.083844894,
    396.085289252,
    388.244845306,
    380.559407959,
    373.025931151,
    365.641426922,
    358.402964475,
    351.307669232,
    344.352721895,
    337.535357507,
    330.852864517,
    324.302583851,
    317.881907988,
    311.588280045,
    305.419192867,
    299.37218813,
    293.444855449,
    287.634831503,
    281.939799164,
    276.357486638,
    270.885666623,
    265.522155473,
    260.264812375,
    255.111538544,
    250.06027642,
    245.109008885,
    240.255758494,
    235.49858671,
    230.835593162,
    226.264914905,
    221.784725704,
    217.393235319,
    213.088688811,
    208.869365858,
    204.733580078,
    200.679678373,
    196.706040279,
    192.811077328,
    188.993232425,
    185.250979233,
    181.582821572,
    177.987292827,
    174.462955371,
    171.008399992,
    167.622245339,
    164.303137373,
    161.049748829,
    157.860778692,
    154.734951676,
    151.671017725,
    148.667751505,
    145.723951929,
    142.838441668,
    140.01006669,
    137.237695799,
    134.520220179,
    131.856552961,
    129.245628784,
    126.686403369,
    124.177853108,
    121.718974652,
    119.308784512,
    116.946318663,
    114.630632167,
    112.360798786,
    110.135910622,
    107.955077745,
    105.817427847,
    103.722105885,
    101.668273745,
    99.6551099046,
    97.681809107,
    95.7475820367,
    93.8516550054,
    91.9932696429,
    90.1716825934,
    88.386165219,
    86.6360033079,
    84.9204967893,
    83.2389594533,
    81.5907186767,
    79.9751151539,
    78.3915026337,
    76.8392476602,
    75.3177293204,
    73.8263389951,
    72.364480116,
    70.9315679272,
    69.527029251,
    68.150302259,
    66.8008362472,
    65.4780914158,
    64.1815386536,
    62.9106593261,
    61.664945068,
    60.4438975801,
    59.2470284302,
    58.073858857,
    56.9239195795,
    55.7967506088,
    54.6919010641,
    53.6089289926,
    52.5474011926,
    51.5068930403,
    50.4869883198,
    49.4872790568,
    48.5073653552,
    47.5468552376,
    46.6053644878,
    45.6825164979,
    44.777942117,
    43.8912795039,
    43.0221739824,
    42.1702778992,
    41.3352504849,
    40.5167577179,
    39.7144721905,
    38.9280729783,
    38.1572455113,
    37.4016814485,
    36.6610785545,
    35.9351405786,
    35.223577136,
    34.5261035922,
    33.8424409484,
    33.1723157308,
    32.5154598804,
    31.8716106463,
    31.2405104803,
    30.6219069341,
    30.015552558,
    29.4212048024,
    28.8386259202,
    28.2675828722,
    27.7078472337,
    27.1591951029,
    26.6214070119,
    26.0942678381,
    25.577566719,
    25.0710969673,
    24.5746559883,
    24.0880451991,
    23.6110699488,
    23.1435394411,
    22.6852666576,
    22.2360682829,
    21.7957646319,
    21.3641795772,
    20.941140479,
    20.5264781161,
    20.1200266181,
    19.7216233988,
    19.3311090917,
    18.948327486,
    18.5731254638,
    18.2053529393,
    17.8448627988,
    17.4915108412,
    17.1451557211,
    16.8056588918,
    16.4728845502,
    16.1466995819,
    15.8269735088,
    15.513578436,
    15.2063890015,
    14.9052823252,
    14.6101379606,
    14.320837846,
    14.0372662575,
    13.7593097626,
    13.486857175,
    13.2197995102,
    12.9580299413,
    12.7014437573,
    12.44993832,
    12.2034130241,
    11.9617692561,
    11.7249103553,
    11.492741575,
    11.2651700445,
    11.0421047323,
    10.8234564091,
    10.6091376129,
    10.3990626131,
    10.193147377,
    9.9913095358,
    9.79346835161,
    9.59954468535,
    9.40946096495,
    9.22314115439,
    9.04051072328,
    8.86149661699,
    8.6860272275,
    8.51403236471,
    8.34544322839,
    8.18019238064,
    8.0182137189,
    7.85944244957,
    7.70381506201,
    7.55126930319,
    7.40174415277,
    7.25517979871,
    7.1115176133,
    6.97070012976,
    6.83267101922,
    6.69737506818,
    6.56475815647,
    6.43476723556,
    6.30735030733,
    6.18245640333,
    6.06003556432,
    5.94003882034,
    5.82241817109,
    5.70712656675,
    5.59411788912,
    5.48334693324,
    5.37476938924,
    5.26834182467,
    5.16402166706,
    5.06176718699,
    4.96153748127,
    4.8632924567,
    4.76699281397,
    4.67260003191,
    4.58007635217,
    4.48938476405,
    4.40048898968,
    4.31335346959,
    4.22794334839,
    4.14422446091,
    4.06216331847,
    3.98172709552,
    3.9028836165,
    3.82560134296,
    3.74984936097,
    3.67559736873,
    3.60281566444,
    3.53147513445,
    3.4615472416,
    3.39300401381,
    3.32581803286,
    3.25996242347,
    3.19541084251,
    3.1321374685,
    3.07011699124,
    3.00932460171,
    2.94973598215,
    2.8913272963,
    2.83407517993,
    2.77795673142,
    2.72294950264,
    2.66903148997,
    2.61618112549,
    2.56437726833,
    2.51359919628,
    2.46382659741,
    2.41503956204,
    2.3672185747,
    2.32034450635,
    2.27439860674,
    2.22936249689,
    2.18521816177,
    2.14194794306,
    2.09953453208,
    2.0579609629,
    2.01721060555,
    1.97726715935,
    1.93811464636,
    1.89973740508,
    1.86212008409,
    1.82524763597,
    1.78910531123,
    1.75367865247,
    1.71895348856,
    1.68491592896,
    1.6515523582,
    1.61884943041,
    1.58679406397,
    1.55537343631,
    1.52457497877,
    1.49438637155,
    1.4647955388,
    1.4357906438,
    1.4073600842,
    1.37949248739,
    1.35217670598,
    1.32540181328,
    1.29915709897,
    1.27343206483,
    1.24821642049,
    1.22350007937,
    1.19927315459,
    1.17552595506,
    1.15224898159,
    1.12943292307,
    1.10706865278,
    1.08514722472,
    1.06365987001,
    1.04259799343,
    1.02195316995,
    1.00171714137,
    0.981881813,
    0.962439250447,
    0.943381676426,
    0.924701467655,
    0.9063911518,
    0.888443404492,
    0.870851046391,
    0.85360704032,
    0.836704488447,
    0.820136629526,
    0.803896836191,
    0.78797861231,
    0.772375590381,
    0.757081528986,
    0.742090310297,
    0.727395937627,
    0.71299253303,
    0.698874334954,
    0.68503569593,
    0.671471080318,
    0.658175062092,
    0.645142322666,
    0.632367648771,
    0.619845930368,
    0.607572158601,
    0.595541423799,
    0.583748913507,
    0.572189910564,
    0.560859791214,
    0.549754023259,
    0.538868164244,
    0.528197859679,
    0.517738841301,
    0.507486925362,
    0.497438010961,
    0.487588078396,
    0.477933187564,
    0.468469476379,
    0.459193159232,
    0.450100525471,
    0.441187937921,
    0.432451831429,
    0.423888711436,
    0.415495152579,
    0.407267797323,
    0.399203354617,
    0.391298598575,
    0.38355036719,
    0.375955561066,
    0.368511142179,
    0.361214132663,
    0.354061613616,
    0.347050723935,
    0.340178659171,
    0.333442670407,
    0.326840063157,
    0.32036819629,
    0.314024480973,
    0.307806379636,
    0.301711404954,
    0.295737118858,
    0.289881131552,
    0.284141100564,
    0.278514729804,
    0.272999768651,
    0.267594011044,
    0.262295294611,
    0.257101499792,
    0.252010549001,
    0.24702040579,
    0.242129074035,
    0.237334597138,
    0.232635057245,
    0.228028574476,
    0.223513306178,
    0.219087446182,
    0.214749224085,
    0.210496904542,
    0.206328786567,
    0.202243202858,
    0.198238519127,
    0.194313133447,
    0.19046547561,
    0.186694006504,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    0.,
    ])
cgs1 = nest.Create('step_current_generator', 1)
nest.SetStatus(cgs1, [{'amplitude_times': t,
               'amplitude_values': ca_curr}])  # soma

# Connect generators to distal compartments

nest.Connect([cgs1[0]], kn,
             syn_spec={'receptor_type': ksyns['distal_curr']})
nest.Connect([cgs1[0]], fn,
             syn_spec={'receptor_type': fsyns['distal_curr']})

# Simulate

nest.Simulate(300)

# Retrieve data

krec = nest.GetStatus(kmm)[0]['events']
kt = krec['times']
frec = nest.GetStatus(fmm)[0]['events']
ft = frec['times']

pylab.figure()
pylab.subplot(211)
pylab.plot(
    kt,
    krec['V_m.s'],
    'k',
    kt,
    krec['V_m.p'],
    'b',
    kt,
    krec['V_m.d'],
    'r',
    )
pylab.legend(('Soma', 'Proximal dendrite', 'Distal dendrite'),
             loc='upper right')
pylab.axis([0, 300, -76, 31])
pylab.ylabel('Membrane potential [mV]')
pylab.title('Responses of iaf_cond_alpha_mc_kinetics')

pylab.subplot(212)
pylab.plot(
    ft,
    frec['V_m.s'],
    'k',
    ft,
    frec['V_m.p'],
    'b',
    ft,
    frec['V_m.d'],
    'r',
    )
pylab.legend(('Soma', 'Proximal dendrite', 'Distal dendrite'),
             loc='upper right')
pylab.axis([0, 300, -76, 31])
pylab.ylabel('Membrane potential [mV]')
pylab.title('Responses of iaf_cond_alpha_mc_fixedca')
pylab.xlabel('Time [ms]')
