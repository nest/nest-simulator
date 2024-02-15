# -*- coding: utf-8 -*-
#
# test_mini_brunel_ps.py
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


from mpi_test_wrapper import MPITestAssertEqual


@MPITestAssertEqual([1, 2, 4])
def test_mini_brunel_ps():
    """
    Confirm that downscaled Brunel net with precise neurons is invariant under number of MPI ranks.
    """

    import nest

    nest.ResetKernel()

    nest.set(total_num_virtual_procs=4, overwrite_files=True)

    # Model parameters
    NE = 1000  # number of excitatory neurons
    NI = 250  # number of inhibitory neurons
    CE = 100  # number of excitatory synapses per neuron
    CI = 250  # number of inhibitory synapses per neuron
    N_rec = 10  # number of (excitatory) neurons to record
    D = 1.5  # synaptic delay, all connections [ms]
    JE = 0.1  # peak of EPSP [mV]
    eta = 2.0  # external rate relative to threshold rate
    g = 5.0  # ratio inhibitory weight/excitatory weight
    JI = -g * JE  # peak of IPSP [mV]

    neuron_params = {
        "tau_m": 20,  # membrance time constant [ms]
        "t_ref": 2.0,  # refractory period [ms]
        "C_m": 250.0,  # membrane capacitance [pF]
        "E_L": 0.0,  # resting membrane potential [mV]
        "V_th": 20.0,  # threshold potential [mV]
        "V_reset": 0.0,  # reset potential [mV]
    }

    # Threshold rate; the external rate needed for a neuron to reach
    # threshold in absence of feedback
    nu_thresh = neuron_params["V_th"] / (JE * CE * neuron_params["tau_m"])

    # External firing rate; firing rate of a neuron in the external population
    nu_ext = eta * nu_thresh

    # Build network
    enodes = nest.Create("iaf_psc_delta_ps", NE, params=neuron_params)
    inodes = nest.Create("iaf_psc_delta_ps", NI, params=neuron_params)
    ext = nest.Create("poisson_generator_ps", 1, params={"rate": nu_ext * CE * 1000.0})
    srec = nest.Create(
        "spike_recorder",
        1,
        params={
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
            "record_to": "ascii",
            "time_in_steps": True,
        },
    )

    nest.CopyModel("static_synapse", "esyn", params={"weight": JE, "delay": D})
    nest.CopyModel("static_synapse", "isyn", params={"weight": JI, "delay": D})

    nest.Connect(ext, enodes, conn_spec="all_to_all", syn_spec="esyn")
    nest.Connect(ext, inodes, conn_spec="all_to_all", syn_spec="esyn")
    nest.Connect(enodes[:N_rec], srec)
    nest.Connect(
        enodes,
        enodes + inodes,
        conn_spec={"rule": "fixed_indegree", "indegree": CE, "allow_autapses": False, "allow_multapses": True},
        syn_spec="esyn",
    )
    nest.Connect(
        inodes,
        enodes + inodes,
        conn_spec={"rule": "fixed_indegree", "indegree": CI, "allow_autapses": False, "allow_multapses": True},
        syn_spec="isyn",
    )

    # Simulate network
    nest.Simulate(400)

    # Uncomment next line to provoke test failure
    # nest.Simulate(200 if -nest.num_processes == 1 else 400)
