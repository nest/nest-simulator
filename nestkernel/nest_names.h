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
    extern const Name A;
    extern const Name a;                        //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
    extern const Name A_lower;
    extern const Name A_mean;
    extern const Name A_std;
    extern const Name A_upper;
    extern const Name ac;                       //!< Specific to sinusoidally modulated generators
    extern const Name accumulator;              //!< Recorder parameter
    extern const Name Act_h;                    //!< Specific to Hodgkin Huxley models
    extern const Name Act_m;                    //!< Specific to Hodgkin Huxley models
    extern const Name address;                  //!< Node parameter
    extern const Name alpha_1;                  //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name alpha_2;                  //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name amplitude;                //!< Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name archiver_length;          //!< used for ArchivingNode
    extern const Name as;                       //!< Number of available release sites (property arrays)
    extern const Name autapses;                 //!< Connectivity-related

    extern const Name b;                        //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
    extern const Name beta;                     //!< Specific to amat2_*
    extern const Name binary;                   //!< Recorder parameter

    extern const Name c;                        //!< Specific to Izhikevich 2003
    extern const Name c_1;                      //!< Specific to stochastic neuron pp_psc_delta
    extern const Name c_2;                      //!< Specific to stochastic neuron pp_psc_delta
    extern const Name c_3;                      //!< Specific to stochastic neuron pp_psc_delta
    extern const Name c_m;                      //!< Capacity or specific capacitance
    extern const Name C_m;                      //!< Membrane capacitance
    extern const Name calibrate;                //!< Command to calibrate the neuron (sli_neuron)
    extern const Name calibrate_node;           //!< Command to calibrate the neuron (sli_neuron)
    extern const Name clear;                    //!< used for ArchivingNode
    extern const Name close_after_simulate;     //!< Recorder parameter
    extern const Name close_on_reset;           //!< Recorder parameter
    extern const Name coeff_ex;                 //!< tau_lcm=coeff_ex*tau_ex (precise timing neurons (Brette 2007))
    extern const Name coeff_in;                 //!< tau_lcm=coeff_in*tau_in (precise timing neurons (Brette 2007))
    extern const Name coeff_m;                  //!< tau_lcm=coeff_m*tau_m (precise timing neurons (Brette 2007))
    extern const Name connection_count;         //!< Parameters for MUSIC devices
    extern const Name consistent_integration;   //!< Specific to Izhikevich 2003
    extern const Name count_covariance;         //!< Specific to correlomatrix_detector
    extern const Name count_histogram;          //!< Specific to correlation_detector
    extern const Name covariance;               //!< Specific to correlomatrix_detector
    extern const Name currents;                 //!< Recorder parameter

    extern const Name d;                        //!< Specific to Izhikevich 2003
    extern const Name D_lower;
    extern const Name D_mean;
    extern const Name D_std;
    extern const Name D_upper;
    extern const Name dc;                       //!< Specific to sinusoidally modulated generators
    extern const Name dead_time;                //!< Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name dead_time_random;         //!< Random dead time or fixed dead time (stochastic neuron pp_psc_delta)
    extern const Name dead_time_shape;          //!< Shape parameter of the dead time distribution (stochastic neuron pp_psc_delta)
    extern const Name delay;                    //!< Connection parameters
    extern const Name delays;                   //!< Connection parameters
    extern const Name Delta_T;                  //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
    extern const Name delta_tau;                //!< Specific to correlation_and correlomatrix detector
    extern const Name delta_u;                  //!< Specific to population point process model (pp_pop_psc_delta)
    extern const Name dg_ex;                    //!< Derivative of the excitatory conductance
    extern const Name dg_in;                    //!< Derivative of the inhibitory conductance
    extern const Name dhaene_det_spikes;        //!< used for iaflossless_count_exp
    extern const Name dhaene_max_geq_V_th;      //!< used for iaflossless_count_exp
    extern const Name dhaene_quick1;            //!< used for iaflossless_count_exp
    extern const Name dhaene_quick2;            //!< used for iaflossless_count_exp
    extern const Name dhaene_tmax_lt_t1;        //!< used for iaflossless_count_exp
    extern const Name distribution;             //!< Connectivity-related
    extern const Name dt;                       //!< Miscellaneous parameters
    extern const Name dU;                       //!< Unit increment of the utilization for a facilitating synapse [0...1] (Tsodyks2_connection)
    extern const Name dUs;                      //!< Unit increment of the utilization for a facilitating synapse [0...1] (property arrays)

    extern const Name E_ahp;                    //!< Specific to iaf_chxk_2008 neuron
    extern const Name E_ex;                     //!< Excitatory reversal potential
    extern const Name E_in;                     //!< Inhibitory reversal potential
    extern const Name E_K;                      //!< Potassium reversal potential
    extern const Name E_L;                      //!< Resting potential
    extern const Name E_Na;                     //!< Sodium reversal potential
    extern const Name E_rr;                     //!< Other adaptation
    extern const Name E_sfa;                    //!< Other adaptation
    extern const Name element_type;             //!< Node type
    extern const Name epoch;
    extern const Name eq12;                     //!< used for iaflossless_count_exp
    extern const Name eq13;                     //!< used for iaflossless_count_exp
    extern const Name eq7;                      //!< used for iaflossless_count_exp
    extern const Name eq9;                      //!< used for iaflossless_count_exp
    extern const Name eqs7and9;                 //!< used for iaflossless_count_exp
    extern const Name error;                    //!< Indicates an error in a neuron
    extern const Name events;                   //!< Recorder parameter
    extern const Name ex_spikes;                //!< Number of arriving excitatory spikes
    extern const Name exc_conductance;          //!< Recorder parameter

    extern const Name F_lower;
    extern const Name F_mean;
    extern const Name F_std;
    extern const Name F_upper;
    extern const Name fbuffer_size;             //!< Recorder parameter
    extern const Name file;                     //!< Recorder parameter
    extern const Name file_extension;           //!< Recorder parameter
    extern const Name filename;                 //!< Recorder parameter
    extern const Name filenames;                //!< Recorder parameter
    extern const Name flush_after_simulate;     //!< Recorder parameter
    extern const Name flush_records;            //!< Recorder parameter
    extern const Name freq;                     //!< Specific to sinusoidally modulated generators
    extern const Name frequency;                //!< Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name frozen;                   //!< Node parameter

    extern const Name g;                        //!< Conductance
    extern const Name g_ahp;                    //!< Specific to iaf_chxk_2008 neuron
    extern const Name g_ex;                     //!< Excitatory conductance
    extern const Name g_in;                     //!< inhibitory conductance
    extern const Name g_K;                      //!< Potassium conductance
    extern const Name g_L;                      //!< Leak conductance
    extern const Name g_Na;                     //!< Sodium conductance
    extern const Name g_rr;                     //!< Other adaptation
    extern const Name g_sfa;                    //!< Other adaptation
    extern const Name gamma;                    //!< Specific to mirollo_strogatz_ps
    extern const Name gamma_shape;              //!< Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name global_id;                //!< Node parameter
    extern const Name gsl_error_tol;            //!< GSL integrator tolerance

    extern const Name h;                        //!< Summed input to a neuron (Ginzburg neuron)
    extern const Name has_connections;          //!< Specific to iaf_psc_exp_multisynapse and iaf_psc_alpha_multisynapse
    extern const Name histogram;                //!< Specific to correlation_detector
    extern const Name histogram_correction;     //!< Specific to correlation_detector
    extern const Name HMIN;                     //!< Smallest integration step for adaptive stepsize (Brette & Gerstner 2005)

    extern const Name I;                        //!< Specific to mirollo_strogatz_ps
    extern const Name I_adapt;                  //!< Goal of current homeostasis (current homeostasis)
    extern const Name I_e;                      //!< Input current
    extern const Name I_ex;                     //!< Excitatory synaptic input current
    extern const Name I_in;                     //!< Inhibitory synaptic input current
    extern const Name I_L;                      //!< Leak current
    extern const Name I_std;                    //!< Standard deviation of current distribution (current homeostasis)
    extern const Name I_syn;                    //!< used for iaflossless_count_exp
    extern const Name I_syn_ex;                 //!< Total excitatory synaptic current (iaf_tum_2000)
    extern const Name I_syn_in;                 //!< Total inhibitory synaptic current (iaf_tum_2000)
    extern const Name I_total;                  //!< Total current (current homeostasis)
    extern const Name in_spikes;                //!< Number of arriving inhibitory spikes
    extern const Name Inact_n;                  //!< Specific to Hodgkin Huxley models
    extern const Name index_map;                //!< Parameters for MUSIC devices
    extern const Name individual_spike_trains;  //!< Generator parameters
    extern const Name inh_conductance;          //!< Recorder parameter
    extern const Name input_currents_ex;        //!< Incoming excitatory currents
    extern const Name input_currents_in;        //!< Incoming inhibitory currents
    extern const Name Interpol_Order;           //!< Interpolation order (precise timing neurons)
    extern const Name interval;                 //!< Recorder parameter
    extern const Name is_refractory;            //!< Neuron is in refractory period (debugging)

    extern const Name label;                    //!< Miscellaneous parameters
    extern const Name len_kernel;               //!< Specific to population point process model (pp_pop_psc_delta)
    extern const Name lin_left_geq_V_th;        //!< used for iaflossless_count_exp
    extern const Name lin_max_geq_V_th;         //!< used for iaflossless_count_exp
    extern const Name local;                    //!< Node parameter
    extern const Name local_id;                 //!< Node

    extern const Name MAXERR;                   //!< Largest permissible error for adaptive stepsize (Brette & Gerstner 2005)
    extern const Name mean;                     //!< Miscellaneous parameters
    extern const Name memory;                   //!< Recorder parameter
    extern const Name model;                    //!< Node parameter
    extern const Name mother_rng;               //!< Specific to mip_generator
    extern const Name mother_seed;              //!< Specific to mip_generator
    extern const Name multapses;                //!< Connectivity-related
    extern const Name music_channel;            //!< Parameters for MUSIC devices

    extern const Name n;                        //!< Number of synaptic release sites (int >=0) (Tsodyks2_connection)
    extern const Name N;                        //!< Specific to population point process model (pp_pop_psc_delta)
    extern const Name N_channels;               //!< Specific to correlomatrix_detector
    extern const Name n_events;                 //!< Recorder parameter
    extern const Name n_proc;                   //!< Number of component processes of ppd_sup_/gamma_sup_generator
    extern const Name neuron;                   //!< Node type
    extern const Name noise;                    //!< Specific to iaf_chs_2008 neuron
    extern const Name ns;                       //!< Number of release sites (property arrays)

    extern const Name offset;                   //!< Miscellaneous parameters
    extern const Name offsets;                  //!< Recorder parameter
    extern const Name omega;                    //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name order;                    //!< Specific to sinusoidally modulated generators
    extern const Name origin;                   //!< Device parameters
    extern const Name other;                    //!< Node type

    extern const Name p;                        //!< current release probability  (Tsodyks2_connection)
    extern const Name p_copy;                   //!< Specific to mip_generator
    extern const Name parent;                   //!< Node parameter
    extern const Name phase;                    //!< Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name phi;                      //!< Specific to mirollo_strogatz_ps
    extern const Name phi_th;                   //!< Specific to mirollo_strogatz_ps
    extern const Name port;                     //!< Connection parameters
    extern const Name port_name;                //!< Parameters for MUSIC devices
    extern const Name port_width;               //!< Parameters for MUSIC devices
    extern const Name pot_spikes;               //!< used for iaflossless_count_exp
    extern const Name potentials;               //!< Recorder parameter
    extern const Name precise_times;            //!< Recorder parameter
    extern const Name precision;                //!< Recorder parameter
    extern const Name ps;                       //!< current release probability [0...1] (property arrays)
    extern const Name PSC_adapt_step;           //!< PSC increment (current homeostasis)
    extern const Name PSC_Unit_amplitude;       //!< Scaling of PSC (current homeostasis)
    extern const Name published;                //!< Parameters for MUSIC devices

    extern const Name q_rr;                     //!< Other adaptation
    extern const Name q_sfa;                    //!< Other adaptation

    extern const Name rate;                     //!< Specific to ppd_sup_generator and gamma_sup_generator
    extern const Name receptor_type;            //!< Connection parameters
    extern const Name receptor_types;           //!< Connection parameters
    extern const Name record_from;              //!< Recorder parameter
    extern const Name record_to;                //!< Recorder parameter
    extern const Name recordables;              //!< List of recordable state data (Device parameters)
    extern const Name recorder;                 //!< Node type
    extern const Name refractory_input;         //!< Spikes arriving during refractory period are counted (precise timing neurons)
    extern const Name registered;               //!< Parameters for MUSIC devices
    extern const Name rho_0;                    //!< Specific to population point process model (pp_pop_psc_delta)
    extern const Name rms;                      //!< Root mean square
    extern const Name root_finding_epsilon;     //!< Accuracy of the root of the polynomial (precise timing neurons (Brette 2007))
    extern const Name rport;                    //!< Connection parameters
    extern const Name rports;                   //!< Connection parameters
    extern const Name rule;                     //!< Connectivity-related

    extern const Name S;                        //!< Binary state (output) of neuron (Ginzburg neuron)
    extern const Name scientific;               //!< Recorder parameter
    extern const Name screen;                   //!< Recorder parameter
    extern const Name senders;                  //!< Recorder parameter
    extern const Name size_of;                  //!< Connection parameters
    extern const Name source;                   //!< Connection parameters
    extern const Name spike;                    //!< true if the neuron spikes and false if not. (sli_neuron)
    extern const Name spike_times;              //!< Recorder parameter
    extern const Name start;                    //!< Device parameters
    extern const Name state;                    //!< Node parameter
    extern const Name std;                      //!< Miscellaneous parameters
    extern const Name std_mod;                  //!< Miscellaneous parameters
    extern const Name stimulator;               //!< Node type
    extern const Name stop;                     //!< Device parameters
    extern const Name structure;                //!< Node type
    extern const Name success;
    extern const Name synapse;                  //!< Node type
    extern const Name synapse_model;            //!< Connection parameters
    extern const Name synapse_modelid;          //!< Connection parameters

    extern const Name t_lag;                    //!< Lag within a time slice
    extern const Name t_origin;                 //!< Origin of a time-slice
    extern const Name t_ref;                    //!< Refractory period
    extern const Name t_ref_abs;                //!< Absolute refractory period
    extern const Name t_ref_remaining;          //!< Time remaining till end of refractory state
    extern const Name t_ref_tot;                //!< Total refractory period
    extern const Name t_spike;                  //!< Time of last spike
    extern const Name target;                   //!< Connection parameters
    extern const Name target_thread;            //!< Connection parameters
    extern const Name targets;                  //!< Connection parameters
    extern const Name tau_1;                    //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name tau_2;                    //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name tau_ahp;                  //!< Specific to iaf_chxk_2008 neuron
    extern const Name tau_epsp;                 //!< Specific to iaf_chs_2008 neuron
    extern const Name tau_fac;                  //!< facilitation time constant (ms) (Tsodyks2_connection)
    extern const Name tau_facs;                 //!< facilitation time constant (ms) (property arrays)
    extern const Name tau_lcm;                  //!< Least common multiple of tau_m, tau_ex and tau_in (precise timing neurons (Brette 2007))
    extern const Name tau_m;                    //!< Membrane time constant
    extern const Name tau_max;                  //!< Specific to correlation_and correlomatrix detector
    extern const Name tau_minus;                //!< used for ArchivingNode
    extern const Name tau_minus_triplet;        //!< used for ArchivingNode
    extern const Name tau_rec;                  //!< time constant for recovery (ms) (Tsodyks2_connection)
    extern const Name tau_recs;                 //!< time constant for recovery (ms) (property arrays)
    extern const Name tau_reset;                //!< Specific to iaf_chs_2008 neuron
    extern const Name tau_rr;                   //!< Other adaptation
    extern const Name tau_sfa;                  //!< Other adaptation
    extern const Name tau_syn;                  //!< Synapse time constant
    extern const Name tau_syn_ex;               //!< Excitatory synaptic time constant
    extern const Name tau_syn_in;               //!< Inhibitory synaptic time constant
    extern const Name tau_v;                    //!< Specific to amat2_*
    extern const Name tau_w;                    //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
    extern const Name taus_eta;                 //!< Specific to population point process model (pp_pop_psc_delta)
    extern const Name taus_syn;                 //!< Synapse time constants (array)
    extern const Name theta;                    //!< Did not compile without (theta neuron problem)
    extern const Name thread;                   //!< Node parameter
    extern const Name thread_local_id;          //!< Thead-local ID of node, see Kunkel et al 2014, Sec 3.3.2
    extern const Name time_in_steps;            //!< Recorder parameter
    extern const Name times;                    //!< Recorder parameter
    extern const Name to_accumulator;           //!< Recorder parameter
    extern const Name to_file;                  //!< Recorder parameter
    extern const Name to_memory;                //!< Recorder parameter
    extern const Name to_screen;                //!< Recorder parameter
    extern const Name Tstart;                   //!< Specific to correlation_and correlomatrix detector
    extern const Name Tstop;                    //!< Specific to correlation_and correlomatrix detector

    extern const Name u;                        //!< probability of release [0...1] (Tsodyks2_connection)
    extern const Name U_lower;
    extern const Name U_m;                      //!< Specific to Izhikevich 2003
    extern const Name U_mean;
    extern const Name U_std;
    extern const Name U_upper;
    extern const Name update;                   //!< Command to execute the neuron (sli_neuron)
    extern const Name update_node;              //!< Command to execute the neuron (sli_neuron)
    extern const Name us;                       //!< probability of release [0...1] (property arrays)

    extern const Name V_epsp;                   //!< Specific to iaf_chs_2008 neuron
    extern const Name V_m;                      //!< Membrane potential
    extern const Name V_min;                    //!< Absolute lower value for the membrane potential
    extern const Name V_noise;                  //!< Specific to iaf_chs_2008 neuron
    extern const Name V_peak;                   //!< Spike detection threshold (Brette & Gerstner 2005)
    extern const Name V_reset;                  //!< Reset potential
    extern const Name V_th;                     //!< Threshold
    extern const Name V_th_alpha_1;             //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name V_th_alpha_2;             //!< Specific to Kobayashi, Tsubo, Shinomoto 2009
    extern const Name V_th_v;					//!< Specific to amat2_*
    extern const Name vals_eta;                 //!< Specific to population point process model (pp_pop_psc_delta)
    extern const Name vp;                       //!< Node parameter

    extern const Name w;                        //!< Specific to Brette & Gerstner 2005 (aeif_cond-*)
    extern const Name weight;                   //!< Connection parameters
    extern const Name weight_std;               //!< Standard deviation/mean of noisy synapse.
    extern const Name weighted_spikes_ex;       //!< Weighted incoming excitatory spikes
    extern const Name weighted_spikes_in;       //!< Weighted incoming inhibitory spikes
    extern const Name weights;                  //!< Connection parameters
    extern const Name with_noise;
    extern const Name with_reset;               //!< Shall the pp_neuron reset after each spike? (stochastic neuron pp_psc_delta)
    extern const Name withgid;                  //!< Recorder parameter
    extern const Name withpath;                 //!< Recorder parameter
    extern const Name withtime;                 //!< Recorder parameter
    extern const Name withweight;               //!< Recorder parameter

    extern const Name x;                        //!< current scaling factor of the synaptic weight [0...1] (Tsodyks2_connection)
    extern const Name xs;                       //!< current scaling factor of the synaptic weight [0...1] (property arrays)

  }
}

#endif
