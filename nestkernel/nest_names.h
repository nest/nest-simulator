#ifndef NEST_NAMES_H
#define NEST_NAMES_H
/*
 *  nest_names.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "name.h"

namespace nest
{

  /**
   * This namespace contains global Name objects. These can be used in
   * Node::get_status and Node::set_status to make data exchange
   * more efficient and consistent. Creating a Name from a std::string
   * is in O(log n), for n the number of Names already created. Using
   * predefined names should make data exchange much more efficient.
   */
  namespace names
  {
    // Neuron parameters
    extern const Name V_m;        //!< Membrane potential
    extern const Name V_min;      //!< Absolute lower value for the membrane potential
    extern const Name E_L;        //!< Resting potential
    extern const Name I_e;        //!< Input current
    extern const Name I_L;        //!< Leak current
    extern const Name V_th;       //!< Threshold
    extern const Name V_reset;    //!< Reset potential
    extern const Name c_m;        //!< Capacity or specific capacitance
    extern const Name C_m;        //!< Membrane capacitance
    extern const Name tau_m;      //!< Membrane time constant
    extern const Name tau_syn;    //!< Synapse time constant
    extern const Name tau_syn_ex; //!< Excitatory synaptic time constant
    extern const Name tau_syn_in; //!< Inhibitory synaptic time constant
    extern const Name t_ref;      //!< Refractory period
    extern const Name t_ref_abs;  //!< Absolute refractory period
    extern const Name t_ref_tot;  //!< Total refractory period
    extern const Name t_ref_remaining; //!< Time remaining till end of refractory state
    extern const Name t_spike;    //!< Time of last spike
    extern const Name t_origin;   //!< Origin of a time-slice
    extern const Name t_lag;      //!< Lag within a time slice
    extern const Name E_ex;       //!< Excitatory reversal potential
    extern const Name E_in;       //!< Inhibitory reversal potential
    extern const Name g;          //!< Conductance
    extern const Name g_L;        //!< Leak conductance
    extern const Name g_ex;       //!< Excitatory conductance
    extern const Name dg_ex;      //!< Derivative of the excitatory conductance
    extern const Name g_in;       //!< inhibitory conductance
    extern const Name dg_in;      //!< Derivative of the inhibitory conductance
    extern const Name g_Na;       //!< Sodium conductance
    extern const Name E_ex;       //!< Excitatory reversal potential
    extern const Name E_in;       //!< Inhibitory reversal potential
    extern const Name I_ex;       //!< Excitatory synaptic input current
    extern const Name I_in;       //!< Inhibitory synaptic input current
    extern const Name E_Na;       //!< Sodium reversal potential
    extern const Name g_K;        //!< Potassium conductance
    extern const Name E_K;        //!< Potassium reversal potential
    extern const Name in_spikes;  //!< Number of arriving inhibitory spikes
    extern const Name ex_spikes;  //!< Number of arriving excitatory spikes
    extern const Name error;      //!< Indicates an error in a neuron

    // Related to ArchivingNode
    extern const Name tau_minus;  
    extern const Name tau_minus_triplet;
    extern const Name archiver_length;
    extern const Name clear;

    // Specific to precise timing neurons
    extern const Name Interpol_Order;      //!< Interpolation order
    extern const Name refractory_input;    //!< Spikes arriving during refractory period are counted

    // Specific to precise timing neurons (Brette 2007)
    extern const Name root_finding_epsilon; //!< Accuracy of the root of the polynomial
    extern const Name tau_lcm;              //!< Least common multiple of tau_m, tau_ex and tau_in
    extern const Name coeff_m;              //!< tau_lcm=coeff_m*tau_m
    extern const Name coeff_ex;             //!< tau_lcm=coeff_ex*tau_ex
    extern const Name coeff_in;             //!< tau_lcm=coeff_in*tau_in

    // For debugging
    extern const Name is_refractory; //!< Neuron is in refractory period

    // Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name tau_1;
    extern const Name tau_2;
    extern const Name alpha_1;
    extern const Name alpha_2;
    extern const Name omega;
    extern const Name V_th_alpha_1;
    extern const Name V_th_alpha_2;

    // Specific to Brette & Gerstner 2005
    extern const Name V_peak;        //!<  Spike detection threshold (Brette & Gerstner 2005)
    extern const Name a;
    extern const Name b;
    extern const Name w;
    extern const Name Delta_T;
    extern const Name tau_w;

    // Specific to Izhikevich 2003
    extern const Name c;
    extern const Name d;
    extern const Name U_m;
    extern const Name consistent_integration;

    // Tsodyks2_connection
    extern const Name dU; //!< Unit increment of the utilization for a facilitating synapse [0...1]
    extern const Name u;  //!< probability of release [0...1]
    extern const Name x; //!< current scaling factor of the synaptic weight [0...1]
    extern const Name tau_rec; //!< time constant for recovery (ms)
    extern const Name tau_fac; //!< facilitation time constant (ms)

    extern const Name A;
    extern const Name A_upper;
    extern const Name A_lower;
    extern const Name A_mean;
    extern const Name A_std;
    extern const Name U_upper;
    extern const Name U_lower;
    extern const Name U_mean;
    extern const Name U_std;
    extern const Name D_upper;
    extern const Name D_lower;
    extern const Name D_mean;
    extern const Name D_std;
    extern const Name F_upper;
    extern const Name F_lower;
    extern const Name F_mean;
    extern const Name F_std;
    extern const Name epoch;
    extern const Name success;
    extern const Name with_noise;



    // Same as aboce, but for property arrays.
    extern const Name dUs; //!< Unit increment of the utilization for a facilitating synapse [0...1]
    extern const Name us;  //!< probability of release [0...1]
    extern const Name xs; //!< current scaling factor of the synaptic weight [0...1]
    extern const Name tau_recs; //!< time constant for recovery (ms)
    extern const Name tau_facs; //!< facilitation time constant (ms)


    // Other adaptation
    extern const Name E_sfa;
    extern const Name E_rr;
    extern const Name g_sfa;
    extern const Name g_rr;
    extern const Name q_sfa;
    extern const Name q_rr;
    extern const Name tau_sfa;
    extern const Name tau_rr;

    // Specific to Hodgkin Huxley models
    extern const Name Act_m;
    extern const Name Act_h;
    extern const Name Inact_n;

    // Specific to mip_generator
    extern const Name mother_seed;
    extern const Name mother_rng;
    extern const Name p_copy;

    // Specific to correlation_detector
    extern const Name delta_tau;
    extern const Name tau_max;
    extern const Name histogram;
    extern const Name count_histogram;
    extern const Name Tstart;
    extern const Name Tstop;

    // Specific to current homeostasis
    extern const Name I_total;            //<- Total current
    extern const Name I_adapt;            //<- Goal of current homeostasis
    extern const Name I_std;              //<- Standard deviation of current distribution
    extern const Name PSC_adapt_step;     //<- PSC increment
    extern const Name PSC_Unit_amplitude; //<- Scaling of PSC

    // Specific to sli_neuron
    extern const Name update;         // Command to execute the neuron
    extern const Name update_node;    // Command to execute the neuron
    extern const Name calibrate;      // Command to calibrate the neuron
    extern const Name calibrate_node; // Command to calibrate the neuron
    extern const Name spike;          // true if the neuron spikes and false if not.

    // Specific to mirollo_strogatz_ps
    extern const Name phi;
    extern const Name I;
    extern const Name gamma;
    extern const Name phi_th;

    // Specific to stochastic neuron pp_psc_delta
    extern const Name c_1;
    extern const Name c_2;
    extern const Name c_3;
    extern const Name dead_time_random;  // Random dead time or fixed dead time
    extern const Name dead_time_shape;   // Shape parameter of the dead time distribution
    extern const Name with_reset; // Shall the pp_neuron reset after each spike?

    // Specific to Ginzburg neuron
    extern const Name S;     // Binary state (output) of neuron
    extern const Name h;     // Summed input to a neuron

    // Specific to iaf_chxk_2008 neuron
    extern const Name g_ahp;
    extern const Name tau_ahp;
    extern const Name E_ahp;

    // Specific to iaf_chs_2008 neuron
    extern const Name tau_reset;
    extern const Name tau_epsp;
    extern const Name V_epsp;
    extern const Name V_noise;
    extern const Name noise;

    // Specific to iaf_tum_2000
    extern const Name I_syn_ex;  // Total excitatory synaptic current
    extern const Name I_syn_in;  // Total inhibitory synaptic current

    // Names relating to GSL integration
    extern const Name gsl_error_tol;  // GSL integrator tolerance

    // Device parameters
    extern const Name origin;
    extern const Name start;
    extern const Name stop;
    extern const Name recordables;  //!< List of recordable state data

    // Generator parameters
    extern const Name individual_spike_trains;

    // Recorder parameters
    extern const Name withtime;
    extern const Name withgid;
    extern const Name withpath;
    extern const Name withweight;
    extern const Name precise_times;
    extern const Name time_in_steps;
    extern const Name to_file;
    extern const Name to_screen;
    extern const Name to_memory;
    extern const Name to_accumulator;
    extern const Name record_to;
    extern const Name file;
    extern const Name screen;
    extern const Name memory;
    extern const Name accumulator;
    extern const Name file_extension;
    extern const Name precision;
    extern const Name scientific;
    extern const Name binary;
    extern const Name fbuffer_size;
    extern const Name flush_records;
    extern const Name close_after_simulate;
    extern const Name flush_after_simulate;
    extern const Name close_on_reset;
    extern const Name filename;
    extern const Name filenames;
    extern const Name record_from;

    extern const Name senders;
    extern const Name times;
    extern const Name offsets;
    extern const Name n_events;

    extern const Name interval;
    extern const Name events;
    extern const Name potentials;
    extern const Name currents;
    extern const Name spike_times;
    extern const Name exc_conductance;
    extern const Name inh_conductance;

    // Connection parameters
    extern const Name source;
    extern const Name target;
    extern const Name targets;
    extern const Name weight;
    extern const Name weights;
    extern const Name delay;
    extern const Name delays;
    extern const Name receptor_type;
    extern const Name receptor_types;
    extern const Name rport;
    extern const Name rports;
    extern const Name port;
    extern const Name target_thread;
    extern const Name synapse_model;
    extern const Name synapse_modelid;

    // Specific to sinusoidally modulated generators
    extern const Name dc;
    extern const Name ac;
    extern const Name freq;
    extern const Name order;
 
    // Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name amplitude;
    extern const Name phase;
    extern const Name frequency;
    extern const Name rate;
    extern const Name n_proc; // Number of component processes of generator
    extern const Name dead_time;
    extern const Name gamma_shape;

    // Miscellaneous parameters
    extern const Name label;
    extern const Name mean;
    extern const Name std;
    extern const Name rms; // Root mean square
    extern const Name dt;
    extern const Name offset;

    // Node parameters
    extern const Name global_id;
    extern const Name model;
    extern const Name frozen;
    extern const Name address;
    extern const Name local_id;
    extern const Name parent;
    extern const Name state;
    extern const Name thread;
    extern const Name vp;
    extern const Name local;

    // Parameters for MUSIC devices
    extern const Name connection_count;
    extern const Name index_map;
    extern const Name music_channel;
    extern const Name port_name;
    extern const Name port_width;
    extern const Name registered;
    extern const Name published;

    extern const Name theta; // Did not compile without (theta neuron problem)

    // Node types
    extern const Name type;
    extern const Name structure;
    extern const Name neuron;
    extern const Name stimulator;
    extern const Name recorder;
    extern const Name synapse;
    extern const Name other;
  }
}

#endif
