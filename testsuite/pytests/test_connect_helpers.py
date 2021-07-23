# -*- coding: utf-8 -*-
#
# test_connect_helpers.py
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

import numpy as np
import scipy.stats
import nest
from scipy.stats import truncexpon

try:
    from mpi4py import MPI
    haveMPI4Py = True
except ImportError:
    haveMPI4Py = False


def gather_data(data_array):
    '''
    Gathers data from all mpi processes by collecting all element in a list if
    data is a list and summing all elements to one numpy-array if data is one
    numpy-array. Returns gathered data if rank of current mpi node is zero and
    None otherwise.

    '''
    if haveMPI4Py:
        data_array_list = MPI.COMM_WORLD.gather(data_array, root=0)
        if MPI.COMM_WORLD.Get_rank() == 0:
            if isinstance(data_array, list):
                gathered_data = [
                    item for sublist in data_array_list for item in sublist]
            else:
                gathered_data = sum(data_array_list)
            return gathered_data
        else:
            return None
    else:
        return data_array


def bcast_data(data):
    """
    Broadcasts data from the root MPI node to all other nodes.
    """
    if haveMPI4Py:
        data = MPI.COMM_WORLD.bcast(data, root=0)
    return data


def is_array(data):
    '''
    Returns True if data is a list or numpy-array and False otherwise.
    '''
    return isinstance(data, (list, np.ndarray, np.generic))


def mpi_barrier():
    if haveMPI4Py:
        MPI.COMM_WORLD.Barrier()


def mpi_assert(data_original, data_test, TestCase):
    '''
    Compares data_original and data_test using assertTrue from the TestCase.
    '''

    data_original = gather_data(data_original)
    # only test if on rank 0
    if data_original is not None:
        if isinstance(data_original, (np.ndarray, np.generic)) \
           and isinstance(data_test, (np.ndarray, np.generic)):
            TestCase.assertTrue(np.allclose(data_original, data_test))
        else:
            TestCase.assertTrue(data_original == data_test)


def all_equal(x):
    '''
    Tests if all elements in a list are equal.
    Returns True or False
    '''
    return x.count(x[0]) == len(x)


def get_connectivity_matrix(pop1, pop2):
    '''
    Returns a connectivity matrix describing all connections from pop1 to pop2
    such that M_ij describes the connection between the jth neuron in pop1 to
    the ith neuron in pop2.
    '''

    M = np.zeros((len(pop2), len(pop1)))
    connections = nest.GetConnections(pop1, pop2)
    index_dic = {}
    for count, node in enumerate(pop1):
        index_dic[node.get('global_id')] = count
    for count, node in enumerate(pop2):
        index_dic[node.get('global_id')] = count
    for source, target in zip(connections.sources(), connections.targets()):
        M[index_dic[target]][index_dic[source]] += 1
    return M


def get_weighted_connectivity_matrix(pop1, pop2, label):
    '''
    Returns a weighted connectivity matrix describing all connections from
    pop1 to pop2 such that M_ij describes the connection between the jth
    neuron in pop1 to the ith neuron in pop2. Only works without multapses.
    '''

    M = np.zeros((len(pop2), len(pop1)))
    connections = nest.GetConnections(pop1, pop2)
    sources = connections.get('source')
    targets = connections.get('target')
    weights = connections.get(label)
    index_dic = {}
    for count, node in enumerate(pop1):
        index_dic[node.get('global_id')] = count
    for count, node in enumerate(pop2):
        index_dic[node.get('global_id')] = count
    for counter, weight in enumerate(weights):
        source_id = sources[counter]
        target_id = targets[counter]
        M[index_dic[target_id]][index_dic[source_id]] += weight
    return M


def check_synapse(params, values, syn_params, TestCase):
    for i, param in enumerate(params):
        syn_params[param] = values[i]
    TestCase.setUpNetwork(TestCase.conn_dict, syn_params)
    for i, param in enumerate(params):
        conns = nest.GetConnections(TestCase.pop1, TestCase.pop2)
        conn_params = conns.get(param)
        TestCase.assertTrue(all_equal(conn_params))
        TestCase.assertTrue(conn_params[0] == values[i])

# copied from Masterthesis, Daniel Hjertholm


def counter(x, fan, source_pop, target_pop):
    '''
    Count similar elements in list.

    Parameters
    ----------
        x: Any list.

    Return values
    -------------
        list containing counts of similar elements.
    '''

    N_p = len(source_pop) if fan == 'in' else len(target_pop)  # of pool nodes.
    start = min(x)
    counts = [0] * N_p
    for elem in x:
        counts[elem - start] += 1

    return counts


def get_degrees(fan, pop1, pop2):
    M = get_connectivity_matrix(pop1, pop2)
    if fan == 'in':
        degrees = np.sum(M, axis=1)
    elif fan == 'out':
        degrees = np.sum(M, axis=0)
    return degrees

# adapted from Masterthesis, Daniel Hjertholm


def get_expected_degrees_fixedDegrees(N, fan, len_source_pop, len_target_pop):
    N_d = len_target_pop if fan == 'in' else len_source_pop  # of driver nodes.
    N_p = len_source_pop if fan == 'in' else len_target_pop  # of pool nodes.
    expected_degree = N_d * N / float(N_p)
    expected = [expected_degree] * N_p
    return expected

# adapted from Masterthesis, Daniel Hjertholm


def get_expected_degrees_totalNumber(N, fan, len_source_pop, len_target_pop):
    expected_indegree = [N / float(len_target_pop)] * len_target_pop
    expected_outdegree = [N / float(len_source_pop)] * len_source_pop
    if fan == 'in':
        return expected_indegree
    elif fan == 'out':
        return expected_outdegree

# copied from Masterthesis, Daniel Hjertholm


def get_expected_degrees_bernoulli(p, fan, len_source_pop, len_target_pop):
    '''
    Calculate expected degree distribution.

    Degrees with expected number of observations below e_min are combined
    into larger bins.

    Return values
    -------------
        2D array. The four columns contain degree,
        expected number of observation, actual number observations, and
        the number of bins combined.
    '''

    n = len_source_pop if fan == 'in' else len_target_pop
    n_p = len_target_pop if fan == 'in' else len_source_pop
    mid = int(round(n * p))
    e_min = 5

    # Combine from front.
    data_front = []
    cumexp = 0.0
    bins_combined = 0
    for degree in range(mid):
        cumexp += scipy.stats.binom.pmf(degree, n, p) * n_p
        bins_combined += 1
        if cumexp < e_min:
            if degree == mid - 1:
                if len(data_front) == 0:
                    raise RuntimeWarning('Not enough data')
                deg, exp, obs, num = data_front[-1]
                data_front[-1] = (deg, exp + cumexp, obs,
                                  num + bins_combined)
            else:
                continue
        else:
            data_front.append((degree - bins_combined + 1, cumexp, 0,
                               bins_combined))
            cumexp = 0.0
            bins_combined = 0

    # Combine from back.
    data_back = []
    cumexp = 0.0
    bins_combined = 0
    for degree in reversed(range(mid, n + 1)):
        cumexp += scipy.stats.binom.pmf(degree, n, p) * n_p
        bins_combined += 1
        if cumexp < e_min:
            if degree == mid:
                if len(data_back) == 0:
                    raise RuntimeWarning('Not enough data')
                deg, exp, obs, num = data_back[-1]
                data_back[-1] = (degree, exp + cumexp, obs,
                                 num + bins_combined)
            else:
                continue
        else:
            data_back.append((degree, cumexp, 0, bins_combined))
            cumexp = 0.0
            bins_combined = 0
    data_back.reverse()

    expected = np.array(data_front + data_back)
    if fan == 'out':
        assert (sum(expected[:, 3]) == len_target_pop + 1)
    else:  # , 'Something is wrong'
        assert (sum(expected[:, 3]) == len_source_pop + 1)

    # np.hstack((np.asarray(data_front)[0], np.asarray(data_back)[0]))
    return expected

# adapted from Masterthesis, Daniel Hjertholm


def reset_seed(seed, nr_threads):
    '''
    Reset the simulator and seed the PRNGs.

    Parameters
    ----------
        seed: PRNG seed value.
    '''

    nest.ResetKernel()
    nest.SetKernelStatus({'local_num_threads': nr_threads, 'rng_seed': seed})

# copied from Masterthesis, Daniel Hjertholm


def chi_squared_check(degrees, expected, distribution=None):
    '''
    Create a single network and compare the resulting degree distribution
    with the expected distribution using Pearson's chi-squared GOF test.

    Parameters
    ----------
        seed   : PRNG seed value.
        control: Boolean value. If True, _generate_multinomial_degrees will
                 be used instead of _get_degrees.

    Return values
    -------------
        chi-squared statistic.
        p-value from chi-squared test.
    '''

    if distribution in ('pairwise_bernoulli', 'symmetric_pairwise_bernoulli'):
        observed = {}
        for degree in degrees:
            if degree not in observed:
                observed[degree] = 1
            else:
                observed[degree] += 1
        # Add observations to data structure, combining multiple observations
        # where necessary.
        expected[:, 2] = 0.0
        for row in expected:
            for i in range(int(row[3])):
                deg = int(row[0]) + i
                if deg in observed:
                    row[2] += observed[deg]

        # ddof: adjustment to the degrees of freedom. df = k-1-ddof
        return scipy.stats.chisquare(np.array(expected[:, 2]),
                                     np.array(expected[:, 1]))
    else:
        # ddof: adjustment to the degrees of freedom. df = k-1-ddof
        return scipy.stats.chisquare(np.array(degrees), np.array(expected))
