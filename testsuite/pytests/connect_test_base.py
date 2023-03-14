# -*- coding: utf-8 -*-
#
# connect_test_base.py
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
import unittest

try:
    from mpi4py import MPI
    haveMPI4Py = True
except ImportError:
    haveMPI4Py = False


class ConnectTestBase(unittest.TestCase):
    """
    Base class for connection tests.

    This class provides overall setup methods and a range of tests
    that apply to all connection rules. The tests are not to be
    run from this class, but from derived classes, one per rule.
    """

    # Setting default parameter. These parameter might be overwritten
    # by the classes testing one specific rule.
    # We force subclassing by setting rule to None here and provide default
    # values for other parameters.
    rule = None
    conn_dict = {'rule': rule}
    # sizes of populations
    N1 = 6
    N2 = 6
    # time step
    dt = 0.1
    # default params
    w0 = 1.0
    d0 = 1.0
    r0 = 0
    syn0 = 'static_synapse'
    # parameter for test of distributed parameter
    pval = 0.05  # minimum p-value to pass kolmogorov smirnov test
    # number of threads
    nr_threads = 2

    # for now only tests if a multi-thread connect is successful, not whether the threading is actually used
    def setUp(self):
        nest.ResetKernel()
        nest.local_num_threads = self.nr_threads

    def setUpNetwork(self, conn_dict=None, syn_dict=None, N1=None, N2=None):
        if N1 is None:
            N1 = self.N1
        if N2 is None:
            N2 = self.N2
        self.pop1 = nest.Create('iaf_psc_alpha', N1)
        self.pop2 = nest.Create('iaf_psc_alpha', N2)
        nest.set_verbosity('M_FATAL')
        nest.Connect(self.pop1, self.pop2, conn_dict, syn_dict)

    def setUpNetworkOnePop(self, conn_dict=None, syn_dict=None, N=None):
        if N is None:
            N = self.N1
        self.pop = nest.Create('iaf_psc_alpha', N)
        nest.set_verbosity('M_FATAL')
        nest.Connect(self.pop, self.pop, conn_dict, syn_dict)

    def testWeightSetting(self):
        # test if weights are set correctly

        # one weight for all connections
        w0 = 0.351
        label = 'weight'
        syn_params = {label: w0}
        check_synapse([label], [syn_params['weight']], syn_params, self)

    def testDelaySetting(self):
        # test if delays are set correctly

        # one delay for all connections
        d0 = 0.275
        syn_params = {'delay': d0}
        self.setUpNetwork(self.conn_dict, syn_params)
        connections = nest.GetConnections(self.pop1, self.pop2)
        nest_delays = connections.get('delay')
        # all delays need to be equal
        self.assertTrue(all_equal(nest_delays))
        # delay (rounded) needs to equal the delay that was put in
        self.assertTrue(abs(d0 - nest_delays[0]) < self.dt)

    def testRPortSetting(self):
        neuron_model = 'iaf_psc_exp_multisynapse'
        neuron_dict = {'tau_syn': [0.5, 0.7]}
        rtype = 2
        self.pop1 = nest.Create(neuron_model, self.N1, neuron_dict)
        self.pop2 = nest.Create(neuron_model, self.N2, neuron_dict)
        syn_params = {'synapse_model': 'static_synapse',
                      'receptor_type': rtype}
        nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
        conns = nest.GetConnections(self.pop1, self.pop2)
        ports = conns.get('receptor')
        self.assertTrue(all_equal(ports))
        self.assertTrue(ports[0] == rtype)

    def testSynapseSetting(self):
        nest.CopyModel("static_synapse", 'test_syn', {'receptor_type': 0})
        syn_params = {'synapse_model': 'test_syn'}
        self.setUpNetwork(self.conn_dict, syn_params)
        conns = nest.GetConnections(self.pop1, self.pop2)
        syns = conns.get('synapse_model')
        self.assertTrue(all_equal(syns))
        self.assertTrue(syns[0] == syn_params['synapse_model'])

    # tested on each mpi process separately
    def testDefaultParams(self):
        self.setUpNetwork(self.conn_dict)
        conns = nest.GetConnections(self.pop1, self.pop2)
        self.assertTrue(all(x == self.w0 for x in conns.get('weight')))
        self.assertTrue(all(x == self.d0 for x in conns.get('delay')))
        self.assertTrue(all(x == self.r0 for x in conns.get('receptor')))
        self.assertTrue(all(x == self.syn0 for
                            x in conns.get('synapse_model')))

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()

        # test that autapses exist
        conn_params['allow_autapses'] = True
        self.pop1 = nest.Create('iaf_psc_alpha', self.N1)
        nest.Connect(self.pop1, self.pop1, conn_params)
        # make sure all connections do exist
        M = get_connectivity_matrix(self.pop1, self.pop1)
        mpi_assert(np.diag(M), np.ones(self.N1), self)

    def testAutapsesFalse(self):
        conn_params = self.conn_dict.copy()

        # test that autapses were excluded
        conn_params['allow_autapses'] = False
        self.pop1 = nest.Create('iaf_psc_alpha', self.N1)
        nest.Connect(self.pop1, self.pop1, conn_params)
        # make sure all connections do exist
        M = get_connectivity_matrix(self.pop1, self.pop1)
        mpi_assert(np.diag(M), np.zeros(self.N1), self)

    def testHtSynapse(self):
        params = ['P', 'delta_P']
        values = [0.987, 0.362]
        syn_params = {'synapse_model': 'ht_synapse'}
        check_synapse(params, values, syn_params, self)

    def testQuantalStpSynapse(self):
        params = ['U', 'tau_fac', 'tau_rec', 'u', 'a', 'n']
        values = [0.679, 8.45, 746.2, 0.498, 10, 5]
        syn_params = {'synapse_model': 'quantal_stp_synapse'}
        check_synapse(params, values, syn_params, self)

    def testStdpFacetshwSynapseHom(self):
        params = ['a_acausal', 'a_causal', 'a_thresh_th', 'a_thresh_tl',
                  'next_readout_time'
                  ]
        values = [0.162, 0.263, 20.46, 19.83, 0.1]
        syn_params = {'synapse_model': 'stdp_facetshw_synapse_hom'}
        check_synapse(params, values, syn_params, self)

    def testStdpPlSynapseHom(self):
        params = ['Kplus']
        values = [0.173]
        syn_params = {'synapse_model': 'stdp_pl_synapse_hom'}
        check_synapse(params, values, syn_params, self)

    def testStdpSynapseHom(self):
        params = ['Kplus']
        values = [0.382]
        syn_params = {'synapse_model': 'stdp_synapse_hom'}
        check_synapse(params, values, syn_params, self)

    def testStdpSynapse(self):
        params = ['Wmax', 'alpha', 'lambda', 'mu_minus', 'mu_plus', 'tau_plus']
        values = [98.34, 0.945, 0.02, 0.945, 1.26, 19.73]
        syn_params = {'synapse_model': 'stdp_synapse'}
        check_synapse(params, values, syn_params, self)

    def testTsodyks2Synapse(self):
        params = ['U', 'tau_fac', 'tau_rec', 'u', 'x']
        values = [0.362, 0.152, 789.2, 0.683, 0.945]
        syn_params = {'synapse_model': 'tsodyks2_synapse'}
        check_synapse(params, values, syn_params, self)

    def testTsodyksSynapse(self):
        params = ['U', 'tau_fac', 'tau_psc', 'tau_rec', 'x', 'y', 'u']
        values = [0.452, 0.263, 2.56, 801.34, 0.567, 0.376, 0.102]
        syn_params = {'synapse_model': 'tsodyks_synapse'}
        check_synapse(params, values, syn_params, self)

    def testStdpDopamineSynapse(self):
        # ResetKernel() since parameter setting not thread save for this
        # synapse type
        nest.ResetKernel()
        vol = nest.Create('volume_transmitter')
        nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol.get('global_id')})
        params = ['c', 'n']
        values = [0.153, 0.365]
        syn_params = {'synapse_model': 'stdp_dopamine_synapse'}
        check_synapse(params, values, syn_params, self)

    def testRPortAllSynapses(self):
        syns = ['cont_delay_synapse', 'ht_synapse', 'quantal_stp_synapse',
                'static_synapse_hom_w', 'stdp_dopamine_synapse',
                'stdp_facetshw_synapse_hom', 'stdp_pl_synapse_hom',
                'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                'tsodyks_synapse'
                ]
        syn_params = {'receptor_type': 1}

        for i, syn in enumerate(syns):
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol.get('global_id')})
            syn_params['synapse_model'] = syn
            self.pop1 = nest.Create('iaf_psc_exp_multisynapse', self.N1, {
                                       'tau_syn': [0.2, 0.5]})
            self.pop2 = nest.Create('iaf_psc_exp_multisynapse', self.N2, {
                                       'tau_syn': [0.2, 0.5]})
            nest.Connect(self.pop1, self.pop2, self.conn_dict, syn_params)
            conns = nest.GetConnections(self.pop1, self.pop2)
            conn_params = conns.get('receptor')
            self.assertTrue(all_equal(conn_params))
            self.assertTrue(conn_params[0] == syn_params['receptor_type'])
            self.setUp()

    def testWeightAllSynapses(self):
        # test all synapses apart from static_synapse_hom_w where weight is not
        # settable
        syns = ['cont_delay_synapse', 'ht_synapse', 'quantal_stp_synapse',
                'stdp_dopamine_synapse',
                'stdp_facetshw_synapse_hom',
                'stdp_pl_synapse_hom',
                'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                'tsodyks_synapse'
                ]
        syn_params = {'weight': 0.372}

        for syn in syns:
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol.get('global_id')})
            syn_params['synapse_model'] = syn
            check_synapse(
                ['weight'], [syn_params['weight']], syn_params, self)
            self.setUp()

    def testDelayAllSynapses(self):
        syns = ['cont_delay_synapse',
                'ht_synapse', 'quantal_stp_synapse',
                'static_synapse_hom_w',
                'stdp_dopamine_synapse',
                'stdp_facetshw_synapse_hom', 'stdp_pl_synapse_hom',
                'stdp_synapse_hom', 'stdp_synapse', 'tsodyks2_synapse',
                'tsodyks_synapse'
                ]
        syn_params = {'delay': 0.4}

        for syn in syns:
            if syn == 'stdp_dopamine_synapse':
                vol = nest.Create('volume_transmitter')
                nest.SetDefaults('stdp_dopamine_synapse', {'vt': vol.get('global_id')})
            syn_params['synapse_model'] = syn
            check_synapse(
                ['delay'], [syn_params['delay']], syn_params, self)
            self.setUp()


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
    nest.local_num_threads = nr_threads
    nest.rng_seed = seed

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
