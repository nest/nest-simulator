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

// Generated includes:
#include "config.h"

// C++ includes:
#include <string>

namespace nest
{

/**
 * This namespace contains global name std::string objects. These can be
 * used in Node::get_status and Node::set_status.
 *
 * The Name declarations below are sorted alphabetically with lower case
 * letters preceding upper case letters.
 *
 * See testsuite/unittests/test_unused_names.py for a test that checks
 * if all name strings defined are also actually used.
 */
namespace names
{
const std::string A_LTD( "A_LTD" );
const std::string A_LTD_const( "A_LTD_const" );
const std::string A_LTP( "A_LTP" );
const std::string A_minus( "A_minus" );
const std::string A_plus( "A_plus" );
const std::string Act_m( "Act_m" );
const std::string Act_n( "Act_n" );
const std::string AMPA( "AMPA" );
const std::string Aminus( "Aminus" );
const std::string Aminus_triplet( "Aminus_triplet" );
const std::string Aplus( "Aplus" );
const std::string Aplus_triplet( "Aplus_triplet" );
const std::string ASCurrents( "ASCurrents" );
const std::string ASCurrents_sum( "ASCurrents_sum" );
const std::string a( "a" );
const std::string a_acausal( "a_acausal" );
const std::string a_causal( "a_causal" );
const std::string a_thresh_th( "a_thresh_th" );
const std::string a_thresh_tl( "a_thresh_tl" );
const std::string acceptable_latency( "acceptable_latency" );
const std::string activity( "activity" );
const std::string adapt_beta( "adapt_beta" );
const std::string adapt_tau( "adapt_tau" );
const std::string adaptation( "adaptation" );
const std::string adapting_threshold( "adapting_threshold" );
const std::string adaptive_target_buffers( "adaptive_target_buffers" );
const std::string add_compartments( "add_compartments" );
const std::string add_receptors( "add_receptors" );
const std::string after_spike_currents( "after_spike_currents" );
const std::string ahp_bug( "ahp_bug" );
const std::string allow_autapses( "allow_autapses" );
const std::string allow_multapses( "allow_multapses" );
const std::string allow_offgrid_times( "allow_offgrid_times" );
const std::string allow_oversized_mask( "allow_oversized_mask" );
const std::string alpha( "alpha" );
const std::string alpha_1( "alpha_1" );
const std::string alpha_2( "alpha_2" );
const std::string amp_slow( "amp_slow" );
const std::string amplitude( "amplitude" );
const std::string amplitude_times( "amplitude_times" );
const std::string amplitude_values( "amplitude_values" );
const std::string anchor( "anchor" );
const std::string archiver_length( "archiver_length" );
const std::string asc_amps( "asc_amps" );
const std::string asc_decay( "asc_decay" );
const std::string asc_init( "asc_init" );
const std::string asc_r( "asc_r" );
const std::string available( "available" );
const std::string average_gradient( "average_gradient" );
const std::string azimuth_angle( "azimuth_angle" );

const std::string b( "b" );
const std::string batch_size( "batch_size" );
const std::string beta( "beta" );
const std::string beta_1( "beta_1" );
const std::string beta_2( "beta_2" );
const std::string beta_Ca( "beta_Ca" );
const std::string biological_time( "biological_time" );
const std::string box( "box" );
const std::string buffer_size( "buffer_size" );
const std::string buffer_size_spike_data( "buffer_size_spike_data" );
const std::string buffer_size_target_data( "buffer_size_target_data" );

const std::string C_m( "C_m" );
const std::string Ca( "Ca" );
const std::string Ca_astro( "Ca_astro" );
const std::string Ca_tot( "Ca_tot" );
const std::string c( "c" );
const std::string c_1( "c_1" );
const std::string c_2( "c_2" );
const std::string c_3( "c_3" );
const std::string c_reg( "c_reg" );
const std::string capacity( "capacity" );
const std::string center( "center" );
const std::string circular( "circular" );
const std::string clear( "clear" );
const std::string comp_idx( "comp_idx" );
const std::string comparator( "comparator" );
const std::string compartments( "compartments" );
const std::string conc_Mg2( "conc_Mg2" );
const std::string configbit_0( "configbit_0" );
const std::string configbit_1( "configbit_1" );
const std::string connection_count( "connection_count" );
const std::string connection_rules( "connection_rules" );
const std::string connection_type( "connection_type" );
const std::string consistent_integration( "consistent_integration" );
const std::string continuous( "continuous" );
const std::string count_covariance( "count_covariance" );
const std::string count_histogram( "count_histogram" );
const std::string covariance( "covariance" );

const std::string Delta_T( "Delta_T" );
const std::string Delta_V( "Delta_V" );
const std::string d( "d" );
const std::string data( "data" );
const std::string data_path( "data_path" );
const std::string data_prefix( "data_prefix" );
const std::string dead_time( "dead_time" );
const std::string dead_time_random( "dead_time_random" );
const std::string dead_time_shape( "dead_time_shape" );
const std::string delay( "delay" );
const std::string delay_u_bars( "delay_u_bars" );
const std::string deliver_interval( "deliver_interval" );
const std::string delta( "delta" );
const std::string delta_IP3( "delta_IP3" );
const std::string delta_P( "delta_P" );
const std::string delta_tau( "delta_tau" );
const std::string dendritic_curr( "dendritic_curr" );
const std::string dendritic_exc( "dendritic_exc" );
const std::string dendritic_inh( "dendritic_inh" );
const std::string dg( "dg" );
const std::string dg_ex( "dg_ex" );
const std::string dg_in( "dg_in" );
const std::string dI_syn_ex( "dI_syn_ex" );
const std::string dI_syn_in( "dI_syn_in" );
const std::string dict_miss_is_error( "dict_miss_is_error" );
const std::string diffusion_factor( "diffusion_factor" );
const std::string dimension( "dimension" );
const std::string distal_curr( "distal_curr" );
const std::string distal_exc( "distal_exc" );
const std::string distal_inh( "distal_inh" );
const std::string drift_factor( "drift_factor" );
const std::string driver_readout_time( "driver_readout_time" );
const std::string dt( "dt" );
const std::string dU( "U" );

const std::string E_ahp( "E_ahp" );
const std::string E_ex( "E_ex" );
const std::string E_in( "E_in" );
const std::string E_K( "E_K" );
const std::string E_L( "E_L" );
const std::string E_Na( "E_Na" );
const std::string E_rev( "E_rev" );
const std::string E_rev_AMPA( "E_rev_AMPA" );
const std::string E_rev_GABA_A( "E_rev_GABA_A" );
const std::string E_rev_GABA_B( "E_rev_GABA_B" );
const std::string E_rev_h( "E_rev_h" );
const std::string E_rev_KNa( "E_rev_KNa" );
const std::string E_rev_NaP( "E_rev_NaP" );
const std::string E_rev_NMDA( "E_rev_NMDA" );
const std::string E_rev_T( "E_rev_T" );
const std::string E_rr( "E_rr" );
const std::string E_sfa( "E_sfa" );
const std::string e_L( "e_L" );
const std::string edge_wrap( "edge_wrap" );
const std::string element_type( "element_type" );
const std::string elements( "elements" );
const std::string elementsize( "elementsize" );
const std::string ellipsoidal( "ellipsoidal" );
const std::string elliptical( "elliptical" );
const std::string eprop_history_duration( "eprop_history_duration" );
const std::string eprop_isi_trace_cutoff( "eprop_isi_trace_cutoff" );
const std::string eprop_learning_window( "eprop_learning_window" );
const std::string eprop_reset_neurons_on_update( "eprop_reset_neurons_on_update" );
const std::string eprop_update_interval( "eprop_update_interval" );
const std::string eps( "eps" );
const std::string epsilon( "epsilon" );
const std::string equilibrate( "equilibrate" );
const std::string error_signal( "error_signal" );
const std::string eta( "eta" );
const std::string events( "events" );
const std::string extent( "extent" );

const std::string f_target( "f_target" );
const std::string file_extension( "file_extension" );
const std::string filename( "filename" );
const std::string filenames( "filenames" );
const std::string frequency( "frequency" );
const std::string frozen( "frozen" );

const std::string GABA( "GABA" );
const std::string GABA_A( "GABA_A" );
const std::string GABA_B( "GABA_B" );
const std::string g( "g" );
const std::string g_AMPA( "g_AMPA" );
const std::string g_ahp( "g_ahp" );
const std::string g_C( "g_C" );
const std::string g_ex( "g_ex" );
const std::string g_GABA_A( "g_GABA_A" );
const std::string g_GABA_B( "g_GABA_B" );
const std::string g_in( "g_in" );
const std::string g_K( "g_K" );
const std::string g_KL( "g_KL" );
const std::string g_Kv1( "g_Kv1" );
const std::string g_Kv3( "g_Kv3" );
const std::string g_L( "g_L" );
const std::string g_m( "g_m" );
const std::string g_Na( "g_Na" );
const std::string g_NaL( "g_NaL" );
const std::string g_NMDA( "g_NMDA" );
const std::string g_pd( "g_pd" );
const std::string g_peak_AMPA( "g_peak_AMPA" );
const std::string g_peak_GABA_A( "g_peak_GABA_A" );
const std::string g_peak_GABA_B( "g_peak_GABA_B" );
const std::string g_peak_h( "g_peak_h" );
const std::string g_peak_KNa( "g_peak_KNa" );
const std::string g_peak_NaP( "g_peak_NaP" );
const std::string g_peak_NMDA( "g_peak_NMDA" );
const std::string g_peak_T( "g_peak_T" );
const std::string g_ps( "g_ps" );
const std::string g_rr( "g_rr" );
const std::string g_sfa( "g_sfa" );
const std::string g_sp( "g_sp" );
const std::string gamma( "gamma" );
const std::string gamma_shape( "gamma_shape" );
const std::string gaussian( "gaussian" );
const std::string global_id( "global_id" );
const std::string global_max_spikes_sent( "global_max_spikes_sent" );
const std::string grid( "grid" );
const std::string grid3d( "grid3d" );
const std::string growth_curve( "growth_curve" );
const std::string growth_curves( "growth_curves" );
const std::string growth_factor_buffer_spike_data( "growth_factor_buffer_spike_data" );
const std::string growth_factor_buffer_target_data( "growth_factor_buffer_target_data" );
const std::string growth_rate( "growth_rate" );
const std::string gsl_error_tol( "gsl_error_tol" );

const std::string h( "h" );
const std::string h_IP3R( "h_IP3R" );
const std::string has_connections( "has_connections" );
const std::string has_delay( "has_delay" );
const std::string histogram( "histogram" );
const std::string histogram_correction( "histogram_correction" );

const std::string I( "I" );
const std::string I_ahp( "I_ahp" );
const std::string I_e( "I_e" );
const std::string I_h( "I_h" );
const std::string I_AMPA( "I_AMPA" );
const std::string I_GABA( "I_GABA" );
const std::string I_KNa( "I_KNa" );
const std::string I_NMDA( "I_NMDA" );
const std::string I_NaP( "I_NaP" );
const std::string I_SIC( "I_SIC" );
const std::string I_sp( "I_sp" );
const std::string I_stc( "I_stc" );
const std::string I_syn( "I_syn" );
const std::string I_syn_ex( "I_syn_ex" );
const std::string I_syn_in( "I_syn_in" );
const std::string I_T( "I_T" );
const std::string Inact_h( "Inact_h" );
const std::string Inact_p( "Inact_p" );
const std::string IP3( "IP3" );
const std::string IP3_0( "IP3_0" );
const std::string indegree( "indegree" );
const std::string index_map( "index_map" );
const std::string individual_spike_trains( "individual_spike_trains" );
const std::string init_flag( "init_flag" );
const std::string inner_radius( "inner_radius" );
const std::string instant_unblock_NMDA( "instant_unblock_NMDA" );
const std::string instantiations( "instantiations" );
const std::string interval( "interval" );
const std::string is_refractory( "is_refractory" );

const std::string Kd_act( "Kd_act" );
const std::string Kd_IP3_1( "Kd_IP3_1" );
const std::string Kd_IP3_2( "Kd_IP3_2" );
const std::string Kd_inh( "Kd_inh" );
const std::string Km_SERCA( "Km_SERCA" );
const std::string Kplus( "Kplus" );
const std::string Kplus_triplet( "Kplus_triplet" );
const std::string k_IP3R( "k_IP3R" );
const std::string kappa( "kappa" );
const std::string kappa_reg( "kappa_reg" );
const std::string keep_source_table( "keep_source_table" );
const std::string kernel( "kernel" );

const std::string label( "label" );
const std::string lambda( "lambda" );
const std::string lambda_0( "lambda_0" );
const std::string learning_signal( "learning_signal" );
const std::string len_kernel( "len_kernel" );
const std::string linear( "linear" );
const std::string linear_summation( "linear_summation" );
const std::string local( "local" );
const std::string local_num_threads( "local_num_threads" );
const std::string local_spike_counter( "local_spike_counter" );
const std::string lookuptable_0( "lookuptable_0" );
const std::string lookuptable_1( "lookuptable_1" );
const std::string lookuptable_2( "lookuptable_2" );
const std::string loss( "loss" );
const std::string lower_left( "lower_left" );

const std::string m( "m" );
const std::string major_axis( "major_axis" );
const std::string make_symmetric( "make_symmetric" );
const std::string mask( "mask" );
const std::string max( "max" );
const std::string max_buffer_size_target_data( "max_buffer_size_target_data" );
const std::string max_delay( "max_delay" );
const std::string max_num_syn_models( "max_num_syn_models" );
const std::string max_update_time( "max_update_time" );
const std::string mean( "mean" );
const std::string memory( "memory" );
const std::string message_times( "messages_times" );
const std::string messages( "messages" );
const std::string min( "min" );
const std::string min_delay( "min_delay" );
const std::string min_update_time( "min_update_time" );
const std::string minor_axis( "minor_axis" );
const std::string model( "model" );
const std::string model_id( "model_id" );
const std::string modules( "modules" );
const std::string mpi_address( "mpi_address" );
const std::string mpi_rank( "mpi_rank" );
const std::string ms_per_tic( "ms_per_tic" );
const std::string mu( "mu" );
const std::string mu_minus( "mu_minus" );
const std::string mu_plus( "mu_plus" );
const std::string mult_coupling( "mult_coupling" );
const std::string music_channel( "music_channel" );

const std::string N( "N" );
const std::string NMDA( "NMDA" );
const std::string N_channels( "N_channels" );
const std::string N_NaP( "N_NaP" );
const std::string N_T( "N_T" );
const std::string n( "n" );
const std::string n_events( "n_events" );
const std::string n_messages( "n_messages" );
const std::string n_proc( "n_proc" );
const std::string n_receptors( "n_receptors" );
const std::string n_synapses( "n_synapses" );
const std::string network_size( "network_size" );
const std::string neuron( "neuron" );
const std::string new_buffer_size( "new_buffer_size" );
const std::string next_readout_time( "next_readout_time" );
const std::string no_synapses( "no_synapses" );
const std::string node_models( "node_models" );
const std::string node_uses_wfr( "node_uses_wfr" );
const std::string noise( "noise" );
const std::string noisy_rate( "noisy_rate" );
const std::string num_connections( "num_connections" );
const std::string num_processes( "num_processes" );
const std::string number_of_connections( "number_of_connections" );

const std::string off_grid_spiking( "off_grid_spiking" );
const std::string offset( "offset" );
const std::string offsets( "offsets" );
const std::string omega( "omega" );
const std::string optimize_each_step( "optimize_each_step" );
const std::string optimizer( "optimizer" );
const std::string order( "order" );
const std::string origin( "origin" );
const std::string other( "other" );
const std::string outdegree( "outdegree" );
const std::string outer_radius( "outer_radius" );
const std::string overwrite_files( "overwrite_files" );

const std::string P( "P" );
const std::string p( "p" );
const std::string p_copy( "p_copy" );
const std::string p_transmit( "p_transmit" );
const std::string pairwise_bernoulli_on_source( "pairwise_bernoulli_on_source" );
const std::string pairwise_bernoulli_on_target( "pairwise_bernoulli_on_target" );
const std::string pairwise_avg_num_conns( "pairwise_avg_num_conns" );
const std::string params( "params" );
const std::string parent_idx( "parent_idx" );
const std::string phase( "phase" );
const std::string phi_max( "phi_max" );
const std::string pairwise_poisson( "pairwise_poisson" );
const std::string polar_angle( "polar_angle" );
const std::string polar_axis( "polar_axis" );
const std::string pool_size( "pool_size" );
const std::string pool_type( "pool_type" );
const std::string port( "port" );
const std::string port_name( "port_name" );
const std::string port_width( "port_width" );
const std::string ports( "ports" );
const std::string positions( "positions" );
const std::string post_synaptic_element( "post_synaptic_element" );
const std::string post_trace( "post_trace" );
const std::string pre_synaptic_element( "pre_synaptic_element" );
const std::string precise_times( "precise_times" );
const std::string precision( "precision" );
const std::string prepared( "prepared" );
const std::string primary( "primary" );
const std::string print_time( "print_time" );
const std::string proximal_curr( "proximal_curr" );
const std::string proximal_exc( "proximal_exc" );
const std::string proximal_inh( "proximal_inh" );
const std::string psi( "psi" );
const std::string published( "published" );
const std::string pulse_times( "pulse_times" );

const std::string q_rr( "q_rr" );
const std::string q_sfa( "q_sfa" );
const std::string q_stc( "q_stc" );

const std::string radius( "radius" );
const std::string rate( "rate" );
const std::string rate_IP3R( "rate_IP3R" );
const std::string rate_L( "rate_L" );
const std::string rate_SERCA( "rate_SERCA" );
const std::string rate_slope( "rate_slope" );
const std::string rate_times( "rate_times" );
const std::string rate_values( "rate_values" );
const std::string ratio_ER_cyt( "ratio_ER_cyt" );
const std::string readout_cycle_duration( "readout_cycle_duration" );
const std::string readout_signal( "readout_signal" );
const std::string readout_signal_unnorm( "readout_signal_unnorm" );
const std::string receptor_idx( "receptor_idx" );
const std::string receptor_type( "receptor_type" );
const std::string receptor_types( "receptor_types" );
const std::string receptors( "receptors" );
const std::string record_from( "record_from" );
const std::string record_to( "record_to" );
const std::string recordables( "recordables" );
const std::string recorder( "recorder" );
const std::string recording_backends( "recording_backends" );
const std::string rectangular( "rectangular" );
const std::string rectify_output( "rectify_output" );
const std::string rectify_rate( "rectify_rate" );
const std::string recv_buffer_size_secondary_events( "recv_buffer_size_secondary_events" );
const std::string refractory_input( "refractory_input" );
const std::string registered( "registered" );
const std::string regular_spike_arrival( "regular_spike_arrival" );
const std::string relative_amplitude( "relative_amplitude" );
const std::string requires_symmetric( "requires_symmetric" );
const std::string reset_pattern( "reset_pattern" );
const std::string resolution( "resolution" );
const std::string rho( "rho" );
const std::string rng_seed( "rng_seed" );
const std::string rng_type( "rng_type" );
const std::string rng_types( "rng_types" );
const std::string rport( "receptor" );
const std::string rule( "rule" );

const std::string S( "S" );
const std::string S_act_NMDA( "S_act_NMDA" );
const std::string s_NMDA( "s_NMDA" );
const std::string s_AMPA( "s_AMPA" );
const std::string s_GABA( "s_GABA" );
const std::string SIC_scale( "SIC_scale" );
const std::string SIC_th( "SIC_th" );
const std::string sdev( "sdev" );
const std::string send_buffer_size_secondary_events( "send_buffer_size_secondary_events" );
const std::string senders( "senders" );
const std::string shape( "shape" );
const std::string shift_now_spikes( "shift_now_spikes" );
const std::string shrink_factor_buffer_spike_data( "shrink_factor_buffer_spike_data" );
const std::string sigma( "sigma" );
const std::string sigmoid( "sigmoid" );
const std::string sion_chunksize( "sion_chunksize" );
const std::string sion_collective( "sion_collective" );
const std::string sion_n_files( "sion_n_files" );
const std::string size_of( "sizeof" );
const std::string soma_curr( "soma_curr" );
const std::string soma_exc( "soma_exc" );
const std::string soma_inh( "soma_inh" );
const std::string source( "source" );
const std::string spherical( "spherical" );
const std::string spike_buffer_grow_extra( "spike_buffer_grow_extra" );
const std::string spike_buffer_resize_log( "spike_buffer_resize_log" );
const std::string spike_buffer_shrink_limit( "spike_buffer_shrink_limit" );
const std::string spike_buffer_shrink_spare( "spike_buffer_shrink_spare" );
const std::string spike_dependent_threshold( "spike_dependent_threshold" );
const std::string spike_multiplicities( "spike_multiplicities" );
const std::string spike_times( "spike_times" );
const std::string spike_weights( "spike_weights" );
const std::string start( "start" );
const std::string state( "state" );
const std::string std( "std" );
const std::string std_mod( "std_mod" );
const std::string stimulation_backends( "stimulation_backends" );
const std::string stimulator( "stimulator" );
const std::string stimulus_source( "stimulus_source" );
const std::string stop( "stop" );
const std::string structural_plasticity_synapses( "structural_plasticity_synapses" );
const std::string structural_plasticity_update_interval( "structural_plasticity_update_interval" );
const std::string surrogate_gradient( "surrogate_gradient" );
const std::string surrogate_gradient_function( "surrogate_gradient_function" );
const std::string synapse_id( "synapse_id" );
const std::string synapse_label( "synapse_label" );
const std::string synapse_model( "synapse_model" );
const std::string synapse_modelid( "synapse_modelid" );
const std::string synapse_models( "synapse_models" );
const std::string synapse_parameters( "synapse_parameters" );
const std::string synapses_per_driver( "synapses_per_driver" );
const std::string synaptic_elements( "synaptic_elements" );
const std::string synaptic_elements_param( "synaptic_elements_param" );
const std::string synaptic_endpoint( "synaptic_endpoint" );

const std::string T_max( "T_max" );
const std::string T_min( "T_min" );
const std::string Tstart( "Tstart" );
const std::string Tstop( "Tstop" );
const std::string t_clamp( "t_clamp" );
const std::string t_ref( "t_ref" );
const std::string t_ref_abs( "t_ref_abs" );
const std::string t_ref_remaining( "t_ref_remaining" );
const std::string t_ref_tot( "t_ref_tot" );
const std::string t_spike( "t_spike" );
const std::string target( "target" );
const std::string target_signal( "target_signal" );
const std::string target_thread( "target_thread" );
const std::string targets( "targets" );
const std::string tau( "tau" );
const std::string tau_1( "tau_1" );
const std::string tau_2( "tau_2" );
const std::string tau_AMPA( "tau_AMPA" );
const std::string tau_Ca( "tau_Ca" );
const std::string tau_D_KNa( "tau_D_KNa" );
const std::string tau_Delta( "tau_Delta" );
const std::string tau_GABA( "tau_GABA" );
const std::string tau_Mg_fast_NMDA( "tau_Mg_fast_NMDA" );
const std::string tau_Mg_slow_NMDA( "tau_Mg_slow_NMDA" );
const std::string tau_P( "tau_P" );
const std::string tau_V_th( "tau_V_th" );
const std::string tau_ahp( "tau_ahp" );
const std::string tau_c( "tau_c" );
const std::string tau_decay( "tau_decay" );
const std::string tau_decay_AMPA( "tau_decay_AMPA" );
const std::string tau_decay_ex( "tau_decay_ex" );
const std::string tau_decay_GABA_A( "tau_decay_GABA_A" );
const std::string tau_decay_GABA_B( "tau_decay_GABA_B" );
const std::string tau_decay_in( "tau_decay_in" );
const std::string tau_decay_NMDA( "tau_decay_NMDA" );
const std::string tau_epsp( "tau_epsp" );
const std::string tau_fac( "tau_fac" );
const std::string tau_IP3( "tau_IP3" );
const std::string tau_m( "tau_m" );
const std::string tau_max( "tau_max" );
const std::string tau_minus( "tau_minus" );
const std::string tau_minus_stdp( "tau_minus_stdp" );
const std::string tau_minus_triplet( "tau_minus_triplet" );
const std::string tau_m_readout( "tau_m_readout" );
const std::string tau_n( "tau_n" );
const std::string tau_plus( "tau_plus" );
const std::string tau_plus_triplet( "tau_plus_triplet" );
const std::string tau_psc( "tau_psc" );
const std::string tau_rec( "tau_rec" );
const std::string tau_reset( "tau_reset" );
const std::string tau_rise( "tau_rise" );
const std::string tau_rise_AMPA( "tau_rise_AMPA" );
const std::string tau_rise_ex( "tau_rise_ex" );
const std::string tau_rise_GABA_A( "tau_rise_GABA_A" );
const std::string tau_rise_GABA_B( "tau_rise_GABA_B" );
const std::string tau_rise_in( "tau_rise_in" );
const std::string tau_rise_NMDA( "tau_rise_NMDA" );
const std::string tau_rr( "tau_rr" );
const std::string tau_sfa( "tau_sfa" );
const std::string tau_spike( "tau_spike" );
const std::string tau_stc( "tau_stc" );
const std::string tau_syn( "tau_syn" );
const std::string tau_syn_ex( "tau_syn_ex" );
const std::string tau_syn_fast( "tau_syn_fast" );
const std::string tau_syn_in( "tau_syn_in" );
const std::string tau_syn_slow( "tau_syn_slow" );
const std::string tau_theta( "tau_theta" );
const std::string tau_u_bar_bar( "tau_u_bar_bar" );
const std::string tau_u_bar_minus( "tau_u_bar_minus" );
const std::string tau_u_bar_plus( "tau_u_bar_plus" );
const std::string tau_v( "tau_v" );
const std::string tau_vacant( "tau_vacant" );
const std::string tau_w( "tau_w" );
const std::string tau_x( "tau_x" );
const std::string tau_z( "tau_z" );
const std::string th_spike_add( "th_spike_add" );
const std::string th_spike_decay( "th_spike_decay" );
const std::string th_voltage_decay( "th_voltage_decay" );
const std::string th_voltage_index( "th_voltage_index" );
const std::string theta( "theta" );
const std::string theta_eq( "theta_eq" );
const std::string theta_ex( "theta_ex" );
const std::string theta_in( "theta_in" );
const std::string theta_minus( "theta_minus" );
const std::string theta_plus( "theta_plus" );
const std::string third_in( "third_in" );
const std::string third_out( "third_out" );
const std::string thread( "thread" );
const std::string thread_local_id( "thread_local_id" );
const std::string threshold( "threshold" );
const std::string threshold_spike( "threshold_spike" );
const std::string threshold_voltage( "threshold_voltage" );
const std::string tics_per_ms( "tics_per_ms" );
const std::string tics_per_step( "tics_per_step" );
const std::string time_collocate_spike_data( "time_collocate_spike_data" );
const std::string time_collocate_spike_data_cpu( "time_collocate_spike_data_cpu" );
const std::string time_communicate_prepare( "time_communicate_prepare" );
const std::string time_communicate_prepare_cpu( "time_communicate_prepare_cpu" );
const std::string time_communicate_spike_data( "time_communicate_spike_data" );
const std::string time_communicate_spike_data_cpu( "time_communicate_spike_data_cpu" );
const std::string time_communicate_target_data( "time_communicate_target_data" );
const std::string time_communicate_target_data_cpu( "time_communicate_target_data_cpu" );
const std::string time_construction_connect( "time_construction_connect" );
const std::string time_construction_connect_cpu( "time_construction_connect_cpu" );
const std::string time_construction_create( "time_construction_create" );
const std::string time_construction_create_cpu( "time_construction_create_cpu" );
const std::string time_deliver_secondary_data( "time_deliver_secondary_data" );
const std::string time_deliver_secondary_data_cpu( "time_deliver_secondary_data_cpu" );
const std::string time_deliver_spike_data( "time_deliver_spike_data" );
const std::string time_deliver_spike_data_cpu( "time_deliver_spike_data_cpu" );
const std::string time_gather_secondary_data( "time_gather_secondary_data" );
const std::string time_gather_secondary_data_cpu( "time_gather_secondary_data_cpu" );
const std::string time_gather_spike_data( "time_gather_spike_data" );
const std::string time_gather_spike_data_cpu( "time_gather_spike_data_cpu" );
const std::string time_gather_target_data( "time_gather_target_data" );
const std::string time_gather_target_data_cpu( "time_gather_target_data_cpu" );
const std::string time_in_steps( "time_in_steps" );
const std::string time_mpi_synchronization( "time_mpi_synchronization" );
const std::string time_mpi_synchronization_cpu( "time_mpi_synchronization_cpu" );
const std::string time_omp_synchronization_construction( "time_omp_synchronization_construction" );
const std::string time_omp_synchronization_construction_cpu( "time_omp_synchronization_construction_cpu" );
const std::string time_omp_synchronization_simulation( "time_omp_synchronization_simulation" );
const std::string time_omp_synchronization_simulation_cpu( "time_omp_synchronization_simulation_cpu" );
const std::string time_simulate( "time_simulate" );
const std::string time_simulate_cpu( "time_simulate_cpu" );
const std::string time_update( "time_update" );
const std::string time_update_cpu( "time_update_cpu" );
const std::string times( "times" );
const std::string to_do( "to_do" );
const std::string total_num_virtual_procs( "total_num_virtual_procs" );
const std::string type( "type" );
const std::string type_id( "type_id" );

const std::string U( "U" );
const std::string U_m( "U_m" );
const std::string u( "u" );
const std::string u_bar_bar( "u_bar_bar" );
const std::string u_bar_minus( "u_bar_minus" );
const std::string u_bar_plus( "u_bar_plus" );
const std::string u_ref_squared( "u_ref_squared" );
const std::string update_time_limit( "update_time_limit" );
const std::string upper_right( "upper_right" );
const std::string use_compressed_spikes( "use_compressed_spikes" );
const std::string use_wfr( "use_wfr" );

const std::string v( "v" );
const std::string V_act_NMDA( "V_act_NMDA" );
const std::string V_clamp( "V_clamp" );
const std::string v_comp( "v_comp" );
const std::string V_epsp( "V_epsp" );
const std::string V_m( "V_m" );
const std::string V_min( "V_min" );
const std::string V_noise( "V_noise" );
const std::string V_peak( "V_peak" );
const std::string V_reset( "V_reset" );
const std::string V_T( "V_T" );
const std::string V_T_star( "V_T_star" );
const std::string V_th( "V_th" );
const std::string V_th_adapt( "V_th_adapt" );
const std::string V_th_alpha_1( "V_th_alpha_1" );
const std::string V_th_alpha_2( "V_th_alpha_2" );
const std::string V_th_max( "V_th_max" );
const std::string V_th_rest( "V_th_rest" );
const std::string V_th_v( "V_th_v" );
const std::string verbosity( "verbosity" );
const std::string voltage_clamp( "voltage_clamp" );
const std::string voltage_reset_add( "voltage_reset_add" );
const std::string voltage_reset_fraction( "voltage_reset_fraction" );
const std::string volume_transmitter( "volume_transmitter" );
const std::string vp( "vp" );

const std::string Wmax( "Wmax" );
const std::string Wmin( "Wmin" );
const std::string w( "w" );
const std::string weight( "weight" );
const std::string weight_per_lut_entry( "weight_per_lut_entry" );
const std::string weight_recorder( "weight_recorder" );
const std::string weights( "weights" );
const std::string wfr_comm_interval( "wfr_comm_interval" );
const std::string wfr_interpolation_order( "wfr_interpolation_order" );
const std::string wfr_max_iterations( "wfr_max_iterations" );
const std::string wfr_tol( "wfr_tol" );
const std::string with_reset( "with_reset" );

const std::string x( "x" );
const std::string x_bar( "x_bar" );

const std::string y( "y" );
const std::string y_0( "y_0" );
const std::string y_1( "y_1" );

const std::string z( "z" );
const std::string z_connected( "z_connected" );

} // namespace names

} // namespace nest

#endif /* #ifndef NEST_NAMES_H */
