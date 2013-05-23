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

    const Name V_m("V_m");
    const Name V_min("V_min");
    const Name E_L("E_L");
    const Name I_e("I_e");
    const Name I_L("I_L");
    const Name V_th("V_th");
    const Name V_reset("V_reset");
    const Name c_m("c_m");
    const Name C_m("C_m");
    const Name tau_m("tau_m");
    const Name tau_syn("tau_syn");
    const Name tau_syn_ex("tau_syn_ex");
    const Name tau_syn_in("tau_syn_in");
    const Name t_ref("t_ref");
    const Name t_ref_abs("t_ref_abs");
    const Name t_ref_tot("t_ref_tot");
    const Name t_ref_remaining("t_ref_remaining");
    const Name t_spike("t_spike");
    const Name t_origin("t_origin");
    const Name t_lag("t_lag");
    const Name E_ex("E_ex");
    const Name E_in("E_in");
    const Name I_ex("I_ex");
    const Name I_in("I_in");
    const Name g("g");
    const Name g_L("g_L");
    const Name g_ex("g_ex");
    const Name dg_ex("dg_ex");
    const Name g_in("g_in");
    const Name dg_in("dg_in");
    const Name g_Na("g_Na");
    const Name E_Na("E_Na");
    const Name g_K("g_K");
    const Name E_K("E_K");
    const Name in_spikes("in_spikes");
    const Name ex_spikes("ex_spikes");
    const Name error("error");

    // Related to ArchivingNode
    const Name tau_minus("tau_minus");  
    const Name tau_minus_triplet("tau_minus_triplet");
    const Name archiver_length("archiver_length");
    const Name clear("clear");

    // Specific to precise timing neurons
    const Name Interpol_Order("Interpol_Order");
    const Name refractory_input("refractory_input");

    // Specific to precise timing neurons (Brette 2007)
    const Name root_finding_epsilon("root_finding_epsilon");
    const Name tau_lcm("tau_lcm");
    const Name coeff_m("coeff_m");
    const Name coeff_ex("coeff_ex");
    const Name coeff_in("coeff_in");

    // For debugging
    const Name is_refractory("is_refractory"); 

    // Specific to Kobayashi, Tsubo, Shinomoto 2009
    const Name tau_1("tau_1");
    const Name tau_2("tau_2");
    const Name alpha_1("alpha_1");
    const Name alpha_2("alpha_2");
    const Name omega("omega");
    const Name V_th_alpha_1("V_th_alpha_1");
    const Name V_th_alpha_2("V_th_alpha_2");

    // Specific to Brette & Gerstner 2005
    const Name V_peak("V_peak");
    const Name a("a");
    const Name b("b");
    const Name w("w");
    const Name Delta_T("Delta_T");
    const Name tau_w("tau_w");

    // Additional parameters for Izhikevich 2003
    const Name c("c");
    const Name d("d");
    const Name U_m("U_m");
    const Name consistent_integration("consistent_integration");
    
    // Tsodyks2_connection
    const Name dU("U"); //!< Unit increment of the utilization for a facilitating synapse [0...1]
    const Name u("u");  //!< probability of release [0...1]
    const Name x("x"); //!< current scaling factor of the synaptic weight [0...1]
    const Name tau_rec("tau_rec"); //!< time constant for recovery (ms)
    const Name tau_fac("tau_fac"); //!< facilitation time constant (ms)
    
    const Name dUs("Us"); //!< Unit increment of the utilization for a facilitating synapse [0...1]
    const Name us("us");  //!< probability of release [0...1]
    const Name xs("xs"); //!< current scaling factor of the synaptic weight [0...1]
    const Name tau_recs("tau_recs"); //!< time constant for recovery (ms)
    const Name tau_facs("tau_facs"); //!< facilitation time constant (ms)

    const Name A("A");
    const Name A_upper("A_upper");
    const Name A_lower("A_lower");
    const Name A_mean("A_mean");
    const Name A_std("A_std");
    const Name U_upper("U_upper");
    const Name U_lower("U_lower");
    const Name U_mean("U_mean");
    const Name U_std("U_std");
    const Name D_upper("D_upper");
    const Name D_lower("D_lower");
    const Name D_mean("D_mean");
    const Name D_std("D_std");
    const Name F_upper("F_upper");
    const Name F_lower("F_lower");
    const Name F_mean("F_mean");
    const Name F_std("F_std");
    const Name epoch("epoch");
    const Name success("success");
    const Name with_noise("with_noise");


    // Other Adaptation
    const Name E_sfa("E_sfa");
    const Name E_rr("E_rr");
    const Name g_sfa("g_sfa");
    const Name g_rr("g_rr");
    const Name q_sfa("q_sfa");
    const Name q_rr("q_rr");
    const Name tau_sfa("tau_sfa");
    const Name tau_rr("tau_rr");

    // Specific to Hodgkin Huxley models
    const Name Act_m("Act_m");
    const Name Act_h("Act_h");
    const Name Inact_n("Inact_n");

    // Specific to current homeostasis
    const Name I_total("I_total");
    const Name I_adapt("I_adapt");
    const Name I_std("I_std");
    const Name PSC_adapt_step("PSC_adapt_step");
    const Name PSC_Unit_amplitude("PSC_Unit_amplitude");

    // Specific to sli_neuron
    const Name update("update");
    const Name update_node("update_node");
    const Name calibrate("calibrate");
    const Name calibrate_node("calibrate_node");
    const Name spike("spike");

    // Specific to mirollo_strogatz_ps
    const Name phi("phi");
    const Name I("I");
    const Name gamma("gamma");
    const Name phi_th("phi_th");

    // Specific to stochastic neuron pp_psc_delta
    const Name c_1("c_1");
    const Name c_2("c_2");
    const Name c_3("c_3");
    const Name dead_time_random("dead_time_random");
    const Name dead_time_shape("dead_time_shape");
    const Name with_reset("with_reset");

    // Specific to stochastic Ginzburg neuron ginzburg_neuron
    const Name S("S");      // Binary state (output) of neuron
    const Name h("h");      // Summed input to a neuron

    // Specific to iaf_chxk_2008 neuron
    const Name g_ahp("g_ahp");
    const Name tau_ahp("tau_ahp");
    const Name E_ahp("E_ahp");

    // Specific to iaf_chs_2007 neuron
    const Name tau_reset("tau_reset");
    const Name tau_epsp("tau_epsp");
    const Name V_epsp("V_epsp");
    const Name V_noise("V_noise");
    const Name noise("noise");

    // Specific to iaf_tum_2000
    const Name I_syn_ex("I_syn_ex");  // Total excitatory synaptic current
    const Name I_syn_in("I_syn_in");  // Total inhibitory synaptic current

    // Specific to GSL integration
    const Name gsl_error_tol("gsl_error_tol");  // GSL integrator tolerance

    // Specific to mip_generator
    const Name mother_seed("mother_seed");
    const Name mother_rng("mother_rng");
    const Name p_copy("p_copy");

    // Specific to correlation detector
    const Name delta_tau("delta_tau");
    const Name tau_max("tau_max");
    const Name histogram("histogram");
    const Name count_histogram("count_histogram");
    const Name Tstart("Tstart");
    const Name Tstop("Tstop");

    const Name origin("origin");
    const Name start("start");
    const Name stop("stop");
    const Name recordables("recordables");

    const Name individual_spike_trains("individual_spike_trains");

    const Name withtime("withtime");
    const Name withgid("withgid");
    const Name withpath("withpath");
    const Name withweight("withweight");
    const Name precise_times("precise_times");
    const Name time_in_steps("time_in_steps");
    const Name to_file("to_file");
    const Name to_screen("to_screen");
    const Name to_memory("to_memory");
    const Name to_accumulator("to_accumulator");
    const Name record_to("record_to");
    const Name file("file");
    const Name memory("memory");
    const Name screen("screen");
    const Name accumulator("accumulator");
    const Name file_extension("file_extension");
    const Name precision("precision");
    const Name scientific("scientific");
    const Name binary("binary");
    const Name fbuffer_size("fbuffer_size");
    const Name flush_records("flush_records");
    const Name close_after_simulate("close_after_simulate");
    const Name flush_after_simulate("flush_after_simulate");
    const Name close_on_reset("close_on_reset");
    const Name filename("filename");
    const Name filenames("filenames");
    const Name record_from("record_from");

    const Name senders("senders");
    const Name times("times");
    const Name offsets("offsets");
    const Name n_events("n_events");
    const Name interval("interval");
    const Name events("events");
    const Name potentials("potentials");
    const Name currents("currents");
    const Name spike_times("spike_times");
    const Name exc_conductance("exc_conductance");
    const Name inh_conductance("inh_conductance");

    const Name source("source");
    const Name target("target");
    const Name targets("targets");
    const Name weight("weight");
    const Name weights("weights");
    const Name delay("delay");
    const Name delays("delays");
    const Name receptor_type("receptor_type");
    const Name receptor_types("receptor_types");
    const Name rport("receptor");
    const Name rports("receptors");
    const Name port("port");
    const Name target_thread("target_thread");
    const Name synapse_model("synapse_model");
    const Name synapse_modelid("synapse_modelid");

    // Specific to sinusoidally modulated generators
    const Name dc("dc");
    const Name ac("ac");
    const Name freq("freq");
    const Name order("order");
 
    // Specific to ppd_sup_generator and gamma_sup_generator
    const Name amplitude("amplitude");
    const Name phase("phase");
    const Name frequency("frequency");
    const Name rate("rate");
    const Name n_proc("n_proc");
    const Name dead_time("dead_time");
    const Name gamma_shape("gamma_shape");

    // Miscellaneous parameters
    const Name label("label");
    const Name mean("mean");
    const Name std("std");
    const Name rms("rms");
    const Name dt("dt");
    const Name offset("offset");

    // Node parameters
    const Name global_id("global_id");
    const Name model("model");
    const Name frozen("frozen");
    const Name address("address");
    const Name local_id("local_id");
    const Name parent("parent");
    const Name state("state");
    const Name thread("thread");
    const Name vp("vp");
    const Name local("local");

    const Name connection_count("connection_count");
    const Name index_map("index_map");
    const Name music_channel("music_channel");
    const Name port_name("port_name");
    const Name port_width("port_width");
    const Name registered("registered");
    const Name published("published");

    const Name theta("theta");

    // Node types
    const Name type("type");
    const Name structure("structure");
    const Name neuron("neuron");
    const Name stimulator("stimulator");
    const Name recorder("recorder");
    const Name synapse("synapse");
    const Name other("other");
  }
}
