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
 * Node::get_status and Node::set_status to make data exchange
 * more efficient and consistent. Creating a Name from a std::string
 * is in O(log n), for n the number of Names already created. Using
 * predefined names should make data exchange much more efficient.
 */
namespace names
{
extern const Name a; //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
extern const Name a_acausal; //!< Used by stdp_connection_facetshw_hom
extern const Name a_causal;  //!< Used by stdp_connection_facetshw_hom
extern const Name A_lower;
extern const Name A_mean;
extern const Name A_minus; //!< Used by stdp_dopa_connection
extern const Name A_plus;  //!< Used by stdp_dopa_connection
extern const Name A_std;
extern const Name a_thresh_th; //!< Used by stdp_connection_facetshw_hom
extern const Name a_thresh_tl; //!< Used by stdp_connection_facetshw_hom
extern const Name A_upper;
extern const Name acceptable_latency;   //!< Used in music_message_in_proxy
extern const Name accumulator;          //!< Recorder parameter
extern const Name Act_h;                //!< Specific to Hodgkin Huxley models
extern const Name Act_m;                //!< Specific to Hodgkin Huxley models
extern const Name activity;             //!< Used in pulsepacket_generator
extern const Name address;              //!< Node parameter
extern const Name ahp_bug;              //!< Used in iaf_chxk_2008
extern const Name allow_offgrid_spikes; //!< Used in spike_generator
extern const Name allow_offgrid_times;  //!< Used in step_current_generator
extern const Name alpha;                //!< stdp_synapse parameter
extern const Name alpha_1; //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name alpha_2; //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
extern const Name Aminus;  //!< Used by stdp_connection_facetshw_hom
extern const Name Aminus_triplet; //!< Used by stdp_connection_facetshw_hom
extern const Name AMPA;
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
extern const Name GABA_A;
extern const Name GABA_B;
extern const Name gamma;         //!< Specific to mirollo_strogatz_ps
extern const Name gamma_shape;   //!< Specific to ppd_sup_generator and
                                 //!< gamma_sup_generator
extern const Name gaussian;      //!< Parameter for MSP growth curves
extern const Name global_id;     //!< Node parameter
extern const Name grng;          //!< Used in rng_manager
extern const Name grng_seed;     //!< Seed
extern const Name growth_curve;  //!< Growth curve for MSP dynamics
extern const Name growth_rate;   //!< Parameter of the growth curve for MSP
                                 //!< dynamics
extern const Name gsl_error_tol; //!< GSL integrator tolerance

extern const Name h; //!< Summed input to a neuron (Ginzburg neuron)
extern const Name has_connections; //!< Specific to iaf_psc_exp_multisynapse and
                                   //!< iaf_psc_alpha_multisynapse
extern const Name has_delay;       //!< Used in connector_model_impl
extern const Name histogram;       //!< Specific to correlation_detector
extern const Name histogram_correction; //!< Specific to correlation_detector
extern const Name HMIN; //!< Smallest integration step for adaptive stepsize
                        //!< (Brette & Gerstner 2005)

extern const Name I;        //!< mirollo_strogatz_ps, StimulatingDevice
extern const Name I_adapt;  //!< Goal of current homeostasis
                            //!< (current homeostasis)
extern const Name I_ahp;    //!< Used in iaf_chxk_2008
extern const Name I_e;      //!< Input current
extern const Name I_ex;     //!< Excitatory synaptic input current
extern const Name I_h;      //!< specific to Hill & Tononi 2005
extern const Name I_in;     //!< Inhibitory synaptic input current
extern const Name I_KNa;    //!< specific to Hill & Tononi 2005
extern const Name I_L;      //!< Leak current
extern const Name I_NaP;    //!< specific to Hill & Tononi 2005
extern const Name I_stc;    //!< relevant for gif models
extern const Name I_std;    //!< Standard deviation of current distribution
                            //!< (current homeostasis)
extern const Name I_syn;    //!< used for iaflossless_count_exp
extern const Name I_syn_ex; //!< Total excitatory synaptic current
                            //!< (iaf_tum_2000)
extern const Name I_syn_in; //!< Total inhibitory synaptic current
                            //!< (iaf_tum_2000)
extern const Name I_T;      //!< specific to Hill & Tononi 2005
extern const Name I_total;  //!< Total current (current homeostasis)
extern const Name
  in_spikes; //!< Number of arriving inhibitory spikes (sli_neuron)
extern const Name Inact_n; //!< Specific to Hodgkin Huxley models
extern const Name
  Inact_p; //!< Specific to Hodgkin Huxley models with gap junctions
extern const Name indegree;                //!< In FixedInDegreeBuilder
extern const Name index_map;               //!< Parameters for MUSIC devices
extern const Name individual_spike_trains; //!< Generator parameters
extern const Name inh_conductance;         //!< Recorder parameter
extern const Name init_flag; //!< Used by stdp_connection_facetshw_hom
extern const Name initial_connector_capacity; //!< Initial Connector capacity
extern const Name instant_unblock_NMDA;       //!< specific to Hill-Tononi
extern const Name instantiations;             //!< model paramater
extern const Name
  Interpol_Order;           //!< Interpolation order (precise timing neurons)
extern const Name interval; //!< Recorder parameter
extern const Name is_refractory; //!< Neuron is in refractory period (debugging)

extern const Name Kplus;         //!< Used by stdp_connection_facetshw_hom
extern const Name Kplus_triplet; //!< Used by stdp_connection_facetshw_hom

extern const Name label;    //!< Miscellaneous parameters
extern const Name lambda;   //!< stdp_synapse parameter
extern const Name lambda_0; //!< Specific to gif models
extern const Name
  large_connector_growth_factor;         //! Growth factor for large connectors
extern const Name large_connector_limit; //! Cutoff for large connectors
extern const Name len_kernel; //!< Specific to population point process model
                              //!< (pp_pop_psc_delta)
extern const Name linear;     //!< Parameter for MSP growth curves
extern const Name linear_summation;    //!< Specific to rate models
extern const Name local;               //!< Node parameter
extern const Name local_id;            //!< Node
extern const Name local_num_threads;   //!< Local number of threads
extern const Name local_spike_counter; //!< Used by event_delivery_manager
extern const Name lookuptable_0;       //!< Used in stdp_connection_facetshw_hom
extern const Name lookuptable_1;       //!< Used in stdp_connection_facetshw_hom
extern const Name lookuptable_2;       //!< Used in stdp_connection_facetshw_hom

extern const Name make_symmetric; //!< Connectivity-related
extern const Name max_delay;      //!< In ConnBuilder
extern const Name MAXERR; //!< Largest permissible error for adaptive stepsize
                          //!< (Brette & Gerstner 2005)
extern const Name mean;   //!< Miscellaneous parameters
extern const Name memory; //!< Recorder parameter
extern const Name message_times; //!< Used in music_message_in_proxy
extern const Name messages;      //!< Used in music_message_in_proxy
extern const Name min_delay;     //!< In ConnBuilder
extern const Name model;         //!< Node parameter
extern const Name mother_rng;    //!< Specific to mip_generator
extern const Name mother_seed;   //!< Specific to mip_generator
extern const Name ms_per_tic;    //!< Simulation-related
extern const Name mu;        //!< Used by stdp_dopa_connection and rate models
                             //(Gaussian gain function (tuning peak))
extern const Name mu_minus;  //!< stdp_synapse parameter
extern const Name mu_plus;   //!< stdp_synapse parameter
extern const Name multapses; //!< Connectivity-related
extern const Name mult_coupling; //!< Specific to rate models
extern const Name music_channel; //!< Parameters for MUSIC devices

extern const Name n;          //!< Number of synaptic release sites (int >=0)
                              //!< (Tsodyks2_connection)
extern const Name N;          //!< Specific to population point process model
                              //!< (pp_pop_psc_delta)
extern const Name N_channels; //!< Specific to correlomatrix_detector
extern const Name n_events;   //!< Recorder parameter
extern const Name n_messages; //!< Used in music_message_in_proxy
extern const Name
  n_proc; //!< Number of component processes of ppd_sup_/gamma_sup_generator
extern const Name n_receptors; //!< number of receptor ports
extern const Name n_synapses;
extern const Name neuron;            //!< Node type
extern const Name network_size;      //!< Network size
extern const Name next_readout_time; //!< Used by stdp_connection_facetshw_hom
extern const Name NMDA;
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
extern const Name recorder;       //!< Node type
extern const Name rectify_output; //!< Specific to rate models
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
extern const Name sigma; //!< For rate models (Gaussian gain function (tuning spread))
                         //!< or inverse steepness of erfc_neuron
extern const Name sigmoid;   //!< Sigmoid MSP growth curve
extern const Name size_of;   //!< Connection parameters
extern const Name soma_curr; //!< Used by iaf_cond_alpha_mc
extern const Name soma_exc;  //!< Used by iaf_cond_alpha_mc
extern const Name soma_inh;  //!< Used by iaf_cond_alpha_mc
extern const Name source;    //!< Connection parameters
extern const Name spike;     //!< true if the neuron spikes and false if not.
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
extern const Name theta_eq; //!< specific to Hill & Tononi 2005
extern const Name theta_ex; //!< specific to rate neurons (offset excitatory
// multiplicative coupling)
extern const Name theta_in; //!< specific to rate neurons (offset inhibitory
// multiplicative coupling)
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
