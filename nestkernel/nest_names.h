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

#ifndef NEST_NAMES_H
#define NEST_NAMES_H

// Includes from sli:
#include "name.h"

namespace nest
{

/**
 * This namespace contains global Name objects. These can be used in
 * Node::get_status and Node::set_status to make data exchange more
 * efficient and consistent. Creating a Name from a std::string is in
 * O(log n), for n the number of Names already created. Using
 * predefined names makes data exchange much more efficient as it
 * uses integer comparisons instead of string comparisons internally.
 *
 * The Name declarations below and the definitions in nest_names.cpp
 * are sorted alphabetically with lower case letters preceding upper
 * case letters. The ordering of the names has to be the same in both
 * this file and the .cpp file.
 *
 * See testsuite/unittests/test_unused_names.py for a test that checks
 * if a) Name declarations and definitions are consistent
 *    b) all Name objects defined are also actually used.
 */
namespace names
{

extern const Name a;
extern const Name a_acausal;
extern const Name a_causal;
extern const Name A_minus;
extern const Name A_plus;
extern const Name a_thresh_th;
extern const Name a_thresh_tl;
extern const Name acceptable_latency;
extern const Name accumulator;
extern const Name Act_h;
extern const Name Act_m;
extern const Name activity;
extern const Name ahp_bug;
extern const Name allow_offgrid_spikes;
extern const Name allow_offgrid_times;
extern const Name alpha;
extern const Name alpha_1;
extern const Name alpha_2;
extern const Name Aminus;
extern const Name Aminus_triplet;
extern const Name AMPA;
<<<<<<< HEAD
extern const Name amplitude;
extern const Name amplitude_times;
extern const Name amplitude_values;
extern const Name Aplus;
extern const Name Aplus_triplet;
extern const Name archiver_length;
extern const Name autapses;
extern const Name available;

extern const Name b;
extern const Name beta;
extern const Name beta_Ca;
extern const Name binary;

extern const Name c;
extern const Name c_1;
extern const Name c_2;
extern const Name c_3;
extern const Name C_m;
extern const Name Ca;
extern const Name calibrate;
extern const Name calibrate_node;
extern const Name capacity;
extern const Name clear;
extern const Name close_after_simulate;
extern const Name close_on_reset;
extern const Name configbit_0;
extern const Name configbit_1;
extern const Name connection_count;
extern const Name consistent_integration;
extern const Name continuous;
extern const Name count_covariance;
extern const Name count_histogram;
extern const Name covariance;
extern const Name currents;
extern const Name customdict;

extern const Name d;
extern const Name data;
extern const Name data_path;
extern const Name data_prefix;
extern const Name dead_time;
extern const Name dead_time_random;
extern const Name dead_time_shape;
extern const Name delay;
extern const Name delays;
extern const Name deliver_interval;
extern const Name delta_P;
extern const Name Delta_T;
extern const Name delta_tau;
extern const Name delta_u;
extern const Name Delta_V;
extern const Name dg;
extern const Name dg_ex;
extern const Name dg_in;
extern const Name dI_syn_ex;
extern const Name dI_syn_in;
extern const Name dict_miss_is_error;
extern const Name diffusion_factor;
extern const Name distal_curr;
extern const Name distal_exc;
extern const Name distal_inh;
extern const Name distribution;
extern const Name drift_factor;
extern const Name driver_readout_time;
extern const Name dt;
extern const Name dU;

extern const Name E_ahp;
extern const Name E_ex;
extern const Name E_in;
extern const Name E_K;
extern const Name E_L;
extern const Name E_Na;
extern const Name E_rev;
extern const Name E_rev_AMPA;
extern const Name E_rev_GABA_A;
extern const Name E_rev_GABA_B;
extern const Name E_rev_h;
extern const Name E_rev_KNa;
extern const Name E_rev_NaP;
extern const Name E_rev_NMDA;
extern const Name E_rev_T;
extern const Name E_rr;
extern const Name E_sfa;
extern const Name element_type;
extern const Name elementsize;
extern const Name eps;
extern const Name equilibrate;
extern const Name error;
extern const Name eta;
extern const Name events;
extern const Name ex_spikes;

extern const Name fbuffer_size;
extern const Name file;
extern const Name file_extension;
extern const Name filenames;
extern const Name flush_after_simulate;
extern const Name flush_records;
extern const Name frequency;
extern const Name frozen;

extern const Name g;
extern const Name g_ahp;
extern const Name g_AMPA;
extern const Name g_ex;
extern const Name g_ext_ex;
extern const Name g_ext_in;
extern const Name g_GABA_A;
extern const Name g_GABA_B;
extern const Name g_in;
extern const Name g_K;
extern const Name g_KL;
extern const Name g_Kv1;
extern const Name g_Kv3;
extern const Name g_L;
extern const Name g_Na;
extern const Name g_NaL;
extern const Name g_NMDA;
extern const Name g_pd;
extern const Name g_peak_AMPA;
extern const Name g_peak_GABA_A;
extern const Name g_peak_GABA_B;
extern const Name g_peak_h;
extern const Name g_peak_KNa;
extern const Name g_peak_NaP;
extern const Name g_peak_NMDA;
extern const Name g_peak_T;
extern const Name g_rr;
extern const Name g_sfa;
extern const Name g_sp;
=======
extern const Name amplitude;        //!< Signal modulation amplitude
extern const Name amplitude_times;  //!< Used by sted_current_generator
extern const Name amplitude_values; //!< Used by sted_current_generator
extern const Name Aplus;            //!< Used by stdp_connection_facetshw_hom
extern const Name Aplus_triplet;    //!< Used by stdp_connection_facetshw_hom
extern const Name archiver_length;  //!< used for ArchivingNode
extern const Name available;        //!< model paramater
extern const Name autapses;         //!< Connectivity-related

extern const Name b;    //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
extern const Name beta; //!< Specific to amat2_*
extern const Name
  beta_Ca; //!< Increment in calcium concentration with each spike
extern const Name binary; //!< Recorder parameter

extern const Name c;         //!< Specific to Izhikevich 2003
extern const Name c_1;       //!< Specific to stochastic neuron pp_psc_delta
extern const Name c_2;       //!< Specific to stochastic neuron pp_psc_delta
extern const Name c_3;       //!< Specific to stochastic neuron pp_psc_delta
extern const Name C_m;       //!< Membrane capacitance
extern const Name Ca;        //!< Calcium concentration
extern const Name calibrate; //!< Command to calibrate the neuron (sli_neuron)
extern const Name
  calibrate_node;           //!< Command to calibrate the neuron (sli_neuron)
extern const Name capacity; //!< model paramater
extern const Name clear;    //!< used for ArchivingNode
extern const Name close_after_simulate; //!< Recorder parameter
extern const Name close_on_reset;       //!< Recorder parameter
extern const Name
  coeff_ex; //!< tau_lcm=coeff_ex*tau_ex (precise timing neurons (Brette 2007))
extern const Name
  coeff_in; //!< tau_lcm=coeff_in*tau_in (precise timing neurons (Brette 2007))
extern const Name
  coeff_m; //!< tau_lcm=coeff_m*tau_m (precise timing neurons (Brette 2007))
extern const Name configbit_0;      //!< Used in stdp_connection_facetshw_hom
extern const Name configbit_1;      //!< Used in stdp_connection_facetshw_hom
extern const Name connection_count; //!< Parameters for MUSIC devices
extern const Name consistent_integration; //!< Specific to Izhikevich 2003
extern const Name continuous;             //!< Parameter for MSP dynamics
extern const Name count_covariance; //!< Specific to correlomatrix_detector
extern const Name count_histogram;  //!< Specific to correlation_detector
extern const Name covariance;       //!< Specific to correlomatrix_detector
extern const Name currents;         //!< Recorder parameter
extern const Name customdict;       //!< Used by Subnet

extern const Name d; //!< Specific to Izhikevich 2003
extern const Name D_lower;
extern const Name D_mean;
extern const Name D_std;
extern const Name D_upper;
extern const Name data;        //!< Used in music_message_in_proxy
extern const Name data_path;   //!< Data path, used by io_manager
extern const Name data_prefix; //!< Data prefix, used by io_manager
extern const Name
  dead_time; //!< Specific to ppd_sup_generator and gamma_sup_generator
extern const Name dead_time_random; //!< Random dead time or fixed dead time
                                    //!< (stochastic neuron pp_psc_delta)
extern const Name dead_time_shape;  //!< Shape parameter of the dead time
//!< distribution (stochastic neuron pp_psc_delta)
extern const Name delay;            //!< Connection parameters
extern const Name delays;           //!< Connection parameters
extern const Name deliver_interval; //!< Used by volume_transmitter
extern const Name delta_P;          //!< specific to Hill & Tononi 2005
extern const Name Delta_T; //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
extern const Name delta_tau; //!< Specific to correlation_and correlomatrix
                             //!< detector
extern const Name Delta_V;   //!< Specific to gif models
extern const Name delta_u;   //!< Specific to population point process model
                             //!< (pp_pop_psc_delta)
extern const Name dg;        //!< Derivative of the conductance
extern const Name dg_ex;     //!< Derivative of the excitatory conductance
extern const Name dg_in;     //!< Derivative of the inhibitory conductance
extern const Name dI_syn_ex; //!< Derivative of the excitatory synaptic current
extern const Name dI_syn_in; //!< Derivative of the inhibitory synaptic current
extern const Name dict_miss_is_error;  //!< Used by logging_manager
extern const Name diffusion_factor;    //!< Specific to diffusion connection
extern const Name distal_curr;         //!< Used by iaf_cond_alpha_mc
extern const Name distal_exc;          //!< Used by iaf_cond_alpha_mc
extern const Name distal_inh;          //!< Used by iaf_cond_alpha_mc
extern const Name distribution;        //!< Connectivity-related
extern const Name drift_factor;        //!< Specific to diffusion connection
extern const Name driver_readout_time; //!< Used by stdp_connection_facetshw_hom
extern const Name dt;                  //!< Miscellaneous parameters
extern const Name
  dU; //!< Unit increment of the utilization for a facilitating synapse [0...1]
      //!< (Tsodyks2_connection)

extern const Name E_ahp;        //!< Specific to iaf_chxk_2008 neuron
extern const Name E_ex;         //!< Excitatory reversal potential
extern const Name E_in;         //!< Inhibitory reversal potential
extern const Name E_K;          //!< Potassium reversal potential
extern const Name E_L;          //!< Resting potential
extern const Name E_Na;         //!< Sodium reversal potential
extern const Name E_rev;        //!< Reversal potential (array)
extern const Name E_rev_AMPA;   //!< specific to Hill & Tononi 2005
extern const Name E_rev_GABA_A; //!< specific to Hill & Tononi 2005
extern const Name E_rev_GABA_B; //!< specific to Hill & Tononi 2005
extern const Name E_rev_h;      //!< specific to Hill & Tononi 2005
extern const Name E_rev_KNa;    //!< specific to Hill & Tononi 2005
extern const Name E_rev_NaP;    //!< specific to Hill & Tononi 2005
extern const Name E_rev_NMDA;   //!< specific to Hill & Tononi 2005
extern const Name E_rev_T;      //!< specific to Hill & Tononi 2005
extern const Name E_rr;         //!< Other adaptation
extern const Name E_sfa;        //!< Other adaptation
extern const Name element_type; //!< Node type
extern const Name elementsize;  //!< Used in genericmodel
extern const Name epoch;
extern const Name eps;         //!< MSP growth curve parameter
extern const Name equilibrate; //!< specific to ht_neuron
extern const Name error;       //!< Indicates an error (sli_neuron)
extern const Name eta;         //!< MSP growth curve parameter
extern const Name events;      //!< Recorder parameter
extern const Name
  ex_spikes; //!< Number of arriving excitatory spikes (sli_neuron)

extern const Name F_lower;
extern const Name F_mean;
extern const Name F_std;
extern const Name F_upper;
extern const Name fbuffer_size;   //!< Recorder parameter
extern const Name file;           //!< Recorder parameter
extern const Name file_extension; //!< Recorder parameter
extern const Name filename;       //!< Recorder parameter
extern const Name
  filenames; //!< Recorder parameter---keep, will disappear with NESTIO
extern const Name flush_after_simulate; //!< Recorder parameter
extern const Name flush_records;        //!< Recorder parameter
extern const Name frequency;            //!< Signal modulation frequency
extern const Name frozen;               //!< Node parameter

extern const Name g;             //!< Conductance or gain scaling in rate models
extern const Name g_AMPA;        //!< specific to Hill & Tononi 2005
extern const Name g_ahp;         //!< Specific to iaf_chxk_2008 neuron
extern const Name g_ex;          //!< Excitatory conductance
extern const Name g_ext_ex;          //!< Constant external excitatory conductance
extern const Name g_ext_in;          //!< Constant external inhibitory conductance
extern const Name g_GABA_A;      //!< specific to Hill & Tononi 2005
extern const Name g_GABA_B;      //!< specific to Hill & Tononi 2005
extern const Name g_in;          //!< inhibitory conductance
extern const Name g_K;           //!< Potassium conductance
extern const Name g_KL;          //!< specific to Hill & Tononi 2005
extern const Name g_Kv1;         //!< Kv1 Potassium conductance
extern const Name g_Kv3;         //!< Kv3 Potassium conductance
extern const Name g_L;           //!< Leak conductance
extern const Name g_Na;          //!< Sodium conductance
extern const Name g_NaL;         //!< specific to Hill & Tononi 2005
extern const Name g_NMDA;        //!< specific to Hill & Tononi 2005
extern const Name g_peak_AMPA;   //!< specific to Hill & Tononi 2005
extern const Name g_peak_GABA_A; //!< specific to Hill & Tononi 2005
extern const Name g_peak_GABA_B; //!< specific to Hill & Tononi 2005
extern const Name g_peak_h;      //!< specific to Hill & Tononi 2005
extern const Name g_peak_KNa;    //!< specific to Hill & Tononi 2005
extern const Name g_peak_NaP;    //!< specific to Hill & Tononi 2005
extern const Name g_peak_NMDA;   //!< specific to Hill & Tononi 2005
extern const Name g_peak_T;      //!< specific to Hill & Tononi 2005
extern const Name g_pd;          //!< Used by iaf_cond_alpha_mc
extern const Name g_rr;          //!< Other adaptation
extern const Name g_sfa;         //!< Other adaptation
extern const Name g_sp;          //!< Used by iaf_cond_alpha_mc
>>>>>>> iaf_cond_beta
extern const Name GABA_A;
extern const Name GABA_B;
extern const Name gamma_shape;
extern const Name gaussian;
extern const Name global_id;
extern const Name grng;
extern const Name grng_seed;
extern const Name growth_curve;
extern const Name growth_rate;
extern const Name gsl_error_tol;

extern const Name h;
extern const Name has_connections;
extern const Name has_delay;
extern const Name histogram;
extern const Name histogram_correction;
extern const Name HMIN;

extern const Name I;
extern const Name I_ahp;
extern const Name I_e;
extern const Name I_h;
extern const Name I_KNa;
extern const Name I_NaP;
extern const Name I_stc;
extern const Name I_syn;
extern const Name I_syn_ex;
extern const Name I_syn_in;
extern const Name I_T;
extern const Name in_spikes;
extern const Name Inact_n;
extern const Name Inact_p;
extern const Name indegree;
extern const Name index_map;
extern const Name individual_spike_trains;
extern const Name init_flag;
extern const Name initial_connector_capacity;
extern const Name instant_unblock_NMDA;
extern const Name instantiations;
extern const Name Interpol_Order;
extern const Name interval;
extern const Name is_refractory;

extern const Name Kplus;
extern const Name Kplus_triplet;

extern const Name label;
extern const Name lambda;
extern const Name lambda_0;
extern const Name large_connector_growth_factor;
extern const Name large_connector_limit;
extern const Name len_kernel;
extern const Name linear;
extern const Name linear_summation;
extern const Name local;
extern const Name local_id;
extern const Name local_num_threads;
extern const Name local_spike_counter;
extern const Name lookuptable_0;
extern const Name lookuptable_1;
extern const Name lookuptable_2;

extern const Name make_symmetric;
extern const Name max_delay;
extern const Name MAXERR;
extern const Name mean;
extern const Name memory;
extern const Name message_times;
extern const Name messages;
extern const Name min_delay;
extern const Name model;
extern const Name mother_rng;
extern const Name mother_seed;
extern const Name ms_per_tic;
extern const Name mu;
extern const Name mu_minus;
extern const Name mu_plus;
extern const Name mult_coupling;
extern const Name multapses;
extern const Name music_channel;

extern const Name n;
extern const Name N;
extern const Name N_channels;
extern const Name n_events;
extern const Name n_messages;
extern const Name n_proc;
extern const Name n_receptors;
extern const Name n_synapses;
extern const Name network_size;
extern const Name neuron;
extern const Name next_readout_time;
extern const Name NMDA;
<<<<<<< HEAD
extern const Name no_synapses;
extern const Name node_uses_wfr;
extern const Name noise;
extern const Name noisy_rate;
extern const Name num_connections;
extern const Name num_processes;
extern const Name number_of_children;

extern const Name off_grid_spiking;
extern const Name offset;
extern const Name offsets;
extern const Name omega;
extern const Name order;
extern const Name origin;
extern const Name other;
extern const Name outdegree;
extern const Name overwrite_files;

extern const Name p;
extern const Name P;
extern const Name p_copy;
extern const Name p_transmit;
extern const Name parent;
extern const Name phase;
extern const Name port;
extern const Name port_name;
extern const Name port_width;
extern const Name ports;
extern const Name post_synaptic_element;
extern const Name pre_synaptic_element;
extern const Name precise_times;
extern const Name precision;
extern const Name print_time;
extern const Name proximal_curr;
extern const Name proximal_exc;
extern const Name proximal_inh;
extern const Name psi;
extern const Name published;
extern const Name pulse_times;

extern const Name q_rr;
extern const Name q_sfa;
extern const Name q_stc;

extern const Name rate;
extern const Name rate_times;
extern const Name rate_values;
extern const Name readout_cycle_duration;
extern const Name receive_buffer_size;
extern const Name receptor_type;
extern const Name receptor_types;
extern const Name receptors;
extern const Name record_from;
extern const Name record_to;
extern const Name recordables;
extern const Name recorder;
extern const Name rectify_output;
extern const Name refractory_input;
extern const Name registered;
extern const Name relative_amplitude;
extern const Name requires_symmetric;
extern const Name reset_pattern;
extern const Name resolution;
extern const Name rho_0;
extern const Name rng_seeds;
extern const Name rport;
extern const Name rports;
extern const Name rule;

extern const Name S;
extern const Name S_act_NMDA;
extern const Name scientific;
extern const Name screen;
extern const Name sdev;
extern const Name send_buffer_size;
extern const Name senders;
extern const Name shift_now_spikes;
extern const Name sigma;
extern const Name sigmoid;
extern const Name size_of;
extern const Name soma_curr;
extern const Name soma_exc;
extern const Name soma_inh;
extern const Name source;
extern const Name spike;
extern const Name spike_multiplicities;
extern const Name spike_times;
extern const Name spike_weights;
extern const Name start;
extern const Name std;
extern const Name std_mod;
extern const Name stimulator;
extern const Name stop;
extern const Name structural_plasticity_synapses;
extern const Name structural_plasticity_update_interval;
extern const Name structure;
extern const Name supports_precise_spikes;
extern const Name synapse_id;
extern const Name synapse_label;
extern const Name synapse_model;
extern const Name synapse_modelid;
extern const Name synapses_per_driver;
extern const Name synaptic_elements;
extern const Name synaptic_elements_param;

extern const Name t_lag;
extern const Name T_max;
extern const Name T_min;
extern const Name t_origin;
extern const Name t_ref;
extern const Name t_ref_abs;
extern const Name t_ref_remaining;
extern const Name t_ref_tot;
extern const Name t_spike;
extern const Name target;
extern const Name target_thread;
extern const Name targets;
extern const Name tau;
extern const Name tau_1;
extern const Name tau_2;
extern const Name tau_ahp;
extern const Name tau_c;
extern const Name tau_Ca;
extern const Name tau_D_KNa;
extern const Name tau_decay;
extern const Name tau_decay_AMPA;
extern const Name tau_decay_GABA_A;
extern const Name tau_decay_GABA_B;
extern const Name tau_decay_NMDA;
extern const Name tau_epsp;
extern const Name tau_eta;
extern const Name tau_ex_decay
extern const Name tau_ex_rise;
extern const Name tau_fac;
extern const Name tau_in_decay;
extern const Name tau_in_rise;
extern const Name tau_m;
extern const Name tau_max;
extern const Name tau_Mg_fast_NMDA;
extern const Name tau_Mg_slow_NMDA;
extern const Name tau_minus;
extern const Name tau_minus_stdp;
extern const Name tau_minus_triplet;
extern const Name tau_n;
extern const Name tau_P;
extern const Name tau_plus;
extern const Name tau_plus_triplet;
extern const Name tau_psc;
extern const Name tau_rec;
extern const Name tau_reset;
extern const Name tau_rise;
extern const Name tau_rise_AMPA;
extern const Name tau_rise_GABA_A;
extern const Name tau_rise_GABA_B;
extern const Name tau_rise_NMDA;
extern const Name tau_rr;
extern const Name tau_sfa;
extern const Name tau_spike;
extern const Name tau_stc;
extern const Name tau_syn;
extern const Name tau_syn_ex;
extern const Name tau_syn_in;
extern const Name tau_theta;
extern const Name tau_v;
extern const Name tau_vacant;
extern const Name tau_w;
extern const Name theta;
extern const Name theta_eq;
extern const Name theta_ex;
extern const Name theta_in;
extern const Name thread;
extern const Name thread_local_id;
extern const Name tics_per_ms;
extern const Name tics_per_step;
extern const Name time;
extern const Name time_collocate;
extern const Name time_communicate;
extern const Name time_in_steps;
extern const Name times;
extern const Name to_accumulator;
extern const Name to_do;
extern const Name to_file;
extern const Name to_memory;
extern const Name to_screen;
extern const Name total_num_virtual_procs;
extern const Name Tstart;
extern const Name Tstop;
extern const Name type_id;

extern const Name u;
extern const Name U;
extern const Name U_m;
extern const Name update;
extern const Name update_node;
extern const Name use_gid_in_filename;
extern const Name use_wfr;

extern const Name V_act_NMDA;
extern const Name V_epsp;
extern const Name V_m;
extern const Name V_min;
extern const Name V_noise;
extern const Name V_peak;
extern const Name V_reset;
extern const Name V_T;
extern const Name V_T_star;
extern const Name V_th;
extern const Name V_th_alpha_1;
extern const Name V_th_alpha_2;
extern const Name V_th_v;
extern const Name val_eta;
extern const Name voltage_clamp;
extern const Name vp;
extern const Name vt;

extern const Name w;
extern const Name weight;
extern const Name weight_per_lut_entry;
extern const Name weight_recorder;
extern const Name weighted_spikes_ex;
extern const Name weighted_spikes_in;
extern const Name weights;
extern const Name wfr_comm_interval;
extern const Name wfr_interpolation_order;
extern const Name wfr_max_iterations;
extern const Name wfr_tol;
extern const Name with_reset;
extern const Name withgid;
extern const Name withport;
extern const Name withrport;
extern const Name withtargetgid;
extern const Name withtime;
extern const Name withweight;
extern const Name Wmax;
extern const Name Wmin;

extern const Name x;

extern const Name y1;
extern const Name y2;
extern const Name y;
extern const Name y_0;
extern const Name y_1;

extern const Name z;
extern const Name z_connected;

} // namespace names

} // namespace nest

#endif /* #ifndef NEST_NAMES_H */
=======
extern const Name node_uses_wfr;      //!< Node parameter
extern const Name noise;              //!< Specific to iaf_chs_2008 neuron
                                      //!< and rate models
extern const Name noisy_rate;         //!< Specific to rate models
extern const Name no_synapses;        //!< Used by stdp_connection_facetshw_hom
extern const Name num_connections;    //!< In ConnBuilder
extern const Name num_processes;      //!< Number of processes
extern const Name number_of_children; //!< Used by Subnet

extern const Name off_grid_spiking; //!< Used by event_delivery_manager
extern const Name offset;           //!< Miscellaneous parameters
extern const Name offsets;          //!< Recorder parameter
extern const Name omega;     //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name order;     //!< Specific to sinusoidally modulated generators
extern const Name origin;    //!< Device parameters
extern const Name other;     //!< Node type
extern const Name outdegree; //!< In FixedOutDegreeBuilder
extern const Name overwrite_files; //!< Used in io_manager

extern const Name P; //!< specific to Hill & Tononi 2005
extern const Name p; //!< current release probability  (Tsodyks2_connection)
extern const Name p_copy;                //!< Specific to mip_generator
extern const Name p_transmit;            //!< Specific to bernoulli_synapse
extern const Name parent;                //!< Node parameter
extern const Name phase;                 //!< Signal phase in degrees
extern const Name phi;                   //!< Specific to mirollo_strogatz_ps
extern const Name phi_th;                //!< Specific to mirollo_strogatz_ps
extern const Name port;                  //!< Connection parameters
extern const Name ports;                 //!< Recorder parameter
extern const Name port_name;             //!< Parameters for MUSIC devices
extern const Name port_width;            //!< Parameters for MUSIC devices
extern const Name post_synaptic_element; //!< Post synaptic elements
extern const Name potentials;            //!< Recorder parameter
extern const Name pre_synaptic_element;  //!< Pre synaptic elements
extern const Name precise_times;         //!< Recorder parameter
extern const Name precision;             //!< Recorder parameter
extern const Name print_time;            //!< Simulation-related
extern const Name proximal_curr;         //!< Used by iaf_cond_alpha_mc
extern const Name proximal_exc;          //!< Used by iaf_cond_alpha_mc
extern const Name proximal_inh;          //!< Used by iaf_cond_alpha_mc
extern const Name PSC_adapt_step;     //!< PSC increment (current homeostasis)
extern const Name PSC_Unit_amplitude; //!< Scaling of PSC (current homeostasis)
extern const Name psi;         //!< Width parameter for sigmoid growth curve
extern const Name published;   //!< Parameters for MUSIC devices
extern const Name pulse_times; //!< used in pulsepacket:generator

extern const Name q_rr;  //!< Other adaptation
extern const Name q_sfa; //!< Other adaptation
extern const Name q_stc; //!< Specific to gif models

extern const Name rate; //!< Specific to ppd_sup_generator,
                        //!< gamma_sup_generator and rate models
extern const Name readout_cycle_duration; //!< Used by
                                          //!< stdp_connection_facetshw_hom
extern const Name receive_buffer_size;    //!< mpi-related
extern const Name receptor_type;          //!< Connection parameter
extern const Name receptor_types;         //!< Publishing available types
extern const Name receptors;              //!< Used in mpi_manager
extern const Name record_from;            //!< Recorder parameter
extern const Name record_to;              //!< Recorder parameter
extern const Name
  recordables; //!< List of recordable state data (Device parameters)
extern const Name recorder; //!< Node type
extern const Name
  refractory_input; //!< Spikes arriving during refractory period are counted
                    //!< (precise timing neurons)
extern const Name registered; //!< Parameters for MUSIC devices
extern const Name
  relative_amplitude; //!< Signal modulation amplitude relative to mean
extern const Name reset_pattern;      //!< Used in stdp_connection_facetshw_hom
extern const Name resolution;         //!< Time resolution
extern const Name requires_symmetric; //!< Used in connector_model_impl
extern const Name rho_0;     //!< Specific to population point process model
                             //!< (pp_pop_psc_delta)
extern const Name rms;       //!< Root mean square
extern const Name rng_seeds; //!< Used in rng_manager
extern const Name root_finding_epsilon; //!< Accuracy of the root of the
//!< polynomial (precise timing neurons (Brette 2007))
extern const Name rport;  //!< Connection parameters
extern const Name rports; //!< Recorder parameter
extern const Name rule;   //!< Connectivity-related

extern const Name S; //!< Binary state (output) of neuron (Ginzburg neuron)
extern const Name S_act_NMDA;       //!< specific to Hill & Tononi 2005
extern const Name scientific;       //!< Recorder parameter
extern const Name screen;           //!< Recorder parameter
extern const Name sdev;             //!< Used in pulsepacket_generator
extern const Name send_buffer_size; //!< mpi-related
extern const Name senders;          //!< Recorder parameter
extern const Name shift_now_spikes; //!< Used by spike_generator
extern const Name sigmoid;          //!< Sigmoid MSP growth curve
extern const Name size_of;          //!< Connection parameters
extern const Name soma_curr;        //!< Used by iaf_cond_alpha_mc
extern const Name soma_exc;         //!< Used by iaf_cond_alpha_mc
extern const Name soma_inh;         //!< Used by iaf_cond_alpha_mc
extern const Name source;           //!< Connection parameters
extern const Name spike; //!< true if the neuron spikes and false if not.
                         //!< (sli_neuron)
extern const Name spike_multiplicities;           //!x Used by spike_generator
extern const Name spike_times;                    //!< Recorder parameter
extern const Name spike_weights;                  //!< Used by spike_generator
extern const Name start;                          //!< Device parameters
extern const Name state;                          //!< Node parameter
extern const Name std;                            //!< Miscellaneous parameters
extern const Name std_mod;                        //!< Miscellaneous parameters
extern const Name stimulator;                     //!< Node type
extern const Name stop;                           //!< Device parameters
extern const Name structural_plasticity_synapses; //!< Synapses defined for
// structural plasticity
extern const Name structural_plasticity_update_interval; //!< Update interval
// for structural
// plasticity
extern const Name structure; //!< Node type
extern const Name success;
extern const Name supports_precise_spikes; //!< true if model supports precise
                                           //!< spikes
extern const Name synapse;                 //!< Node type
extern const Name synapse_id;          //!< Used by stdp_connection_facetshw_hom
extern const Name synapse_label;       //!< Label id of synapses with labels
extern const Name synapse_model;       //!< Connection parameters
extern const Name synapse_modelid;     //!< Connection parameters
extern const Name synapses_per_driver; //!< Used by stdp_connection_facetshw_hom
extern const Name synaptic_elements;   //!< Synaptic elements used in structural
                                       //!< plasticity
extern const Name synaptic_elements_param; //!< Used to update parameters
                                           //!< of synaptic elements

extern const Name t_lag;     //!< Lag within a time slice
extern const Name T_min;     //!< Minimum time
extern const Name T_max;     //!< Maximum time
extern const Name t_origin;  //!< Origin of a time-slice
extern const Name t_ref;     //!< Refractory period
extern const Name t_ref_abs; //!< Absolute refractory period, iaf_tum_2000
extern const Name
  t_ref_remaining;           //!< Time remaining till end of refractory state
extern const Name t_ref_tot; //!< Total refractory period, iaf_tum_2000
extern const Name t_spike;   //!< Time of last spike
extern const Name target;    //!< Connection parameters
extern const Name target_thread; //!< Connection parameters
extern const Name targets;       //!< Connection parameters
extern const Name tau;           //!< Used by stdp_connection_facetshw_hom
                                 //!< and rate models
extern const Name tau_1;     //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name tau_2;     //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name tau_ahp;   //!< Specific to iaf_chxk_2008 neuron
extern const Name tau_Ca;    //!< Rate of loss of calcium concentration
extern const Name tau_c;     //!< Used by stdp_dopa_connection
extern const Name tau_D_KNa; //!< specific to Hill & Tononi 2005
extern const Name tau_decay; //!< Synapse decay constant (beta fct decay)
extern const Name tau_decay_AMPA;   //!< specific to Hill & Tononi 2005
extern const Name tau_decay_GABA_A; //!< specific to Hill & Tononi 2005
extern const Name tau_decay_GABA_B; //!< specific to Hill & Tononi 2005
extern const Name tau_decay_NMDA;   //!< specific to Hill & Tononi 2005
extern const Name tau_epsp;         //!< Specific to iaf_chs_2008 neuron
extern const Name tau_eta; //!< Specific to population point process model
                           //!< (pp_pop_psc_delta)
extern const Name
  tau_fac; //!< facilitation time constant (ms) (Tsodyks2_connection)
extern const Name
  tau_lcm;               //!< Least common multiple of tau_m, tau_ex and tau_in
                         //!< (precise timing neurons (Brette 2007))
extern const Name tau_m; //!< Membrane time constant
extern const Name
  tau_max; //!< Specific to correlation_and correlomatrix detector
extern const Name tau_Mg_fast_NMDA;  //!< specific to Hill & Tononi 2005
extern const Name tau_Mg_slow_NMDA;  //!< specific to Hill & Tononi 2005
extern const Name tau_minus;         //!< used for ArchivingNode
extern const Name tau_minus_stdp;    //!< Used by stdp_connection_facetshw_hom
extern const Name tau_minus_triplet; //!< used for ArchivingNode
extern const Name tau_n;             //!< Used by stdp_dopa_connection
extern const Name tau_P;             //!< specific to Hill & Tononi 2005
extern const Name tau_plus;          //!< stdp_synapse parameter
extern const Name tau_plus_triplet;  //!< Used by stdp_connection_facetshw_hom
extern const Name tau_psc;           //!< Used by stdp_connection_facetshw_hom
extern const Name
  tau_rec; //!< time constant for recovery (ms) (Tsodyks2_connection)
extern const Name tau_reset;       //!< Specific to iaf_chs_2008 neuron
extern const Name tau_ex_decay;        //!< Synapse excitatory decay constant (iaf_cond_beta)
extern const Name tau_ex_rise;        //!< Synapse excitatory rise constant (iaf_cond_beta)
extern const Name tau_in_decay;        //!< Synapse inhibitory decay constant (iaf_cond_beta)
extern const Name tau_in_rise;        //!< Synapse inhibitory rise constant (iaf_cond_beta)
extern const Name tau_rise;        //!< Synapse rise constant (beta fct rise)
extern const Name tau_rise_AMPA;   //!< specific to Hill & Tononi 2005
extern const Name tau_rise_GABA_A; //!< specific to Hill & Tononi 2005
extern const Name tau_rise_GABA_B; //!< specific to Hill & Tononi 2005
extern const Name tau_rise_NMDA;   //!< specific to Hill & Tononi 2005
extern const Name tau_rr;          //!< Other adaptation
extern const Name tau_sfa;         //!< Other adaptation
extern const Name tau_spike;       //!< Specific to Hill-Tononi (2005)
extern const Name tau_stc;         //!< Specific to gif models
extern const Name tau_syn;         //!< Synapse time constant
extern const Name tau_syn_ex;      //!< Excitatory synaptic time constant
extern const Name tau_syn_in;      //!< Inhibitory synaptic time constant
extern const Name tau_theta;       //!< Specific to Hill-Tononi (2005)
extern const Name tau_v;           //!< Specific to amat2_*
extern const Name tau_vacant;      //!< Parameter for MSP dynamics
extern const Name tau_w; //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
extern const Name theta; //!< Did not compile without (theta neuron problem)
extern const Name theta_eq;                //!< specific to Hill & Tononi 2005
extern const Name thread;                  //!< Node parameter
extern const Name thread_local_id;         //!< Thead-local ID of node,
                                           //!< see Kunkel et al 2014, Sec 3.3.2
extern const Name tics_per_ms;             //!< Simulation-related
extern const Name tics_per_step;           //!< Simulation-related
extern const Name time_in_steps;           //!< Recorder parameter
extern const Name time;                    //!< Simulation-related
extern const Name time_collocate;          //!< Used by event_delivery_manager
extern const Name time_communicate;        //!< Used by event_delivery_manager
extern const Name times;                   //!< Recorder parameter
extern const Name to_accumulator;          //!< Recorder parameter
extern const Name to_do;                   //!< Simulation-related
extern const Name to_file;                 //!< Recorder parameter
extern const Name to_memory;               //!< Recorder parameter
extern const Name to_screen;               //!< Recorder parameter
extern const Name total_num_virtual_procs; //!< Total number virtual processes
extern const Name Tstart;                  //!< Specific to correlation and
                                           //!< correlomatrix detector
extern const Name Tstop; //!< Specific to correlation and correlomatrix detector
extern const Name type_id; //!< model paramater

extern const Name u; //!< probability of release [0...1] (Tsodyks2_connection)
extern const Name U; //!< Used by stdp_connection_facetshw_hom
extern const Name U_lower;
extern const Name U_m; //!< Specific to Izhikevich 2003
extern const Name U_mean;
extern const Name U_std;
extern const Name U_upper;
extern const Name update;      //!< Command to execute the neuron (sli_neuron)
extern const Name update_node; //!< Command to execute the neuron (sli_neuron)
extern const Name use_wfr;     //!< Simulation-related
extern const Name use_gid_in_filename; //!< use gid in the filename

extern const Name V_act_NMDA; //!< specific to Hill & Tononi 2005
extern const Name V_epsp;     //!< Specific to iaf_chs_2008 neuron
extern const Name V_m;        //!< Membrane potential
extern const Name V_min;   //!< Absolute lower value for the membrane potential
extern const Name V_noise; //!< Specific to iaf_chs_2008 neuron
extern const Name
  V_peak; //!< Spike detection threshold (Brette & Gerstner 2005)
extern const Name V_reset;  //!< Reset potential
extern const Name V_T;      //!< Voltage offset
extern const Name V_T_star; //!< Specific to gif models
extern const Name V_th;     //!< Threshold
extern const Name
  V_th_alpha_1; //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name
  V_th_alpha_2;           //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name V_th_v; //!< Specific to amat2_*
extern const Name
  val_eta; //!< Specific to population point process model (pp_pop_psc_delta)
extern const Name voltage_clamp; //!< Enforce voltage clamp
extern const Name vt;            //!< Used by stdp_dopa_connection
extern const Name vp;            //!< Node parameter

extern const Name w;      //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
extern const Name weight; //!< Connection parameters
extern const Name weight_per_lut_entry; //!< Used by
                                        //!< stdp_connection_facetshw_hom
extern const Name weight_std; //!< Standard deviation/mean of noisy synapse.
extern const Name weighted_spikes_ex; //!< Weighted incoming excitatory spikes
extern const Name weighted_spikes_in; //!< Weighted incoming inhibitory spikes
extern const Name weights;            //!< Connection parameters --- topology
extern const Name weight_recorder;    //!< Device name
extern const Name wfr_comm_interval;  //!< Simulation-related
extern const Name wfr_interpolation_order; //!< Simulation-related
extern const Name wfr_max_iterations;      //!< Simulation-related
extern const Name wfr_tol;                 //!< Simulation-related
extern const Name with_noise;              //!< Simulation-related
extern const Name with_reset; //!< Shall the pp_neuron reset after each spike?
                              //!< (stochastic neuron pp_psc_delta)
extern const Name withgid;    //!< Recorder parameter
extern const Name withpath;   //!< Recorder parameter
extern const Name withport;   //!< Recorder parameter
extern const Name withrport;  //!< Recorder parameter
extern const Name withtargetgid; //!< Recorder parameter
extern const Name withtime;      //!< Recorder parameter
extern const Name withweight;    //!< Recorder parameter
extern const Name Wmax;          //!< stdp_synapse parameter
extern const Name Wmin;          //!< Used by stdp_dopa_connection

extern const Name x; //!< current scaling factor of the synaptic weight [0...1]
                     //!< (Tsodyks2_connection)

extern const Name y;   //!< Used by stdp_connection_facetshw_hom
extern const Name y_0; //!< Used by ac_generator
extern const Name y_1; //!< Used by ac_generator
extern const Name y1;  //!< Used in iaf_psc_alpha_canon
extern const Name y2;  //!< Used in iaf_psc_alpha_canon

extern const Name z; //!< Number of available synaptic elements per node
extern const Name
  z_connected; //!< Number of connected synaptic elements per node
}
}

#endif
>>>>>>> iaf_cond_beta
