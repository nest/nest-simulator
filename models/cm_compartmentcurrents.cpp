/*
 *  cm_compartmentcurrents.cpp
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
#include "cm_compartmentcurrents.h"


nest::Na::Na()
  // state variables
  : m_Na_( 0.0 )
  , h_Na_( 0.0 )
  // parameters
  , gbar_Na_( 0.0 )
  , e_Na_( 50. )
{
  // some default initialization
  init_statevars( -75. );
}
nest::Na::Na( const DictionaryDatum& channel_params )
  // state variables
  : m_Na_( 0.0 )
  , h_Na_( 0.0 )
  // parameters
  , gbar_Na_( 0.0 )
  , e_Na_( 50. )
{
  // update sodium channel parameters
  if ( channel_params->known( "gbar_Na" ) )
  {
    gbar_Na_ = getValue< double >( channel_params, "gbar_Na" );
  }
  if ( channel_params->known( "e_Na" ) )
  {
    e_Na_ = getValue< double >( channel_params, "e_Na" );
  }

  // set the initialization of the channel
  double v_init = -75.;
  if ( channel_params->known( "v_comp" ) )
  {
    v_init = getValue< double >( channel_params, "v_comp" );
  }
  init_statevars( v_init );

}

void
nest::Na::init_statevars( double v_init )
{
  double tau_dummy; // not required for initialization
  compute_statevar_m( v_init, tau_dummy, m_Na_ );
  compute_statevar_h( v_init, tau_dummy, h_Na_ );
}

void
nest::Na::pre_run_hook()
{
}

void
nest::Na::append_recordables( std::map< Name, double* >* recordables, const long compartment_idx )
{
  ( *recordables )[ Name( "m_Na_" + std::to_string( compartment_idx ) ) ] = &m_Na_;
  ( *recordables )[ Name( "h_Na_" + std::to_string( compartment_idx ) ) ] = &h_Na_;
}

void
nest::Na::compute_statevar_m( const double v_comp, double& tau_m_Na, double& m_inf_Na )
{
  /**
   * Channel rate equations from the following .mod file:
   * https://senselab.med.yale.edu/ModelDB/ShowModel?model=140828&file=/Branco_2010/mod.files/na.mod#tabs-2
   */
  // auxiliary variables
  double v_comp_plus_35 = v_comp + 35.013;

  // trap the case where alpha_m and beta_m are 0/0 by substituting explicitly
  // precomputed limiting values
  double alpha_m, frac_alpha_plus_beta_m;
  if ( std::abs( v_comp_plus_35 ) > 1e-5 )
  {
    double exp_vcp35_div_9 = std::exp( 0.111111111111111 * v_comp_plus_35 );
    double frac_evcp35d9 = 1. / ( exp_vcp35_div_9 - 1. );

    alpha_m = 0.182 * v_comp_plus_35 * exp_vcp35_div_9 * frac_evcp35d9;
    double beta_m = 0.124 * v_comp_plus_35 * frac_evcp35d9;
    frac_alpha_plus_beta_m = 1. / ( alpha_m + beta_m );
  }
  else
  {
    alpha_m = 1.638;
    frac_alpha_plus_beta_m = 1. / ( alpha_m + 1.116 );
  }

  // activation and timescale for state variable 'm'
  tau_m_Na = q10_ * frac_alpha_plus_beta_m;
  m_inf_Na = alpha_m * frac_alpha_plus_beta_m;
}

void
nest::Na::compute_statevar_h( const double v_comp, double& tau_h_Na, double& h_inf_Na )
{
  /**
   * Channel rate equations from the following .mod file:
   * https://senselab.med.yale.edu/ModelDB/ShowModel?model=140828&file=/Branco_2010/mod.files/na.mod#tabs-2
   */
  double v_comp_plus_50 = v_comp + 50.013;
  double v_comp_plus_75 = v_comp + 75.013;

  // trap the case where alpha_h or beta_h are 0/0 by substituting
  // precomputed limiting values
  double alpha_h, beta_h;
  if ( std::abs( v_comp_plus_50 ) > 1e-5 )
  {
    alpha_h = 0.024 * v_comp_plus_50 / ( 1.0 - std::exp( -0.2 * v_comp_plus_50 ) );
  }
  else
  {
    alpha_h = 0.12;
  }
  if ( std::abs( v_comp_plus_75 ) > 1e-9 )
  {
    beta_h = -0.0091 * v_comp_plus_75 / ( 1.0 - std::exp( 0.2 * v_comp_plus_75 ) );
  }
  else
  {
    beta_h = 0.0455;
  }

  // activation and timescale for state variable 'h'
  tau_h_Na = q10_ / ( alpha_h + beta_h );
  h_inf_Na = 1. / ( 1. + std::exp( ( v_comp + 65. ) / 6.2 ) );
}

std::pair< double, double >
nest::Na::f_numstep( const double v_comp )
{
  const double dt = Time::get_resolution().get_ms();
  double g_val = 0., i_val = 0.;

  if ( gbar_Na_ > 1e-9 )
  {
    double tau_m_Na, m_inf_Na;
    compute_statevar_m( v_comp, tau_m_Na, m_inf_Na );

    double tau_h_Na, h_inf_Na;
    compute_statevar_h( v_comp, tau_h_Na, h_inf_Na );

    // advance state variable 'm' one timestep
    double p_m_Na = std::exp( -dt / tau_m_Na );
    m_Na_ *= p_m_Na;
    m_Na_ += ( 1. - p_m_Na ) * m_inf_Na;

    // advance state variable 'h' one timestep
    double p_h_Na = std::exp( -dt / tau_h_Na );
    h_Na_ *= p_h_Na;
    h_Na_ += ( 1. - p_h_Na ) * h_inf_Na;

    // compute the conductance of the sodium channel
    double g_Na = gbar_Na_ * std::pow( m_Na_, 3 ) * h_Na_;

    // add to variables for numerical integration
    g_val += g_Na / 2.;
    i_val += g_Na * ( e_Na_ - v_comp / 2. );
  }

  return std::make_pair( g_val, i_val );
}


nest::K::K()
  // state variables
  : n_K_( 0.0 )
  // parameters
  , gbar_K_( 0.0 )
  , e_K_( -85. )
{
}
nest::K::K( const DictionaryDatum& channel_params )
  // state variables
  : n_K_( 0.0 )
  // parameters
  , gbar_K_( 0.0 )
  , e_K_( -85. )
{
  // update potassium channel parameters
  if ( channel_params->known( "gbar_K" ) )
  {
    gbar_K_ = getValue< double >( channel_params, "gbar_K" );
  }
  if ( channel_params->known( "e_Na" ) )
  {
    e_K_ = getValue< double >( channel_params, "e_K" );
  }

  // initialize the state variables
  double v_init = -75.;
  if ( channel_params->known( "v_comp" ) )
  {
    v_init = getValue< double >( channel_params, "v_comp" );
  }
  init_statevars( v_init );
}

void
nest::K::init_statevars( double v_init )
{
  double tau_dummy;
  compute_statevar_n( v_init, tau_dummy, n_K_ );
}

void
nest::K::pre_run_hook()
{
}

void
nest::K::append_recordables( std::map< Name, double* >* recordables, const long compartment_idx )
{
  ( *recordables )[ Name( "n_K_" + std::to_string( compartment_idx ) ) ] = &n_K_;
}

void
nest::K::compute_statevar_n( const double v_comp, double& tau_n_K, double& n_inf_K )
{
  /**
   * Channel rate equations from the following .mod file:
   * https://senselab.med.yale.edu/ModelDB/ShowModel?model=140828&file=/Branco_2010/mod.files/kv.mod#tabs-2
   */
  // auxiliary variables
  double v_comp_minus_25 = v_comp - 25.;

  // trap the case where alpha_n and beta_n are 0/0 by substituting explicitly
  // precomputed limiting values
  double alpha_n, frac_alpha_plus_beta_n;
  if ( std::abs( v_comp_minus_25 ) > 1e-5 )
  {
    double exp_vm25_div_9 = std::exp( 0.111111111111111 * v_comp_minus_25 );
    double frac_evm25d9 = 1. / ( exp_vm25_div_9 - 1. );

    alpha_n = 0.02 * v_comp_minus_25 * exp_vm25_div_9 * frac_evm25d9;
    double beta_n = 0.002 * v_comp_minus_25 * frac_evm25d9;
    frac_alpha_plus_beta_n = 1. / ( alpha_n + beta_n );
  }
  else
  {
    alpha_n = 0.18;
    double beta_n = 0.018;
    frac_alpha_plus_beta_n = 1. / ( alpha_n + beta_n );
  }

  // activation and timescale of state variable 'n'
  tau_n_K = q10_ * frac_alpha_plus_beta_n;
  n_inf_K = alpha_n * frac_alpha_plus_beta_n;
}

std::pair< double, double >
nest::K::f_numstep( const double v_comp )
{
  const double dt = Time::get_resolution().get_ms();
  double g_val = 0., i_val = 0.;

  if ( gbar_K_ > 1e-9 )
  {
    double tau_n_K, n_inf_K;
    compute_statevar_n( v_comp, tau_n_K, n_inf_K );

    // advance state variable 'm' one timestep
    double p_n_K = std::exp( -dt / tau_n_K );
    n_K_ *= p_n_K;
    n_K_ += ( 1. - p_n_K ) * n_inf_K;

    // compute the conductance of the potassium channel
    double g_K = gbar_K_ * n_K_;

    // add to variables for numerical integration
    g_val += g_K / 2.;
    i_val += g_K * ( e_K_ - v_comp / 2. );
  }

  return std::make_pair( g_val, i_val );
}


nest::AMPA::AMPA( const long syn_index )
  // initialization state variables
  : g_r_AMPA_( 0.0 )
  , g_d_AMPA_( 0.0 )
  // initialization parameters
  , e_rev_( 0.0 )
  , tau_r_( 0.2 )
  , tau_d_( 3.0 )
{
  syn_idx = syn_index;

  double tp = ( tau_r_ * tau_d_ ) / ( tau_d_ - tau_r_ ) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );
}
nest::AMPA::AMPA( const long syn_index, const DictionaryDatum& receptor_params )
  // initialization state variables
  : g_r_AMPA_( 0.0 )
  , g_d_AMPA_( 0.0 )
  // initialization parameters
  , e_rev_( 0.0 )
  , tau_r_( 0.2 )
  , tau_d_( 3.0 )
{
  syn_idx = syn_index;

  // update AMPA receptor parameters
  if ( receptor_params->known( "e_AMPA" ) )
  {
    e_rev_ = getValue< double >( receptor_params, "e_AMPA" );
  }
  if ( receptor_params->known( "tau_r_AMPA" ) )
  {
    tau_r_ = getValue< double >( receptor_params, "tau_r_AMPA" );
  }
  if ( receptor_params->known( "tau_d_AMPA" ) )
  {
    tau_d_ = getValue< double >( receptor_params, "tau_d_AMPA" );
  }

  double tp = ( tau_r_ * tau_d_ ) / ( tau_d_ - tau_r_ ) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );
}

void
nest::AMPA::append_recordables( std::map< Name, double* >* recordables )
{
  ( *recordables )[ Name( "g_r_AMPA_" + std::to_string( syn_idx ) ) ] = &g_r_AMPA_;
  ( *recordables )[ Name( "g_d_AMPA_" + std::to_string( syn_idx ) ) ] = &g_d_AMPA_;
}

std::pair< double, double >
nest::AMPA::f_numstep( const double v_comp, const long lag )
{
  // update conductance
  g_r_AMPA_ *= prop_r_;
  g_d_AMPA_ *= prop_d_;

  // add spikes
  double s_val = b_spikes_->get_value( lag ) * g_norm_;
  g_r_AMPA_ -= s_val;
  g_d_AMPA_ += s_val;

  // compute synaptic conductance
  double g_AMPA = g_r_AMPA_ + g_d_AMPA_;

  // total current
  double i_tot = g_AMPA * ( e_rev_ - v_comp );
  // voltage derivative of total current
  double d_i_tot_dv = -g_AMPA;

  // for numerical integration
  double g_val = -d_i_tot_dv / 2.;
  double i_val = i_tot + g_val * v_comp;

  return std::make_pair( g_val, i_val );
}


nest::GABA::GABA( const long syn_index )
  // initialization state variables
  : g_r_GABA_( 0.0 )
  , g_d_GABA_( 0.0 )
  // initialization parameters
  , e_rev_( -80. )
  , tau_r_( 0.2 )
  , tau_d_( 10.0 )
{
  syn_idx = syn_index;

  double tp = ( tau_r_ * tau_d_ ) / ( tau_d_ - tau_r_ ) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );
}
nest::GABA::GABA( const long syn_index, const DictionaryDatum& receptor_params )
  // initialization state variables
  : g_r_GABA_( 0.0 )
  , g_d_GABA_( 0.0 )
  // initialization parameters
  , e_rev_( -80. )
  , tau_r_( 0.2 )
  , tau_d_( 10.0 )
{
  syn_idx = syn_index;

  // update GABA receptor parameters
  if ( receptor_params->known( "e_GABA" ) )
  {
    e_rev_ = getValue< double >( receptor_params, "e_GABA" );
  }
  if ( receptor_params->known( "tau_r_GABA" ) )
  {
    tau_r_ = getValue< double >( receptor_params, "tau_r_GABA" );
  }
  if ( receptor_params->known( "tau_d_GABA" ) )
  {
    tau_d_ = getValue< double >( receptor_params, "tau_d_GABA" );
  }

  double tp = ( tau_r_ * tau_d_ ) / ( tau_d_ - tau_r_ ) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );
}

void
nest::GABA::append_recordables( std::map< Name, double* >* recordables )
{
  ( *recordables )[ Name( "g_r_GABA_" + std::to_string( syn_idx ) ) ] = &g_r_GABA_;
  ( *recordables )[ Name( "g_d_GABA_" + std::to_string( syn_idx ) ) ] = &g_d_GABA_;
}

std::pair< double, double >
nest::GABA::f_numstep( const double v_comp, const long lag )
{
  // update conductance
  g_r_GABA_ *= prop_r_;
  g_d_GABA_ *= prop_d_;

  // add spikes
  double s_val = b_spikes_->get_value( lag ) * g_norm_;
  g_r_GABA_ -= s_val;
  g_d_GABA_ += s_val;

  // compute synaptic conductance
  double g_GABA = g_r_GABA_ + g_d_GABA_;

  // total current
  double i_tot = g_GABA * ( e_rev_ - v_comp );
  // voltage derivative of total current
  double d_i_tot_dv = -g_GABA;

  // for numerical integration
  double g_val = -d_i_tot_dv / 2.;
  double i_val = i_tot + g_val * v_comp;

  return std::make_pair( g_val, i_val );
}


nest::NMDA::NMDA( const long syn_index )
  // initialization state variables
  : g_r_NMDA_( 0.0 )
  , g_d_NMDA_( 0.0 )
  // initialization parameters
  , e_rev_( 0. )
  , tau_r_( 0.2 )
  , tau_d_( 43.0 )
{
  syn_idx = syn_index;

  double tp = ( tau_r_ * tau_d_ ) / ( tau_d_ - tau_r_ ) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );
}
nest::NMDA::NMDA( const long syn_index, const DictionaryDatum& receptor_params )
  // initialization state variables
  : g_r_NMDA_( 0.0 )
  , g_d_NMDA_( 0.0 )
  // initialization parameters
  , e_rev_( 0. )
  , tau_r_( 0.2 )
  , tau_d_( 43.0 )
{
  syn_idx = syn_index;

  // update NMDA receptor parameters
  if ( receptor_params->known( "e_NMDA" ) )
  {
    e_rev_ = getValue< double >( receptor_params, "e_NMDA" );
  }
  if ( receptor_params->known( "tau_r_NMDA" ) )
  {
    tau_r_ = getValue< double >( receptor_params, "tau_r_NMDA" );
  }
  if ( receptor_params->known( "tau_d_NMDA" ) )
  {
    tau_d_ = getValue< double >( receptor_params, "tau_d_NMDA" );
  }

  double tp = ( tau_r_ * tau_d_ ) / ( tau_d_ - tau_r_ ) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );
}

void
nest::NMDA::append_recordables( std::map< Name, double* >* recordables )
{
  ( *recordables )[ Name( "g_r_NMDA_" + std::to_string( syn_idx ) ) ] = &g_r_NMDA_;
  ( *recordables )[ Name( "g_d_NMDA_" + std::to_string( syn_idx ) ) ] = &g_d_NMDA_;
}

std::pair< double, double >
nest::NMDA::f_numstep( const double v_comp, const long lag )
{
  // update conductance
  g_r_NMDA_ *= prop_r_;
  g_d_NMDA_ *= prop_d_;

  // add spikes
  double s_val = b_spikes_->get_value( lag ) * g_norm_;
  g_r_NMDA_ -= s_val;
  g_d_NMDA_ += s_val;

  // compute conductance window
  double g_NMDA = g_r_NMDA_ + g_d_NMDA_;

  // auxiliary variables
  std::pair< double, double > NMDA_sigmoid = NMDA_sigmoid__and__d_NMDAsigmoid_dv( v_comp );

  // total current
  double i_tot = g_NMDA * NMDA_sigmoid.first * ( e_rev_ - v_comp );
  // voltage derivative of total current
  double d_i_tot_dv = g_NMDA * ( NMDA_sigmoid.second * ( e_rev_ - v_comp ) - NMDA_sigmoid.first );

  // for numerical integration
  double g_val = -d_i_tot_dv / 2.;
  double i_val = i_tot + g_val * v_comp;

  return std::make_pair( g_val, i_val );
}


nest::AMPA_NMDA::AMPA_NMDA( const long syn_index )
  // initialization state variables
  : g_r_AN_AMPA_( 0.0 )
  , g_d_AN_AMPA_( 0.0 )
  , g_r_AN_NMDA_( 0.0 )
  , g_d_AN_NMDA_( 0.0 )
  // initialization parameters
  , e_rev_( 0. )
  , tau_r_AMPA_( 0.2 )
  , tau_d_AMPA_( 3.0 )
  , tau_r_NMDA_( 0.2 )
  , tau_d_NMDA_( 43.0 )
  , NMDA_ratio_( 2.0 )
{
  syn_idx = syn_index;

  // AMPA normalization constant
  double tp = ( tau_r_AMPA_ * tau_d_AMPA_ ) / ( tau_d_AMPA_ - tau_r_AMPA_ ) * std::log( tau_d_AMPA_ / tau_r_AMPA_ );
  g_norm_AMPA_ = 1. / ( -std::exp( -tp / tau_r_AMPA_ ) + std::exp( -tp / tau_d_AMPA_ ) );
  // NMDA normalization constant
  tp = ( tau_r_NMDA_ * tau_d_NMDA_ ) / ( tau_d_NMDA_ - tau_r_NMDA_ ) * std::log( tau_d_NMDA_ / tau_r_NMDA_ );
  g_norm_NMDA_ = 1. / ( -std::exp( -tp / tau_r_NMDA_ ) + std::exp( -tp / tau_d_NMDA_ ) );
}
nest::AMPA_NMDA::AMPA_NMDA( const long syn_index, const DictionaryDatum& receptor_params )
  // initialization state variables
  : g_r_AN_AMPA_( 0.0 )
  , g_d_AN_AMPA_( 0.0 )
  , g_r_AN_NMDA_( 0.0 )
  , g_d_AN_NMDA_( 0.0 )
  // initialization parameters
  , e_rev_( 0. )
  , tau_r_AMPA_( 0.2 )
  , tau_d_AMPA_( 3.0 )
  , tau_r_NMDA_( 0.2 )
  , tau_d_NMDA_( 43.0 )
  , NMDA_ratio_( 2.0 )
{
  syn_idx = syn_index;

  // update AMPA+NMDA receptor parameters
  if ( receptor_params->known( "e_AMPA_NMDA" ) )
  {
    e_rev_ = getValue< double >( receptor_params, "e_AMPA_NMDA" );
  }
  if ( receptor_params->known( "tau_r_AMPA" ) )
  {
    tau_r_AMPA_ = getValue< double >( receptor_params, "tau_r_AMPA" );
  }
  if ( receptor_params->known( "tau_d_AMPA" ) )
  {
    tau_d_AMPA_ = getValue< double >( receptor_params, "tau_d_AMPA" );
  }
  if ( receptor_params->known( "tau_r_NMDA" ) )
  {
    tau_r_NMDA_ = getValue< double >( receptor_params, "tau_r_NMDA" );
  }
  if ( receptor_params->known( "tau_d_NMDA" ) )
  {
    tau_d_NMDA_ = getValue< double >( receptor_params, "tau_d_NMDA" );
  }
  if ( receptor_params->known( "NMDA_ratio" ) )
  {
    NMDA_ratio_ = getValue< double >( receptor_params, "NMDA_ratio" );
  }

  // AMPA normalization constant
  double tp = ( tau_r_AMPA_ * tau_d_AMPA_ ) / ( tau_d_AMPA_ - tau_r_AMPA_ ) * std::log( tau_d_AMPA_ / tau_r_AMPA_ );
  g_norm_AMPA_ = 1. / ( -std::exp( -tp / tau_r_AMPA_ ) + std::exp( -tp / tau_d_AMPA_ ) );
  // NMDA normalization constant
  tp = ( tau_r_NMDA_ * tau_d_NMDA_ ) / ( tau_d_NMDA_ - tau_r_NMDA_ ) * std::log( tau_d_NMDA_ / tau_r_NMDA_ );
  g_norm_NMDA_ = 1. / ( -std::exp( -tp / tau_r_NMDA_ ) + std::exp( -tp / tau_d_NMDA_ ) );
}

void
nest::AMPA_NMDA::append_recordables( std::map< Name, double* >* recordables )
{
  ( *recordables )[ Name( "g_r_AN_AMPA_" + std::to_string( syn_idx ) ) ] = &g_r_AN_AMPA_;
  ( *recordables )[ Name( "g_d_AN_AMPA_" + std::to_string( syn_idx ) ) ] = &g_d_AN_AMPA_;
  ( *recordables )[ Name( "g_r_AN_NMDA_" + std::to_string( syn_idx ) ) ] = &g_r_AN_NMDA_;
  ( *recordables )[ Name( "g_d_AN_NMDA_" + std::to_string( syn_idx ) ) ] = &g_d_AN_NMDA_;
}

std::pair< double, double >
nest::AMPA_NMDA::f_numstep( const double v_comp, const long lag )
{
  // update conductance
  g_r_AN_AMPA_ *= prop_r_AMPA_;
  g_d_AN_AMPA_ *= prop_d_AMPA_;
  g_r_AN_NMDA_ *= prop_r_NMDA_;
  g_d_AN_NMDA_ *= prop_d_NMDA_;

  // add spikes
  double s_val_ = b_spikes_->get_value( lag );
  double s_val = s_val_ * g_norm_AMPA_;
  g_r_AN_AMPA_ -= s_val;
  g_d_AN_AMPA_ += s_val;
  s_val = s_val_ * g_norm_NMDA_;
  g_r_AN_NMDA_ -= s_val;
  g_d_AN_NMDA_ += s_val;

  // compute conductance window
  double g_AMPA = g_r_AN_AMPA_ + g_d_AN_AMPA_;
  double g_NMDA = g_r_AN_NMDA_ + g_d_AN_NMDA_;

  // auxiliary variable
  std::pair< double, double > NMDA_sigmoid = NMDA_sigmoid__and__d_NMDAsigmoid_dv( v_comp );

  // total current
  double i_tot = ( g_AMPA + NMDA_ratio_ * g_NMDA * NMDA_sigmoid.first ) * ( e_rev_ - v_comp );
  // voltage derivative of total current
  double d_i_tot_dv =
    -g_AMPA + NMDA_ratio_ * g_NMDA * ( NMDA_sigmoid.second * ( e_rev_ - v_comp ) - NMDA_sigmoid.first );

  // for numerical integration
  double g_val = -d_i_tot_dv / 2.;
  double i_val = i_tot + g_val * v_comp;

  return std::make_pair( g_val, i_val );
}
