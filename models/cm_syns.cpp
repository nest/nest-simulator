#include "cm_syns.h"


// AMPA synapse ////////////////////////////////////////////////////////////////
nest::AMPA::AMPA( std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params )
  : e_rev_(0.0)
  , tau_r_(0.2)
  , tau_d_(3.0)
{
  // update sodium channel parameters
  if( receptor_params->known( "e_AMPA" ) )
      e_rev_ = getValue< double >( receptor_params, "e_AMPA" );
  if( receptor_params->known( "tau_r_AMPA" ) )
      tau_r_ = getValue< double >( receptor_params, "tau_r_AMPA" );
  if( receptor_params->known( "tau_d_AMPA" ) )
      tau_d_ = getValue< double >( receptor_params, "tau_d_AMPA" );

  double tp = (tau_r_ * tau_d_) / (tau_d_ - tau_r_) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );

  // store pointer to ringbuffer
  b_spikes_ = b_spikes;
}

std::pair< double, double > nest::AMPA::f_numstep( const double v_comp, const double dt, const long lag )
{
  // construct propagators
  double prop_r = std::exp( -dt / tau_r_ );
  double prop_d = std::exp( -dt / tau_d_ );

  // update conductance
  g_r_ *= prop_r; g_d_ *= prop_d;

  // add spikes
  double s_val = b_spikes_->get_value( lag ) * g_norm_;
  g_r_ -= s_val;
  g_d_ += s_val;

  // compute synaptic conductance
  double g_AMPA = g_r_ + g_d_;

  // total current
  double i_tot = g_AMPA * ( e_rev_ - v_comp );
  // voltage derivative of total current
  double d_i_tot_dv = - g_AMPA;

  // for numberical integration
  double g_val = - d_i_tot_dv / 2.;
  double i_val = i_tot - d_i_tot_dv * v_comp / 2.;

  return std::make_pair(g_val, i_val);
}
////////////////////////////////////////////////////////////////////////////////


// GABA synapse ////////////////////////////////////////////////////////////////
nest::GABA::GABA( std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params )
  : e_rev_(-80.)
  , tau_r_(0.2)
  , tau_d_(10.0)
{
  // update sodium channel parameters
  if( receptor_params->known( "e_GABA" ) )
      e_rev_ = getValue< double >( receptor_params, "e_GABA" );
  if( receptor_params->known( "tau_r_GABA" ) )
      tau_r_ = getValue< double >( receptor_params, "tau_r_GABA" );
  if( receptor_params->known( "tau_d_GABA" ) )
      tau_d_ = getValue< double >( receptor_params, "tau_d_GABA" );

  double tp = (tau_r_ * tau_d_) / (tau_d_ - tau_r_) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );

  // store pointer to ringbuffer
  b_spikes_ = b_spikes;
}

std::pair< double, double > nest::GABA::f_numstep( const double v_comp, const double dt, const long lag )
{
  // construct propagators
  double prop_r = std::exp( -dt / tau_r_ );
  double prop_d = std::exp( -dt / tau_d_ );

  // update conductance
  g_r_ *= prop_r; g_d_ *= prop_d;

  // add spikes
  double s_val = b_spikes_->get_value( lag ) * g_norm_;
  g_r_ -= s_val;
  g_d_ += s_val;

  // compute synaptic conductance
  double g_GABA = g_r_ + g_d_;

  // total current
  double i_tot = g_GABA * ( e_rev_ - v_comp );
  // voltage derivative of total current
  double d_i_tot_dv = - g_GABA;

  // for numberical integration
  double g_val = - d_i_tot_dv / 2.;
  double i_val = i_tot - d_i_tot_dv * v_comp / 2.;

  return std::make_pair(g_val, i_val);
}
////////////////////////////////////////////////////////////////////////////////


// NMDA synapse ////////////////////////////////////////////////////////////////
nest::NMDA::NMDA( std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params )
  : e_rev_(0.)
  , tau_r_(0.2)
  , tau_d_(43.0)
{
  // update sodium channel parameters
  if( receptor_params->known( "e_NMDA" ) )
      e_rev_ = getValue< double >( receptor_params, "e_NMDA" );
  if( receptor_params->known( "tau_r_NMDA" ) )
      tau_r_ = getValue< double >( receptor_params, "tau_r_NMDA" );
  if( receptor_params->known( "tau_d_NMDA" ) )
      tau_d_ = getValue< double >( receptor_params, "tau_d_NMDA" );

  double tp = (tau_r_ * tau_d_) / (tau_d_ - tau_r_) * std::log( tau_d_ / tau_r_ );
  g_norm_ = 1. / ( -std::exp( -tp / tau_r_ ) + std::exp( -tp / tau_d_ ) );

  // store pointer to ringbuffer
  b_spikes_ = b_spikes;
}

std::pair< double, double > nest::NMDA::f_numstep( const double v_comp, const double dt, const long lag )
{
  double prop_r = std::exp( -dt / tau_r_ );
  double prop_d = std::exp( -dt / tau_d_ );

  // update conductance
  g_r_ *= prop_r; g_d_ *= prop_d;

  // add spikes
  double s_val = b_spikes_->get_value( lag ) * g_norm_;
  g_r_ -= s_val;
  g_d_ += s_val;

  // compute conductance window
  double g_NMDA = g_r_ + g_d_;

  // total current
  double i_tot = g_NMDA * NMDAsigmoid( v_comp ) * (e_rev_ - v_comp);
  // voltage derivative of total current
  double d_i_tot_dv = g_NMDA * ( d_NMDAsigmoid_dv( v_comp ) * (e_rev_ - v_comp) -
                                 NMDAsigmoid( v_comp ));

  // for numberical integration
  double g_val = - d_i_tot_dv / 2.;
  double i_val = i_tot - d_i_tot_dv * v_comp / 2.;

  return std::make_pair( g_val, i_val );
}
////////////////////////////////////////////////////////////////////////////////


// AMPA_NMDA synapse ///////////////////////////////////////////////////////////
nest::AMPA_NMDA::AMPA_NMDA( std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params )
  : e_rev_(0.)
  , tau_r_AMPA_(0.2)
  , tau_d_AMPA_(3.0)
  , tau_r_NMDA_(0.2)
  , tau_d_NMDA_(43.0)
  , NMDA_ratio_(2.0)
{
  // update sodium channel parameters
  if( receptor_params->known( "e_AMPA_NMDA" ) )
      e_rev_ = getValue< double >( receptor_params, "e_AMPA_NMDA" );
  if( receptor_params->known( "tau_r_AMPA" ) )
      tau_r_AMPA_ = getValue< double >( receptor_params, "tau_r_AMPA" );
  if( receptor_params->known( "tau_d_AMPA" ) )
      tau_d_AMPA_ = getValue< double >( receptor_params, "tau_d_AMPA" );
  if( receptor_params->known( "tau_r_NMDA" ) )
      tau_r_NMDA_ = getValue< double >( receptor_params, "tau_r_NMDA" );
  if( receptor_params->known( "tau_d_NMDA" ) )
      tau_d_NMDA_ = getValue< double >( receptor_params, "tau_d_NMDA" );
  if( receptor_params->known( "NMDA_ratio" ) )
      NMDA_ratio_ = getValue< double >( receptor_params, "NMDA_ratio" );

  // AMPA normalization constant
  double tp = (tau_r_AMPA_ * tau_d_AMPA_) / (tau_d_AMPA_ - tau_r_AMPA_) * std::log( tau_d_AMPA_ / tau_r_AMPA_ );
  g_norm_AMPA_ = 1. / ( -std::exp( -tp / tau_r_AMPA_ ) + std::exp( -tp / tau_d_AMPA_ ) );
  // NMDA normalization constant
  tp = (tau_r_NMDA_ * tau_d_NMDA_) / (tau_d_NMDA_ - tau_r_NMDA_) * std::log( tau_d_NMDA_ / tau_r_NMDA_ );
  g_norm_NMDA_ = 1. / ( -std::exp( -tp / tau_r_NMDA_ ) + std::exp( -tp / tau_d_NMDA_ ) );

  // store pointer to ringbuffer
  b_spikes_ = b_spikes;
}

std::pair< double, double > nest::AMPA_NMDA::f_numstep( const double v_comp, const double dt, const long lag )
{
  double prop_r_AMPA = std::exp( -dt / tau_r_AMPA_ );
  double prop_d_AMPA = std::exp( -dt / tau_d_AMPA_ );
  double prop_r_NMDA = std::exp( -dt / tau_r_NMDA_ );
  double prop_d_NMDA = std::exp( -dt / tau_d_NMDA_ );

  // update conductance
  g_r_AMPA_ *= prop_r_AMPA; g_d_AMPA_ *= prop_d_AMPA;
  g_r_NMDA_ *= prop_r_NMDA; g_d_NMDA_ *= prop_d_NMDA;

  // add spikes
  double s_val_ = b_spikes_->get_value( lag );
  double s_val = s_val_ * g_norm_AMPA_;
  g_r_AMPA_ -= s_val;
  g_d_AMPA_ += s_val;
  s_val = s_val_ * g_norm_NMDA_;
  g_r_NMDA_ -= s_val;
  g_d_NMDA_ += s_val;

  // compute conductance window
  double g_AMPA = g_r_AMPA_ + g_d_AMPA_;
  double g_NMDA = g_r_NMDA_ + g_d_NMDA_;

  // total current
  double i_tot = ( g_AMPA + NMDA_ratio_ * g_NMDA * NMDAsigmoid( v_comp ) ) * (e_rev_ - v_comp);
  // voltage derivative of total current
  double d_i_tot_dv = - g_AMPA + NMDA_ratio_ *
                        g_NMDA * ( d_NMDAsigmoid_dv( v_comp ) * (e_rev_ - v_comp) -
                                   NMDAsigmoid( v_comp ));

  // for numberical integration
  double g_val = - d_i_tot_dv / 2.;
  double i_val = i_tot - d_i_tot_dv * v_comp / 2.;

  return std::make_pair( g_val, i_val );
}
////////////////////////////////////////////////////////////////////////////////


