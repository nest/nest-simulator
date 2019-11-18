/*
 *  nest_names.cpp
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
#include "nest_names.h"

namespace nest
{

namespace names
{

const Name a( "a" );
const Name a_acausal( "a_acausal" );
const Name a_causal( "a_causal" );
const Name A_LTD( "A_LTD" );
const Name A_LTD_const( "A_LTD_const" );
const Name A_LTP( "A_LTP" );
const Name A_minus( "A_minus" );
const Name A_plus( "A_plus" );
const Name a_thresh_th( "a_thresh_th" );
const Name a_thresh_tl( "a_thresh_tl" );
const Name acceptable_latency( "acceptable_latency" );
const Name Act_m( "Act_m" );
const Name Act_n( "Act_n" );
const Name activity( "activity" );
const Name adaptive_spike_buffers( "adaptive_spike_buffers" );
const Name adaptive_target_buffers( "adaptive_target_buffers" );
const Name ahp_bug( "ahp_bug" );
const Name allow_autapses( "allow_autapses" );
const Name allow_multapses( "allow_multapses" );
const Name allow_offgrid_times( "allow_offgrid_times" );
const Name alpha( "alpha" );
const Name alpha_1( "alpha_1" );
const Name alpha_2( "alpha_2" );
const Name Aminus( "Aminus" );
const Name Aminus_triplet( "Aminus_triplet" );
const Name AMPA( "AMPA" );
const Name amplitude( "amplitude" );
const Name amplitude_times( "amplitude_times" );
const Name amplitude_values( "amplitude_values" );
const Name Aplus( "Aplus" );
const Name Aplus_triplet( "Aplus_triplet" );
const Name archiver_length( "archiver_length" );
const Name available( "available" );

const Name b( "b" );
const Name beta( "beta" );
const Name beta_Ca( "beta_Ca" );
const Name buffer_size( "buffer_size" );
const Name buffer_size_secondary_events( "buffer_size_secondary_events" );
const Name buffer_size_spike_data( "buffer_size_spike_data" );
const Name buffer_size_target_data( "buffer_size_target_data" );

const Name c( "c" );
const Name c_1( "c_1" );
const Name c_2( "c_2" );
const Name c_3( "c_3" );
const Name C_m( "C_m" );
const Name Ca( "Ca" );
const Name calibrate( "calibrate" );
const Name calibrate_node( "calibrate_node" );
const Name capacity( "capacity" );
const Name clear( "clear" );
const Name comparator( "comparator" );
const Name configbit_0( "configbit_0" );
const Name configbit_1( "configbit_1" );
const Name connection_count( "connection_count" );
const Name consistent_integration( "consistent_integration" );
const Name continuous( "continuous" );
const Name count_covariance( "count_covariance" );
const Name count_histogram( "count_histogram" );
const Name covariance( "covariance" );
const Name currents( "currents" );

const Name d( "d" );
const Name data( "data" );
const Name data_path( "data_path" );
const Name data_prefix( "data_prefix" );
const Name dead_time( "dead_time" );
const Name dead_time_random( "dead_time_random" );
const Name dead_time_shape( "dead_time_shape" );
const Name delay( "delay" );
const Name delay_u_bars( "delay_u_bars" );
const Name deliver_interval( "deliver_interval" );
const Name delta( "delta" );
const Name delta_P( "delta_P" );
const Name Delta_T( "Delta_T" );
const Name delta_tau( "delta_tau" );
const Name delta_u( "delta_u" );
const Name Delta_V( "Delta_V" );
const Name dg( "dg" );
const Name dg_ex( "dg_ex" );
const Name dg_in( "dg_in" );
const Name dI_syn_ex( "dI_syn_ex" );
const Name dI_syn_in( "dI_syn_in" );
const Name dict_miss_is_error( "dict_miss_is_error" );
const Name diffusion_factor( "diffusion_factor" );
const Name dimension( "dimension" );
const Name distal_curr( "distal_curr" );
const Name distal_exc( "distal_exc" );
const Name distal_inh( "distal_inh" );
const Name distribution( "distribution" );
const Name drift_factor( "drift_factor" );
const Name driver_readout_time( "driver_readout_time" );
const Name dt( "dt" );
const Name dU( "U" );

const Name E_ahp( "E_ahp" );
const Name E_ex( "E_ex" );
const Name E_in( "E_in" );
const Name E_K( "E_K" );
const Name E_L( "E_L" );
const Name E_Na( "E_Na" );
const Name E_rev( "E_rev" );
const Name E_rev_AMPA( "E_rev_AMPA" );
const Name E_rev_GABA_A( "E_rev_GABA_A" );
const Name E_rev_GABA_B( "E_rev_GABA_B" );
const Name E_rev_h( "E_rev_h" );
const Name E_rev_KNa( "E_rev_KNa" );
const Name E_rev_NaP( "E_rev_NaP" );
const Name E_rev_NMDA( "E_rev_NMDA" );
const Name E_rev_T( "E_rev_T" );
const Name E_rr( "E_rr" );
const Name E_sfa( "E_sfa" );
const Name element_type( "element_type" );
const Name elementsize( "elementsize" );
const Name eps( "eps" );
const Name equilibrate( "equilibrate" );
const Name error( "error" );
const Name eta( "eta" );
const Name events( "events" );
const Name ex_spikes( "ex_spikes" );

const Name file_extension( "file_extension" );
const Name filename( "filename" );
const Name filenames( "filenames" );
const Name frequency( "frequency" );
const Name frozen( "frozen" );

const Name g( "g" );
const Name g_ahp( "g_ahp" );
const Name g_AMPA( "g_AMPA" );
const Name g_ex( "g_ex" );
const Name g_GABA_A( "g_GABA_A" );
const Name g_GABA_B( "g_GABA_B" );
const Name g_in( "g_in" );
const Name g_K( "g_K" );
const Name g_KL( "g_KL" );
const Name g_Kv1( "g_Kv1" );
const Name g_Kv3( "g_Kv3" );
const Name g_L( "g_L" );
const Name g_Na( "g_Na" );
const Name g_NaL( "g_NaL" );
const Name g_NMDA( "g_NMDA" );
const Name g_pd( "g_pd" );
const Name g_peak_AMPA( "g_peak_AMPA" );
const Name g_peak_GABA_A( "g_peak_GABA_A" );
const Name g_peak_GABA_B( "g_peak_GABA_B" );
const Name g_peak_h( "g_peak_h" );
const Name g_peak_KNa( "g_peak_KNa" );
const Name g_peak_NaP( "g_peak_NaP" );
const Name g_peak_NMDA( "g_peak_NMDA" );
const Name g_peak_T( "g_peak_T" );
const Name g_rr( "g_rr" );
const Name g_sfa( "g_sfa" );
const Name g_sp( "g_sp" );
const Name GABA_A( "GABA_A" );
const Name GABA_B( "GABA_B" );
const Name gamma_shape( "gamma_shape" );
const Name gaussian( "gaussian" );
const Name global_id( "global_id" );
const Name grng( "grng" );
const Name grng_seed( "grng_seed" );
const Name growth_curve( "growth_curve" );
const Name growth_factor_buffer_spike_data( "growth_factor_buffer_spike_data" );
const Name growth_factor_buffer_target_data( "growth_factor_buffer_target_data" );
const Name growth_rate( "growth_rate" );
const Name gsl_error_tol( "gsl_error_tol" );

const Name h( "h" );
const Name has_connections( "has_connections" );
const Name has_delay( "has_delay" );
const Name histogram( "histogram" );
const Name histogram_correction( "histogram_correction" );

const Name I( "I" );
const Name I_ahp( "I_ahp" );
const Name I_e( "I_e" );
const Name I_h( "I_h" );
const Name I_KNa( "I_KNa" );
const Name I_NaP( "I_NaP" );
const Name I_sp( "I_sp" );
const Name I_stc( "I_stc" );
const Name I_syn( "I_syn" );
const Name I_syn_ex( "I_syn_ex" );
const Name I_syn_in( "I_syn_in" );
const Name I_T( "I_T" );
const Name in_spikes( "in_spikes" );
const Name Inact_h( "Inact_h" );
const Name Inact_p( "Inact_p" );
const Name indegree( "indegree" );
const Name index_map( "index_map" );
const Name individual_spike_trains( "individual_spike_trains" );
const Name init_flag( "init_flag" );
const Name instant_unblock_NMDA( "instant_unblock_NMDA" );
const Name instantiations( "instantiations" );
const Name Interpol_Order( "Interpol_Order" );
const Name interval( "interval" );
const Name is_refractory( "is_refractory" );

const Name keep_source_table( "keep_source_table" );
const Name Kplus( "Kplus" );
const Name Kplus_triplet( "Kplus_triplet" );

const Name label( "label" );
const Name lambda( "lambda" );
const Name lambda_0( "lambda_0" );
const Name len_kernel( "len_kernel" );
const Name linear( "linear" );
const Name linear_summation( "linear_summation" );
const Name local( "local" );
const Name local_num_threads( "local_num_threads" );
const Name local_spike_counter( "local_spike_counter" );
const Name lookuptable_0( "lookuptable_0" );
const Name lookuptable_1( "lookuptable_1" );
const Name lookuptable_2( "lookuptable_2" );

const Name make_symmetric( "make_symmetric" );
const Name max( "max" );
const Name max_buffer_size_spike_data( "max_buffer_size_spike_data" );
const Name max_buffer_size_target_data( "max_buffer_size_target_data" );
const Name max_num_syn_models( "max_num_syn_models" );
const Name max_delay( "max_delay" );
const Name mean( "mean" );
const Name memory( "memory" );
const Name message_times( "messages_times" );
const Name messages( "messages" );
const Name min( "min" );
const Name min_delay( "min_delay" );
const Name model( "model" );
const Name mother_rng( "mother_rng" );
const Name mother_seed( "mother_seed" );
const Name ms_per_tic( "ms_per_tic" );
const Name mu( "mu" );
const Name mu_minus( "mu_minus" );
const Name mu_plus( "mu_plus" );
const Name mult_coupling( "mult_coupling" );
const Name music_channel( "music_channel" );

const Name n( "n" );
const Name N( "N" );
const Name N_channels( "N_channels" );
const Name n_events( "n_events" );
const Name n_messages( "n_messages" );
const Name n_proc( "n_proc" );
const Name n_receptors( "n_receptors" );
const Name n_synapses( "n_synapses" );
const Name network_size( "network_size" );
const Name neuron( "neuron" );
const Name next_readout_time( "next_readout_time" );
const Name NMDA( "NMDA" );
const Name no_synapses( "no_synapses" );
const Name node_uses_wfr( "node_uses_wfr" );
const Name noise( "noise" );
const Name noisy_rate( "noisy_rate" );
const Name num_connections( "num_connections" );
const Name num_processes( "num_processes" );

const Name off_grid_spiking( "off_grid_spiking" );
const Name offset( "offset" );
const Name offsets( "offsets" );
const Name omega( "omega" );
const Name order( "order" );
const Name origin( "origin" );
const Name other( "other" );
const Name outdegree( "outdegree" );
const Name overwrite_files( "overwrite_files" );

const Name p( "p" );
const Name P( "P" );
const Name p_copy( "p_copy" );
const Name p_transmit( "p_transmit" );
const Name phase( "phase" );
const Name port( "port" );
const Name port_name( "port_name" );
const Name port_width( "port_width" );
const Name ports( "ports" );
const Name post_synaptic_element( "post_synaptic_element" );
const Name post_trace( "post_trace" );
const Name pre_synaptic_element( "pre_synaptic_element" );
const Name precise_times( "precise_times" );
const Name precision( "precision" );
const Name print_time( "print_time" );
const Name proximal_curr( "proximal_curr" );
const Name proximal_exc( "proximal_exc" );
const Name proximal_inh( "proximal_inh" );
const Name psi( "psi" );
const Name published( "published" );
const Name pulse_times( "pulse_times" );

const Name q_rr( "q_rr" );
const Name q_sfa( "q_sfa" );
const Name q_stc( "q_stc" );

const Name rate( "rate" );
const Name rate_times( "rate_times" );
const Name rate_values( "rate_values" );
const Name readout_cycle_duration( "readout_cycle_duration" );
const Name receptor_type( "receptor_type" );
const Name receptor_types( "receptor_types" );
const Name receptors( "receptors" );
const Name record_from( "record_from" );
const Name record_to( "record_to" );
const Name recordables( "recordables" );
const Name recorder( "recorder" );
const Name recording_backends( "recording_backends" );
const Name rectify_output( "rectify_output" );
const Name refractory_input( "refractory_input" );
const Name registered( "registered" );
const Name relative_amplitude( "relative_amplitude" );
const Name requires_symmetric( "requires_symmetric" );
const Name reset_pattern( "reset_pattern" );
const Name resolution( "resolution" );
const Name rho( "rho" );
const Name rho_0( "rho_0" );
const Name rng_seeds( "rng_seeds" );
const Name rport( "receptor" );
const Name rule( "rule" );

const Name S( "S" );
const Name S_act_NMDA( "S_act_NMDA" );
const Name scale( "scale" );
const Name sdev( "sdev" );
const Name senders( "senders" );
const Name shift_now_spikes( "shift_now_spikes" );
const Name sigma( "sigma" );
const Name sigmoid( "sigmoid" );
const Name sion_chunksize( "sion_chunksize" );
const Name sion_collective( "sion_collective" );
const Name sion_n_files( "sion_n_files" );
const Name size_of( "sizeof" );
const Name soma_curr( "soma_curr" );
const Name soma_exc( "soma_exc" );
const Name soma_inh( "soma_inh" );
const Name sort_connections_by_source( "sort_connections_by_source" );
const Name source( "source" );
const Name spike( "spike" );
const Name spike_multiplicities( "spike_multiplicities" );
const Name spike_times( "spike_times" );
const Name spike_weights( "spike_weights" );
const Name start( "start" );
const Name state( "state" );
const Name std( "std" );
const Name std_mod( "std_mod" );
const Name stimulator( "stimulator" );
const Name stop( "stop" );
const Name structural_plasticity_synapses( "structural_plasticity_synapses" );
const Name structural_plasticity_update_interval( "structural_plasticity_update_interval" );
const Name synapse_id( "synapse_id" );
const Name synapse_label( "synapse_label" );
const Name synapse_model( "synapse_model" );
const Name synapse_modelid( "synapse_modelid" );
const Name synapses_per_driver( "synapses_per_driver" );
const Name synaptic_elements( "synaptic_elements" );
const Name synaptic_elements_param( "synaptic_elements_param" );
const Name synaptic_endpoint( "synaptic_endpoint" );

const Name t_clamp( "t_clamp" );
const Name t_lag( "t_lag" );
const Name T_max( "T_max" );
const Name T_min( "T_min" );
const Name t_origin( "t_origin" );
const Name t_ref( "t_ref" );
const Name t_ref_abs( "t_ref_abs" );
const Name t_ref_remaining( "t_ref_remaining" );
const Name t_ref_tot( "t_ref_tot" );
const Name t_spike( "t_spike" );
const Name target( "target" );
const Name target_thread( "target_thread" );
const Name targets( "targets" );
const Name tau( "tau" );
const Name tau_1( "tau_1" );
const Name tau_2( "tau_2" );
const Name tau_ahp( "tau_ahp" );
const Name tau_bar_bar( "tau_bar_bar" );
const Name tau_c( "tau_c" );
const Name tau_Ca( "tau_Ca" );
const Name tau_D_KNa( "tau_D_KNa" );
const Name tau_decay( "tau_decay" );
const Name tau_decay_AMPA( "tau_decay_AMPA" );
const Name tau_decay_GABA_A( "tau_decay_GABA_A" );
const Name tau_decay_GABA_B( "tau_decay_GABA_B" );
const Name tau_decay_NMDA( "tau_decay_NMDA" );
const Name tau_epsp( "tau_epsp" );
const Name tau_eta( "tau_eta" );
const Name tau_fac( "tau_fac" );
const Name tau_m( "tau_m" );
const Name tau_max( "tau_max" );
const Name tau_Mg_fast_NMDA( "tau_Mg_fast_NMDA" );
const Name tau_Mg_slow_NMDA( "tau_Mg_slow_NMDA" );
const Name tau_minus( "tau_minus" );
const Name tau_minus_stdp( "tau_minus_stdp" );
const Name tau_minus_triplet( "tau_minus_triplet" );
const Name tau_n( "tau_n" );
const Name tau_P( "tau_P" );
const Name tau_plus( "tau_plus" );
const Name tau_plus_triplet( "tau_plus_triplet" );
const Name tau_psc( "tau_psc" );
const Name tau_rec( "tau_rec" );
const Name tau_reset( "tau_reset" );
const Name tau_decay_ex( "tau_decay_ex" );
const Name tau_rise_ex( "tau_rise_ex" );
const Name tau_decay_in( "tau_decay_in" );
const Name tau_rise_in( "tau_rise_in" );
const Name tau_rise( "tau_rise" );
const Name tau_rise_AMPA( "tau_rise_AMPA" );
const Name tau_rise_GABA_A( "tau_rise_GABA_A" );
const Name tau_rise_GABA_B( "tau_rise_GABA_B" );
const Name tau_rise_NMDA( "tau_rise_NMDA" );
const Name tau_rr( "tau_rr" );
const Name tau_sfa( "tau_sfa" );
const Name tau_spike( "tau_spike" );
const Name tau_stc( "tau_stc" );
const Name tau_syn( "tau_syn" );
const Name tau_syn_ex( "tau_syn_ex" );
const Name tau_syn_in( "tau_syn_in" );
const Name tau_theta( "tau_theta" );
const Name tau_v( "tau_v" );
const Name tau_V_th( "tau_V_th" );
const Name tau_vacant( "tau_vacant" );
const Name tau_w( "tau_w" );
const Name tau_x( "tau_x" );
const Name tau_z( "tau_z" );
const Name theta( "theta" );
const Name theta_eq( "theta_eq" );
const Name theta_ex( "theta_ex" );
const Name theta_in( "theta_in" );
const Name theta_minus( "theta_minus" );
const Name theta_plus( "theta_plus" );
const Name thread( "thread" );
const Name thread_local_id( "thread_local_id" );
const Name tics_per_ms( "tics_per_ms" );
const Name tics_per_step( "tics_per_step" );
const Name time( "time" );
const Name time_collocate( "time_collocate" );
const Name time_communicate( "time_communicate" );
const Name time_in_steps( "time_in_steps" );
const Name times( "times" );
const Name to_do( "to_do" );
const Name total_num_virtual_procs( "total_num_virtual_procs" );
const Name Tstart( "Tstart" );
const Name Tstop( "Tstop" );
const Name type_id( "type_id" );

const Name u( "u" );
const Name u_bar_bar( "u_bar_bar" );
const Name u_bar_minus( "u_bar_minus" );
const Name u_bar_plus( "u_bar_plus" );
const Name U( "U" );
const Name U_m( "U_m" );
const Name u_ref_squared( "u_ref_squared" );
const Name update( "update" );
const Name update_node( "update_node" );
const Name use_wfr( "use_wfr" );

const Name V_act_NMDA( "V_act_NMDA" );
const Name V_clamp( "V_clamp" );
const Name V_epsp( "V_epsp" );
const Name V_m( "V_m" );
const Name V_min( "V_min" );
const Name V_noise( "V_noise" );
const Name V_peak( "V_peak" );
const Name V_reset( "V_reset" );
const Name V_T( "V_T" );
const Name V_T_star( "V_T_star" );
const Name V_th( "V_th" );
const Name V_th_alpha_1( "V_th_alpha_1" );
const Name V_th_alpha_2( "V_th_alpha_2" );
const Name V_th_max( "V_th_max" );
const Name V_th_rest( "V_th_rest" );
const Name V_th_v( "V_th_v" );
const Name val_eta( "val_eta" );
const Name voltage_clamp( "voltage_clamp" );
const Name vp( "vp" );
const Name vt( "vt" );

const Name w( "w" );
const Name weight( "weight" );
const Name weight_per_lut_entry( "weight_per_lut_entry" );
const Name weight_recorder( "weight_recorder" );
const Name weighted_spikes_ex( "weighted_spikes_ex" );
const Name weighted_spikes_in( "weighted_spikes_in" );
const Name weights( "weights" );
const Name wfr_comm_interval( "wfr_comm_interval" );
const Name wfr_interpolation_order( "wfr_interpolation_order" );
const Name wfr_max_iterations( "wfr_max_iterations" );
const Name wfr_tol( "wfr_tol" );
const Name with_reset( "with_reset" );
const Name Wmax( "Wmax" );
const Name Wmin( "Wmin" );

const Name x( "x" );
const Name x_bar( "x_bar" );

const Name y1( "y1" );
const Name y2( "y2" );
const Name y( "y" );
const Name y_0( "y_0" );
const Name y_1( "y_1" );

const Name z( "z" );
const Name z_connected( "z_connected" );

} // namespace names

} // namespace nest
