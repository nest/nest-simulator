Glossary
==============

Common abbreviations in NEST
------------------------------
.. glossary::

 iaf
   integrate and fire

 gif
   generalized integrate and fire

 cond
   conductance-based

 psc
   post synaptic current (current-based)

 hh
   hodgkin huxley

 rng
   random number generator

 wfr
   waveform relaxation method

 aeif
   adaptive exponential integrate and fire

 ht
   hill and tononi

 pp
   point process

 in
   inhibitory

 ex
   excitatory

 MAM
   multi-area model

 mpi
   message passing interface

 stdp
   spike-timing dependent plasticity synapse

 st
   short term plasticity

 vp
   virtual process

Physical units and variable names used for NEST parameters
-------------------------------------------------------------

.. note::

   all parameters listed here are defined as `type double` in NEST

.. glossary::

 **time**
    milliseconds `ms`

 tau_m
    Membrane time constant in ms

 t_ref
    Duration of refractory period in ms

 t_spike
    point in time of last spike in

 **capacitance**
    picofarads `pF`

 C_m
    Capacitance of the membrane in pF

 **current**
    picoamperes `pA`

 I_e
    Constant input current in pA.

 **conductance**
    nanosiemens `nS`

   g_L
    Leak conductance in nS

   g_K
    Potassium peak conductance in nS.

   g_Na
    Sodium peak conductance in nS.

 **spike rates**
    spikes/s

 **modulation frequencies**
    herz `Hz`

 frequency
    frequncy in Hz

 **voltage**
   millivolts `mV`

 V_m
   Membrane potential in mV

 E_L
   Resting membrane potential in mV.

 V_th
   Spike threshold in mV.

 V_reset double
   Reset potential of the membrane in mV.

 V_min
   Absolute lower value for the membrane potential in mV

 E_ex
   Excitatory reversal potential in mV.

 E_in
    Inhibitory reversal potential in mV.

 E_Na
   Sodium reversal potential in mV.

 E_K
   Potassium reversal potential in mV.


connection rules

one-to-one
all-to-all
fixed-indegree
fixed-outdegree
fixed-total-number
pairwise-bernoulli

syn_spec - dictionary for synapse parameters
conn_dict- dictionary for connection rules

Simulation

simulate
setstatus
getstatus
connect
create

nodes - neurons, devices

connections - synapses
weight -
delay -
syn_spec



    GetDefaults(model)

    Return a dictionary with the default parameters of the given model, specified by a string.

    SetDefaults(model, params)

    Set the default parameters of the given model to the values specified in the params dictionary.

    CopyModel(existing, new, params=None)

    Create a new model by copying an existing one. Default parameters can be given as params, or else are taken from existing.




    ResetKernel()

    Reset the simulation kernel. This will destroy the network as well as all custom models created with CopyModel(). The parameters of built-in models are reset to their defaults. Calling this function is equivalent to restarting NEST.

    ResetNetwork()

    Reset all nodes and connections to the defaults of their respective model.


    GetKernelStatus(keys=none)

    Obtain parameters of the simulation kernel. Returns:
        Parameter dictionary if called without argument
        Single parameter value if called with single parameter name
        List of parameter values if called with list of parameter names
        Set parameters for the simulation kernel.

Simulation kernel

Simulate - nestkernel/nestmodule.cpp
ResumeSimulation - n/a
ResetKernel - "
ResetNetwork - "
reset - lib/sli/sli-init.sli

kernel - nestkernel/kernel_manager.h
SetStatus - nestkernel/nestmodule.cpp # reference sli in  example
GetStatus
ShowStatus

MemoryInfo
memory thisjob

model_manager.h
kernel_manager.h
nestmodule.cpp

kernel status dictionary

  The following parameters are available in the kernel status dictionary.

  Time and resolution
  resolution	 doubletype 	- The resolution of the simulation (in ms)
  time	 doubletype 	- The current simulation time
  to_do	 integertype	- The number of steps yet to be simulated (read only)
  max_delay	 doubletype 	- The maximum delay in the network
  min_delay	 doubletype 	- The minimum delay in the network
  ms_per_tic	 doubletype 	- The number of milliseconds per tic
  tics_per_ms	 doubletype 	- The number of tics per millisecond
  tics_per_step	 integertype	- The number of tics per simulation time step
  T_max	 doubletype 	- The largest representable time value (read only)
  T_min	 doubletype 	- The smallest representable time value (read only)

  Parallel processing
  total_num_virtual_procs  integertype	- The total number of virtual processes
  local_num_threads	 integertype	- The local number of threads
  num_processes	 integertype	- The number of MPI processes (read only)
  num_rec_processes	 integertype	- The number of MPI processes reserved for recording spikes
  num_sim_processes	 integertype	- The number of MPI processes reserved for simulating neurons
  off_grid_spiking	 booltype  	- Whether to transmit precise spike times in MPI
  communication (read only)

  Random number generators
  grng_seed	 integertype	- Seed for global random number generator used
  synchronously by all virtual processes to
  create, e.g., fixed fan-out connections
  (write only).
  rng_seeds	 arraytype  	- Seeds for the per-virtual-process random
  number generators used for most purposes.
  Array with one integer per virtual process,
  all must be unique and differ from
  grng_seed (write only).

  Output
  data_path	 stringtype 	- A path, where all data is written to
  (default is the current directory)
  data_prefix	 stringtype 	- A common prefix for all data files
  overwrite_files	 booltype  	- Whether to overwrite existing data files
  print_time	 booltype  	- Whether to print progress information during the simulation

  Network information
  network_size	 integertype	- The number of nodes in the network (read only)
  num_connections	 integertype	- The number of connections in the network
  (read only, local only)

  Waveform relaxation method (wfr)
  use_wfr	 booltype  	- Whether to use waveform relaxation method
  wfr_comm_interval	 doubletype 	- Desired waveform relaxation communication interval
  wfr_tol	 doubletype 	- Convergence tolerance of waveform relaxation method
  wfr_max_iterations	 integertype	- Maximal number of iterations used for waveform relaxation
  wfr_interpolation_order  integertype	- Interpolation order of polynomial used in wfr iterations

  Miscellaneous
  dict_miss_is_error	 booltype  	- Whether missed dictionary entries are treated as errors

tau_syn_X are synaptic time constants. Depending on which neuron model you are
using these time constants either determine time course of the postsynaptic
current (..._psc_... models) or the change in conductance (..._cond_...) models.
The ex and in indicate the time constants for excitatory (positive weight) and
inhibitory (negative weight) synapses.

Models with a single time constant each for excitatory and inhibitory neurons
have either exponential synapes (..._..._exp models), i.e., PSC or conductance
jump on spike arrival and then decay with the corresponding time constant, or
alpha-shape synapses (..._..._alpha models), which have a time course ~ t exp(-t/tau_syn_...).
