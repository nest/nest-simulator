# -*- coding: utf-8 -*-
#
# tcd_nest.py
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

# ConnPlotter --- A Tool to Generate Connectivity Pattern Matrices

"""
Interface routines to extract synapse information from NEST.

This file provides the interface to NEST required to plot effective
kernel connectivity as total charge deposited (TCD) as a function of
mean membrane potential.

In order to use TCD plots, you need to create an instance of class
SynapsesNEST. The constructor will import NEST to obtain all necessary
information. TCD can then be obtained by calling the generated object.

NB: At present, TCD is supported only for the ht_model. NMDA charge
    deposition is based on steady-state value for open channels at given
    voltage.
"""

# ----------------------------------------------------------------------------

import numpy as np

__all__ = ['TCD_NEST']

# ----------------------------------------------------------------------------


class TCD(object):
    """
    Access total charge deposited (TCD) information for NEST neurons.

    Create one instance of this class and call it to obtain charge
    information.

    NB: The constructor for this class imports NEST.

    NB: Currently, only ht_model is supported, with synapse types
        AMPA, NMDA, GABA_A, GABA_B.
    """

    # ------------------------------------------------------------------------

    def __init__(self, modelList):

        """
        Create TCD computer for given modelList.
        The constructor instantiates NEST, including a call to
        ResetKernel() and instantiates all models in modelList.
        From all models derived from ht_model, synapse information
        is extracted and stored. Afterward, ResetKernel() is called
        once more.

        modelList: tuples of (parent, model, dict)

        Note: nest must have been imported before and all necessary modules
              loaded.
        """
        import nest
        nest.ResetKernel()

        # keep "list" over all models derived from ht_neuron
        ht_kids = set(["ht_neuron"])

        for parent, model, props in modelList:
            if parent in ht_kids and model not in ht_kids:
                nest.CopyModel(parent, model, props)
                ht_kids.add(model)

        # ht_kids now contains all models derived from ht_neuron
        # We collect in _tcd_info a mapping from (targetmodel, synapstype)
        # to an object containing all required information for TCD computation.
        self._tcd_info = {}
        for mod in ht_kids:
            props = nest.GetDefaults(mod)
            for syn in ['AMPA', 'GABA_A', 'GABA_B']:
                self._tcd_info[(mod, syn)] = self._TcdBeta(syn, props)
            self._tcd_info[(mod, 'NMDA')] = self._TcdNMDA(props)

        # delete models we created
        nest.ResetKernel()

    # ------------------------------------------------------------------------

    def __call__(self, syn_type, target, V):
        """
        Return total charge deposited by a single spike through
        synapse of syn_type with syn_wght onto target, given that
        target has membrane potential V.

        Arguments:
        syn_type   synapse type (string: AMPA, NMDA, GABA_A, GABA_B)
        target     name of target neuron model (string)
        V          membrane potential (double)

        Returns:
        charge (double)
        """

        return self._tcd_info[(target, syn_type)](V)

    # ------------------------------------------------------------------------

    class _TcdBeta(object):
        """
        Class representing plain beta-function synapse model.
        """

        def __init__(self, syn, props):
            """
            syn is name of synapse type.
            props is property dictionary of ht_neuron.
            """
            td = props['tau_decay_' + syn]  # decay time
            tr = props['tau_rise_' + syn]  # rise time
            # integral over g(t)
            self._int_g = (props['g_peak_' + syn] * (td - tr) /
                           ((tr / td) ** (tr / (td - tr)) -
                            (tr / td) ** (td / (td - tr))))
            self._e_rev = props['E_rev_' + syn]

        def __call__(self, V):
            """
            V is membrane potential.
            """
            return -self._int_g * (V - self._e_rev)

        def __str__(self):
            return "_int_g = %f, _e_rev = %f" % (self._int_g, self._e_rev)

    # ------------------------------------------------------------------------

    class _TcdNMDA(object):
        """
        Class representing NMDA synapse model in ht_neuron.

        Note: NMDA charge deposition is based on steady-state value
              for open channels at given voltage.
        """

        def __init__(self, props):
            """
            props is property dictionary of ht_neuron.
            """
            td = props['tau_decay_NMDA']  # decay time
            tr = props['tau_rise_NMDA']  # rise time
            # integral over g(t)
            self._int_g = (props['g_peak_NMDA'] * (td - tr) /
                           ((tr / td) ** (tr / (td - tr)) -
                            (tr / td) ** (td / (td - tr))))
            self._e_rev = props['E_rev_NMDA']
            self._v_act = props['V_act_NMDA']
            self._s_act = props['S_act_NMDA']

        def __call__(self, V):
            """
            V is membrane potential.
            """
            return (-self._int_g * (V - self._e_rev) /
                    (1. + np.exp((self._v_act - V) / self._s_act)))

        def __str__(self):
            return "_int_g = %f, _e_rev = %f, _v_act = %f, _s_act = %f" \
                   % (self._int_g, self._e_rev, self._v_act, self._s_act)


# ----------------------------------------------------------------------------

if __name__ == '__main__':

    import matplotlib.pyplot as plt
    import sys

    sys.path.append('/Users/plesser/Projects/hill-model/scripts')

    import ht_def_new_sq
    import ht_params

    htl, htc, htm = ht_def_new_sq.hill_tononi(ht_params.Params)

    tcd = TCD(htm)

    v = np.linspace(-90, 0, 100)
    syns = ['AMPA', 'NMDA', 'GABA_A', 'GABA_B']

    for s in syns:
        g = np.array([tcd(s, 'Relay', vm) for vm in v])
        plt.plot(v, g)

    plt.legend(syns)
    plt.show()
