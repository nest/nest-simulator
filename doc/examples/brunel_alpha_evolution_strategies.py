# -*- coding: utf-8 -*-
#
# brunel_alpha_evolution_strategies.py
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

"""Use evolution strategies to find parameters for a random balanced network (alpha synapses)
-----------------------------------------------------------------------------------------------------

This script uses an optimization algorithm to find the appropriate
parameter values for the external drive "eta" and the relative ratio
of excitation and inhibition "g" for a balanced random network that
lead to particular population-averaged rates, coefficients of
variation and correlations.

From an initial Gaussian search distribution parameterized with mean
and standard deviation network parameters are sampled. Network
realizations of these parameters are simulated and evaluated according
to an objective function that measures how close the activity
statistics are to their desired values (~fitness). From these fitness
values the approximate natural gradient of the fitness landscape is
computed and used to update the parameters of the search
distribution. This procedure is repeated until the maximal number of
function evaluations is reached or the width of the search
distribution becomes extremely small.  We use the following fitness
function:

.. math::

    f = - alpha(r - r*)^2 - beta(cv - cv*)^2 - gamma(corr - corr*)^2

where `alpha`, `beta` and `gamma` are weighting factors, and stars indicate
target values.

The network contains an excitatory and an inhibitory population on
the basis of the network used in [1]_.

The optimization algorithm (evolution strategies) is described in
Wierstra et al. [2]_.


References
~~~~~~~~~~~~

.. [1] Brunel N (2000). Dynamics of Sparsely Connected Networks of
       Excitatory and Inhibitory Spiking Neurons. Journal of Computational
       Neuroscience 8, 183-208.

.. [2] Wierstra et al. (2014). Natural evolution strategies. Journal of
       Machine Learning Research, 15(1), 949-980.

See Also
~~~~~~~~~~

:doc:`brunel_alpha_nest`

Authors
~~~~~~~

Jakob Jordan
"""

from __future__ import print_function
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse
import numpy as np

import nest

from numpy import exp

###############################################################################
# Analysis


def cut_warmup_time(spikes, warmup_time):
    # Removes initial warmup time from recorded spikes
    spikes['senders'] = spikes['senders'][
        spikes['times'] > warmup_time]
    spikes['times'] = spikes['times'][
        spikes['times'] > warmup_time]

    return spikes


def compute_rate(spikes, N_rec, sim_time):
    # Computes average rate from recorded spikes
    return (1. * len(spikes['times']) / N_rec / sim_time * 1e3)


def sort_spikes(spikes):
    # Sorts recorded spikes by gid
    unique_gids = sorted(np.unique(spikes['senders']))
    spiketrains = []
    for gid in unique_gids:
        spiketrains.append(spikes['times'][spikes['senders'] == gid])
    return unique_gids, spiketrains


def compute_cv(spiketrains):
    # Computes coefficient of variation from sorted spikes
    if spiketrains:
        isis = np.hstack([np.diff(st) for st in spiketrains])
        if len(isis) > 1:
            return np.std(isis) / np.mean(isis)
        else:
            return 0.
    else:
        return 0.


def bin_spiketrains(spiketrains, t_min, t_max, t_bin):
    # Bins sorted spikes
    bins = np.arange(t_min, t_max, t_bin)
    return bins, [np.histogram(s, bins=bins)[0] for s in spiketrains]


def compute_correlations(binned_spiketrains):
    # Computes correlations from binned spiketrains
    n = len(binned_spiketrains)
    if n > 1:
        cc = np.corrcoef(binned_spiketrains)
        return 1. / (n * (n - 1.)) * (np.sum(cc) - n)
    else:
        return 0.


def compute_statistics(parameters, espikes, ispikes):
    # Computes population-averaged rates coefficients of variation and
    # correlations from recorded spikes of excitatory and inhibitory
    # populations

    espikes = cut_warmup_time(espikes, parameters['warmup_time'])
    ispikes = cut_warmup_time(ispikes, parameters['warmup_time'])

    erate = compute_rate(espikes, parameters['N_rec'], parameters['sim_time'])
    irate = compute_rate(espikes, parameters['N_rec'], parameters['sim_time'])

    egids, espiketrains = sort_spikes(espikes)
    igids, ispiketrains = sort_spikes(ispikes)

    ecv = compute_cv(espiketrains)
    icv = compute_cv(ispiketrains)

    ecorr = compute_correlations(
        bin_spiketrains(espiketrains, 0., parameters['sim_time'], 1.)[1])
    icorr = compute_correlations(
        bin_spiketrains(ispiketrains, 0., parameters['sim_time'], 1.)[1])

    return (np.mean([erate, irate]),
            np.mean([ecv, icv]),
            np.mean([ecorr, icorr]))


###############################################################################
# Network simulation


def simulate(parameters):
    # Simulates the network and returns recorded spikes for excitatory
    # and inhibitory population

    # Code taken from brunel_alpha_nest.py

    def LambertWm1(x):
        nest.ll_api.sli_push(x)
        nest.ll_api.sli_run('LambertWm1')
        y = nest.ll_api.sli_pop()
        return y

    def ComputePSPnorm(tauMem, CMem, tauSyn):
        a = (tauMem / tauSyn)
        b = (1.0 / tauSyn - 1.0 / tauMem)

        # time of maximum
        t_max = 1.0 / b * (-LambertWm1(-exp(-1.0 / a) / a) - 1.0 / a)

        # maximum of PSP for current of unit amplitude
        return (exp(1.0) / (tauSyn * CMem * b) *
                ((exp(-t_max / tauMem) - exp(-t_max / tauSyn)) / b -
                 t_max * exp(-t_max / tauSyn)))

    # number of excitatory neurons
    NE = int(parameters['gamma'] * parameters['N'])
    # number of inhibitory neurons
    NI = parameters['N'] - NE

    # number of excitatory synapses per neuron
    CE = int(parameters['epsilon'] * NE)
    # number of inhibitory synapses per neuron
    CI = int(parameters['epsilon'] * NI)

    tauSyn = 0.5  # synaptic time constant in ms
    tauMem = 20.0  # time constant of membrane potential in ms
    CMem = 250.0  # capacitance of membrane in in pF
    theta = 20.0  # membrane threshold potential in mV
    neuron_parameters = {
        'C_m': CMem,
        'tau_m': tauMem,
        'tau_syn_ex': tauSyn,
        'tau_syn_in': tauSyn,
        't_ref': 2.0,
        'E_L': 0.0,
        'V_reset': 0.0,
        'V_m': 0.0,
        'V_th': theta
    }
    J = 0.1        # postsynaptic amplitude in mV
    J_unit = ComputePSPnorm(tauMem, CMem, tauSyn)
    J_ex = J / J_unit  # amplitude of excitatory postsynaptic current
    # amplitude of inhibitory postsynaptic current
    J_in = -parameters['g'] * J_ex

    nu_th = (theta * CMem) / (J_ex * CE * exp(1) * tauMem * tauSyn)
    nu_ex = parameters['eta'] * nu_th
    p_rate = 1000.0 * nu_ex * CE

    nest.ResetKernel()
    nest.set_verbosity('M_FATAL')

    nest.SetKernelStatus({'rng_seeds': [parameters['seed']],
                          'resolution': parameters['dt']})

    nest.SetDefaults('iaf_psc_alpha', neuron_parameters)
    nest.SetDefaults('poisson_generator', {'rate': p_rate})

    nodes_ex = nest.Create('iaf_psc_alpha', NE)
    nodes_in = nest.Create('iaf_psc_alpha', NI)
    noise = nest.Create('poisson_generator')
    espikes = nest.Create('spike_detector', params={'label': 'brunel-py-ex'})
    ispikes = nest.Create('spike_detector', params={'label': 'brunel-py-in'})

    nest.CopyModel('static_synapse', 'excitatory',
                   {'weight': J_ex, 'delay': parameters['delay']})
    nest.CopyModel('static_synapse', 'inhibitory',
                   {'weight': J_in, 'delay': parameters['delay']})

    nest.Connect(noise, nodes_ex, syn_spec='excitatory')
    nest.Connect(noise, nodes_in, syn_spec='excitatory')

    if parameters['N_rec'] > NE:
        raise ValueError(
            'Requested recording from {} neurons, \
            but only {} in excitatory population'.format(
                parameters['N_rec'], NE))
    if parameters['N_rec'] > NI:
        raise ValueError(
            'Requested recording from {} neurons, \
            but only {} in inhibitory population'.format(
                parameters['N_rec'], NI))
    nest.Connect(nodes_ex[:parameters['N_rec']], espikes)
    nest.Connect(nodes_in[:parameters['N_rec']], ispikes)

    conn_parameters_ex = {'rule': 'fixed_indegree', 'indegree': CE}
    nest.Connect(
        nodes_ex, nodes_ex + nodes_in, conn_parameters_ex, 'excitatory')

    conn_parameters_in = {'rule': 'fixed_indegree', 'indegree': CI}
    nest.Connect(
        nodes_in, nodes_ex + nodes_in, conn_parameters_in, 'inhibitory')

    nest.Simulate(parameters['sim_time'])

    return (nest.GetStatus(espikes, 'events')[0],
            nest.GetStatus(ispikes, 'events')[0])


###############################################################################
# Optimization


def default_population_size(dimensions):
    # Returns a population size suited for the given number of dimensions
    # See Wierstra et al. (2014)

    return 4 + int(np.floor(3 * np.log(dimensions)))


def default_learning_rate_mu():
    # Returns a default learning rate for the mean of the search distribution
    # See Wierstra et al. (2014)

    return 1


def default_learning_rate_sigma(dimensions):
    # Returns a default learning rate for the standard deviation of the
    # search distribution for the given number of dimensions
    # See Wierstra et al. (2014)

    return (3 + np.log(dimensions)) / (12. * np.sqrt(dimensions))


def compute_utility(fitness):
    # Computes utility and order used for fitness shaping
    # See Wierstra et al. (2014)

    n = len(fitness)
    order = np.argsort(fitness)[::-1]
    fitness = fitness[order]

    utility = [
        np.max([0, np.log((n / 2) + 1)]) - np.log(k + 1) for k in range(n)]
    utility = utility / np.sum(utility) - 1. / n

    return order, utility


def optimize(func, mu, sigma, learning_rate_mu=None, learning_rate_sigma=None,
             population_size=None, fitness_shaping=True,
             mirrored_sampling=True, record_history=False,
             max_generations=2000, min_sigma=1e-8, verbosity=0):

    ###########################################################################
    # Optimizes an objective function via evolution strategies using the
    # natural gradient of multinormal search distributions in natural
    # coordinates.  Does not consider covariances between parameters (
    # "Separable natural evolution strategies").
    # See Wierstra et al. (2014)
    #
    # Parameters
    # ----------
    # func: function
    #     The function to be maximized.
    # mu: float
    #     Initial mean of the search distribution.
    # sigma: float
    #     Initial standard deviation of the search distribution.
    # learning_rate_mu: float
    #     Learning rate of mu.
    # learning_rate_sigma: float
    #     Learning rate of sigma.
    # population_size: int
    #     Number of individuals sampled in each generation.
    # fitness_shaping: bool
    #     Whether to use fitness shaping, compensating for large
    #     deviations in fitness, see Wierstra et al. (2014).
    # mirrored_sampling: bool
    #     Whether to use mirrored sampling, i.e., evaluating a mirrored
    #     sample for each sample, see Wierstra et al. (2014).
    # record_history: bool
    #     Whether to record history of search distribution parameters,
    #     fitness values and individuals.
    # max_generations: int
    #     Maximal number of generations.
    # min_sigma: float
    #     Minimal value for standard deviation of search
    #     distribution. If any dimension has a value smaller than this,
    #     the search is stoppped.
    # verbosity: bool
    #     Whether to continuously print progress information.
    #
    # Returns
    # -------
    # dict
    #     Dictionary of final parameters of search distribution and
    #     history.

    if not isinstance(mu, np.ndarray):
        raise TypeError('mu needs to be of type np.ndarray')
    if not isinstance(sigma, np.ndarray):
        raise TypeError('sigma needs to be of type np.ndarray')

    if learning_rate_mu is None:
        learning_rate_mu = default_learning_rate_mu()
    if learning_rate_sigma is None:
        learning_rate_sigma = default_learning_rate_sigma(mu.size)
    if population_size is None:
        population_size = default_population_size(mu.size)

    generation = 0
    mu_history = []
    sigma_history = []
    pop_history = []
    fitness_history = []

    while True:

        # create new population using the search distribution
        s = np.random.normal(0, 1, size=(population_size,) + np.shape(mu))
        z = mu + sigma * s

        # add mirrored perturbations if enabled
        if mirrored_sampling:
            z = np.vstack([z, mu - sigma * s])
            s = np.vstack([s, -s])

        # evaluate fitness for every individual in population
        fitness = np.fromiter((func(*zi) for zi in z), np.float)

        # print status if enabled
        if verbosity > 0:
            print(
                '# Generation {:d} | fitness {:.3f} | mu {} | sigma {}'.format(
                    generation, np.mean(fitness),
                    ', '.join(str(np.round(mu_i, 3)) for mu_i in mu),
                    ', '.join(str(np.round(sigma_i, 3)) for sigma_i in sigma)
                ))

        # apply fitness shaping if enabled
        if fitness_shaping:
            order, utility = compute_utility(fitness)
            s = s[order]
            z = z[order]
        else:
            utility = fitness

        # bookkeeping
        if record_history:
            mu_history.append(mu.copy())
            sigma_history.append(sigma.copy())
            pop_history.append(z.copy())
            fitness_history.append(fitness)

        # exit if max generations reached or search distributions are
        # very narrow
        if generation == max_generations or np.all(sigma < min_sigma):
            break

        # update parameter of search distribution via natural gradient
        # descent in natural coordinates
        mu += learning_rate_mu * sigma * np.dot(utility, s)
        sigma *= np.exp(learning_rate_sigma / 2. * np.dot(utility, s**2 - 1))

        generation += 1

    return {
        'mu': mu,
        'sigma': sigma,
        'fitness_history': np.array(fitness_history),
        'mu_history': np.array(mu_history),
        'sigma_history': np.array(sigma_history),
        'pop_history': np.array(pop_history)
    }


def optimize_network(optimization_parameters, simulation_parameters):
    # Searches for suitable network parameters to fulfill defined constraints

    np.random.seed(simulation_parameters['seed'])

    def objective_function(g, eta):
        # Returns the fitness of a specific network parametrization

        # create local copy of parameters that uses parameters given
        # by optimization algorithm
        simulation_parameters_local = simulation_parameters.copy()
        simulation_parameters_local['g'] = g
        simulation_parameters_local['eta'] = eta

        # perform the network simulation
        espikes, ispikes = simulate(simulation_parameters_local)

        # analyse the result and compute fitness
        rate, cv, corr = compute_statistics(
            simulation_parameters, espikes, ispikes)
        fitness = \
            - optimization_parameters['fitness_weight_rate'] * (
                rate - optimization_parameters['target_rate']) ** 2 \
            - optimization_parameters['fitness_weight_cv'] * (
                cv - optimization_parameters['target_cv']) ** 2 \
            - optimization_parameters['fitness_weight_corr'] * (
                corr - optimization_parameters['target_corr']) ** 2

        return fitness

    return optimize(
        objective_function,
        np.array(optimization_parameters['mu']),
        np.array(optimization_parameters['sigma']),
        max_generations=optimization_parameters['max_generations'],
        record_history=True,
        verbosity=optimization_parameters['verbosity']
    )

###############################################################################
# Main


if __name__ == '__main__':
    simulation_parameters = {
        'seed': 123,
        'dt': 0.1,            # (ms) simulation resolution
        'sim_time': 1000.,    # (ms) simulation duration
        'warmup_time': 300.,  # (ms) duration ignored during analysis
        'delay': 1.5,         # (ms) synaptic delay
        'g': None,            # relative ratio of excitation and inhibition
        'eta': None,          # relative strength of external drive
        'epsilon': 0.1,       # average connectivity of network
        'N': 400,             # number of neurons in network
        'gamma': 0.8,         # relative size of excitatory and
                              # inhibitory population
        'N_rec': 40,          # number of neurons to record activity from
    }

    optimization_parameters = {
        'verbosity': 1,             # print progress over generations
        'max_generations': 20,      # maximal number of generations
        'target_rate': 1.89,        # (spikes/s) target rate
        'target_corr': 0.0,         # target correlation
        'target_cv': 1.,            # target coefficient of variation
        'mu': [1., 3.],             # initial mean for search distribution
                                    # (mu(g), mu(eta))
        'sigma': [0.15, 0.05],      # initial sigma for search
                                    # distribution (sigma(g), sigma(eta))

        # hyperparameters of the fitness function; these are used to
        # compensate for the different typical scales of the
        # individual measures, rate ~ O(1), cv ~ (0.1), corr ~ O(0.01)
        'fitness_weight_rate': 1.,    # relative weight of rate deviation
        'fitness_weight_cv': 10.,     # relative weight of cv deviation
        'fitness_weight_corr': 100.,  # relative weight of corr deviation
    }

    # optimize network parameters
    optimization_result = optimize_network(optimization_parameters,
                                           simulation_parameters)

    simulation_parameters['g'] = optimization_result['mu'][0]
    simulation_parameters['eta'] = optimization_result['mu'][1]

    espikes, ispikes = simulate(simulation_parameters)

    rate, cv, corr = compute_statistics(
        simulation_parameters, espikes, ispikes)
    print('Statistics after optimization:', end=' ')
    print('Rate: {:.3f}, cv: {:.3f}, correlation: {:.3f}'.format(
        rate, cv, corr))

    # plot results
    fig = plt.figure(figsize=(10, 4))
    ax1 = fig.add_axes([0.06, 0.12, 0.25, 0.8])
    ax2 = fig.add_axes([0.4, 0.12, 0.25, 0.8])
    ax3 = fig.add_axes([0.74, 0.12, 0.25, 0.8])

    ax1.set_xlabel('Time (ms)')
    ax1.set_ylabel('Neuron id')

    ax2.set_xlabel(r'Relative strength of inhibition $g$')
    ax2.set_ylabel(r'Relative strength of external drive $\eta$')

    ax3.set_xlabel('Generation')
    ax3.set_ylabel('Fitness')

    # raster plot
    ax1.plot(espikes['times'], espikes['senders'], ls='', marker='.')

    # search distributions and individuals
    for mu, sigma in zip(optimization_result['mu_history'],
                         optimization_result['sigma_history']):
        ellipse = Ellipse(
            xy=mu, width=2 * sigma[0], height=2 * sigma[1], alpha=0.5, fc='k')
        ellipse.set_clip_box(ax2.bbox)
        ax2.add_artist(ellipse)
    ax2.plot(optimization_result['mu_history'][:, 0],
             optimization_result['mu_history'][:, 1],
             marker='.', color='k', alpha=0.5)
    for generation in optimization_result['pop_history']:
        ax2.scatter(generation[:, 0], generation[:, 1])

    # fitness over generations
    ax3.errorbar(np.arange(len(optimization_result['fitness_history'])),
                 np.mean(optimization_result['fitness_history'], axis=1),
                 yerr=np.std(optimization_result['fitness_history'], axis=1))

    fig.savefig('brunel_alpha_evolution_strategies.pdf')
