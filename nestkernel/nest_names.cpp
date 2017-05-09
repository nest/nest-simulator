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
const Name A_lower( "A_lower" );
const Name A_mean( "A_mean" );
const Name A_std( "A_std" );
const Name A_upper( "A_upper" );
const Name accumulator( "accumulator" );
const Name Act_h( "Act_h" );
const Name Act_m( "Act_m" );
const Name address( "address" );
const Name alpha_1( "alpha_1" );
const Name alpha_2( "alpha_2" );
const Name AMPA( "AMPA" );
const Name amplitude( "amplitude" );
const Name archiver_length( "archiver_length" );
const Name autapses( "autapses" );

const Name b( "b" );
const Name beta( "beta" );
const Name beta_Ca( "beta_Ca" );
const Name binary( "binary" );

const Name c( "c" );
const Name c_1( "c_1" );
const Name c_2( "c_2" );
const Name c_3( "c_3" );
const Name C_m( "C_m" );
const Name Ca( "Ca" );
const Name calibrate( "calibrate" );
const Name calibrate_node( "calibrate_node" );
const Name clear( "clear" );
const Name close_after_simulate( "close_after_simulate" );
const Name close_on_reset( "close_on_reset" );
const Name coeff_ex( "coeff_ex" );
const Name coeff_in( "coeff_in" );
const Name coeff_m( "coeff_m" );
const Name connection_count( "connection_count" );
const Name consistent_integration( "consistent_integration" );
const Name continuous( "continuous" );
const Name count_covariance( "count_covariance" );
const Name count_histogram( "count_histogram" );
const Name covariance( "covariance" );
const Name currents( "currents" );

const Name d( "d" );
const Name D_lower( "D_lower" );
const Name D_mean( "D_mean" );
const Name D_std( "D_std" );
const Name D_upper( "D_upper" );
const Name dead_time( "dead_time" );
const Name dead_time_random( "dead_time_random" );
const Name dead_time_shape( "dead_time_shape" );
const Name delay( "delay" );
const Name delays( "delays" );
const Name delta_P( "delta_P" );
const Name Delta_T( "Delta_T" );
const Name delta_tau( "delta_tau" );
const Name Delta_V( "Delta_V" );
const Name delta_u( "delta_u" );
const Name dg( "dg" );
const Name dg_ex( "dg_ex" );
const Name dg_in( "dg_in" );
const Name dI_syn_ex( "dI_syn_ex" );
const Name dI_syn_in( "dI_syn_in" );
const Name distribution( "distribution" );
const Name dt( "dt" );
const Name dU( "U" );

const Name E_ahp( "E_ahp" );
const Name E_ex( "E_ex" );
const Name E_in( "E_in" );
const Name E_K( "E_K" );
const Name E_L( "E_L" );
const Name E_Na( "E_Na" );
const Name equilibrate( "equilibrate" );
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
const Name epoch( "epoch" );
const Name eps( "eps" );
const Name error( "error" );
const Name eta( "eta" );
const Name events( "events" );
const Name ex_spikes( "ex_spikes" );

const Name F_lower( "F_lower" );
const Name F_mean( "F_mean" );
const Name F_std( "F_std" );
const Name F_upper( "F_upper" );
const Name fbuffer_size( "fbuffer_size" );
const Name file( "file" );
const Name file_extension( "file_extension" );
const Name filename( "filename" );
const Name filenames( "filenames" );
const Name filter_events( "filter_events" );
const Name filter_values( "filter_values" );
const Name filter_report_interval( "filter_report_interval" );
const Name filter_start_times( "filter_start_times" );
const Name filter_stop_times( "filter_stop_times" );
const Name flush_after_simulate( "flush_after_simulate" );
const Name flush_records( "flush_records" );
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
const Name GABA_A( "GABA_A" );
const Name GABA_B( "GABA_B" );
const Name gamma( "gamma" );
const Name gamma_shape( "gamma_shape" );
const Name gaussian( "gaussian" );
const Name global_id( "global_id" );
const Name growth_curve( "growth_curve" );
const Name growth_rate( "growth_rate" );
const Name gsl_error_tol( "gsl_error_tol" );

const Name h( "h" );
const Name has_connections( "has_connections" );
const Name has_delay( "has_delay" );
const Name histogram( "histogram" );
const Name histogram_correction( "histogram_correction" );
const Name HMIN( "HMIN" );

const Name I( "I" );
const Name I_adapt( "I_adapt" );
const Name I_e( "I_e" );
const Name I_ex( "I_ex" );
const Name I_h( "I_h" );
const Name I_in( "I_in" );
const Name I_KNa( "I_KNa" );
const Name I_L( "I_L" );
const Name I_NaP( "I_NaP" );
const Name I_stc( "I_stc" );
const Name I_std( "I_std" );
const Name I_syn( "I_syn" );
const Name I_syn_ex( "I_syn_ex" );
const Name I_syn_in( "I_syn_in" );
const Name I_T( "I_T" );
const Name I_total( "I_total" );
const Name in_spikes( "in_spikes" );
const Name Inact_n( "Inact_n" );
const Name Inact_p( "Inact_p" );
const Name indegree( "indegree" );
const Name index_map( "index_map" );
const Name individual_spike_trains( "individual_spike_trains" );
const Name inh_conductance( "inh_conductance" );
const Name instant_unblock_NMDA( "instant_unblock_NMDA" );
const Name Interpol_Order( "Interpol_Order" );
const Name interval( "interval" );
const Name is_refractory( "is_refractory" );

const Name label( "label" );
const Name lambda_0( "lambda_0" );
const Name len_kernel( "len_kernel" );
const Name linear( "linear" );
const Name local( "local" );
const Name local_id( "local_id" );

const Name make_symmetric( "make_symmetric" );
const Name max_delay( "max_delay" );
const Name MAXERR( "MAXERR" );
const Name mean( "mean" );
const Name memory( "memory" );
const Name min_delay( "min_delay" );
const Name model( "model" );
const Name mother_rng( "mother_rng" );
const Name mother_seed( "mother_seed" );
const Name multapses( "multapses" );
const Name music_channel( "music_channel" );

const Name n( "n" );
const Name N( "N" );
const Name N_channels( "N_channels" );
const Name n_events( "n_events" );
const Name n_proc( "n_proc" );
const Name n_receptors( "n_receptors" );
const Name n_synapses( "n_synapses" );
const Name neuron( "neuron" );
const Name NMDA( "NMDA" );
const Name node_uses_wfr( "node_uses_wfr" );
const Name noise( "noise" );
const Name num_connections( "num_connections" );

const Name offset( "offset" );
const Name offsets( "offsets" );
const Name omega( "omega" );
const Name order( "order" );
const Name origin( "origin" );
const Name other( "other" );
const Name outdegree( "outdegree" );

const Name p( "p" );
const Name P( "P" );
const Name p_copy( "p_copy" );
const Name parent( "parent" );
const Name phase( "phase" );
const Name phi( "phi" );
const Name phi_th( "phi_th" );
const Name port( "port" );
const Name ports( "ports" );
const Name port_name( "port_name" );
const Name port_width( "port_width" );
const Name post_synaptic_element( "post_synaptic_element" );
const Name potentials( "potentials" );
const Name pre_synaptic_element( "pre_synaptic_element" );
const Name precise_times( "precise_times" );
const Name precision( "precision" );
const Name PSC_adapt_step( "PSC_adapt_step" );
const Name PSC_Unit_amplitude( "PSC_Unit_amplitude" );
const Name psi( "psi" );
const Name published( "published" );

const Name q_rr( "q_rr" );
const Name q_sfa( "q_sfa" );
const Name q_stc( "q_stc" );

const Name rate( "rate" );
const Name receptor_type( "receptor_type" );
const Name receptor_types( "receptor_types" );
const Name record_from( "record_from" );
const Name record_spikes( "record_spikes" );
const Name record_to( "record_to" );
const Name recordables( "recordables" );
const Name recorder( "recorder" );
const Name refractory_input( "refractory_input" );
const Name registered( "registered" );
const Name relative_amplitude( "relative_amplitude" );
const Name requires_symmetric( "requires_symmetric" );
const Name rho_0( "rho_0" );
const Name rms( "rms" );
const Name root_finding_epsilon( "root_finding_epsilon" );
const Name rport( "receptor" );
const Name rports( "receptors" );
const Name rule( "rule" );

const Name S( "S" );
const Name S_act_NMDA( "S_act_NMDA" );
const Name scientific( "scientific" );
const Name screen( "screen" );
const Name senders( "senders" );
const Name sigmoid( "sigmoid" );
const Name size_of( "sizeof" );
const Name source( "source" );
const Name spike( "spike" );
const Name spike_times( "spike_times" );
const Name start( "start" );
const Name state( "state" );
const Name std( "std" );
const Name std_mod( "std_mod" );
const Name stimulator( "stimulator" );
const Name stop( "stop" );
const Name structural_plasticity_synapses( "structural_plasticity_synapses" );
const Name structural_plasticity_update_interval(
  "structural_plasticity_update_interval" );
const Name structure( "structure" );
const Name success( "success" );
const Name supports_precise_spikes( "supports_precise_spikes" );
const Name synapse( "synapse" );
const Name synapse_label( "synapse_label" );
const Name synapse_model( "synapse_model" );
const Name synapse_modelid( "synapse_modelid" );
const Name synaptic_elements( "synaptic_elements" );

const Name t_lag( "t_lag" );
const Name t_origin( "t_origin" );
const Name t_ref( "t_ref" );
const Name t_ref_abs( "t_ref_abs" );
const Name t_ref_remaining( "t_ref_remaining" );
const Name t_ref_tot( "t_ref_tot" );
const Name t_spike( "t_spike" );
const Name target( "target" );
const Name target_thread( "target_thread" );
const Name targets( "targets" );
const Name tau_1( "tau_1" );
const Name tau_2( "tau_2" );
const Name tau_ahp( "tau_ahp" );
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
const Name tau_filter( "tau_filter" );
const Name tau_lcm( "tau_lcm" );
const Name tau_m( "tau_m" );
const Name tau_max( "tau_max" );
const Name tau_Mg_fast_NMDA( "tau_Mg_fast_NMDA" );
const Name tau_Mg_slow_NMDA( "tau_Mg_slow_NMDA" );
const Name tau_minus( "tau_minus" );
const Name tau_minus_triplet( "tau_minus_triplet" );
const Name tau_P( "tau_P" );
const Name tau_rec( "tau_rec" );
const Name tau_reset( "tau_reset" );
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
const Name tau_vacant( "tau_vacant" );
const Name tau_w( "tau_w" );
const Name theta( "theta" );
const Name theta_eq( "theta_eq" );
const Name thread( "thread" );
const Name thread_local_id( "thread_local_id" );
const Name time_in_steps( "time_in_steps" );
const Name times( "times" );
const Name to_accumulator( "to_accumulator" );
const Name to_file( "to_file" );
const Name to_memory( "to_memory" );
const Name to_screen( "to_screen" );
const Name Tstart( "Tstart" );
const Name Tstop( "Tstop" );

const Name u( "u" );
const Name U_lower( "U_lower" );
const Name U_m( "U_m" );
const Name U_mean( "U_mean" );
const Name U_std( "U_std" );
const Name U_upper( "U_upper" );
const Name update( "update" );
const Name update_node( "update_node" );

const Name V_act_NMDA( "V_act_NMDA" );
const Name V_epsp( "V_epsp" );
const Name V_m( "V_m" );
const Name V_min( "V_min" );
const Name V_noise( "V_noise" );
const Name V_peak( "V_peak" );
const Name V_reset( "V_reset" );
const Name V_T_star( "V_T_star" );
const Name V_th( "V_th" );
const Name V_th_alpha_1( "V_th_alpha_1" );
const Name V_th_alpha_2( "V_th_alpha_2" );
const Name V_th_v( "V_th_v" );
const Name val_eta( "val_eta" );
const Name voltage_clamp( "voltage_clamp" );
const Name vp( "vp" );

const Name w( "w" );
const Name weight( "weight" );
const Name weight_std( "weight_std" );
const Name weighted_spikes_ex( "weighted_spikes_ex" );
const Name weighted_spikes_in( "weighted_spikes_in" );
const Name weight_recorder( "weight_recorder" );
const Name weights( "weights" );
const Name with_noise( "with_noise" );
const Name with_reset( "with_reset" );
const Name withgid( "withgid" );
const Name withpath( "withpath" );
const Name withport( "withport" );
const Name withrport( "withrport" );
const Name withtargetgid( "withtargetgid" );
const Name withtime( "withtime" );
const Name withweight( "withweight" );

const Name x( "x" );

const Name z( "z" );
const Name z_connected( "z_connected" );
}
}
