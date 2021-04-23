#include "cm_compartmentcurrents.h"


// Na channel //////////////////////////////////////////////////////////////////
nest::Na::Na()
    : m_Na_(0.0)
    , h_Na_(0.0)
    , gbar_Na_(0.0)
    , e_Na_(50.)
{
}
nest::Na::Na(const DictionaryDatum& channel_params)
    : m_Na_(0.0)
    , h_Na_(0.0)
    , gbar_Na_(0.0)
    , e_Na_(50.)
{
    // update sodium channel parameters
    if( channel_params->known( "g_Na" ) )
        gbar_Na_ = getValue< double >( channel_params, "g_Na" );
    if( channel_params->known( "e_Na" ) )
        e_Na_ = getValue< double >( channel_params, "e_Na" );
}

std::pair< double, double > nest::Na::f_numstep( const double v_comp, const double dt)
{
  double g_val = 0., i_val = 0.;

  if (gbar_Na_ > 1e-9)
  {
    // activation and timescale of state variable 'm'
    double m_inf_Na = (0.182*v_comp + 6.3723659999999995)/((1.0 - 0.020438532058318047*exp(-0.1111111111111111*v_comp))*((-0.124*v_comp - 4.3416119999999996)/(1.0 - 48.927192870146527*exp(0.1111111111111111*v_comp)) + (0.182*v_comp + 6.3723659999999995)/(1.0 - 0.020438532058318047*exp(-0.1111111111111111*v_comp))));
    double tau_m_Na = 0.3115264797507788/((-0.124*v_comp - 4.3416119999999996)/(1.0 - 48.927192870146527*exp(0.1111111111111111*v_comp)) + (0.182*v_comp + 6.3723659999999995)/(1.0 - 0.020438532058318047*exp(-0.1111111111111111*v_comp)));

    // activation and timescale of state variable 'h'
    double h_inf_Na = 1.0/(exp(0.16129032258064516*v_comp + 10.483870967741936) + 1.0);
    double tau_h_Na = 0.3115264797507788/((-0.0091000000000000004*v_comp - 0.68261830000000012)/(1.0 - 3277527.8765015295*exp(0.20000000000000001*v_comp)) + (0.024*v_comp + 1.200312)/(1.0 - 4.5282043263959816e-5*exp(-0.20000000000000001*v_comp)));

    // advance state variable 'm' one timestep
    double p_m_Na = exp( -dt / tau_m_Na );
    m_Na_ *= p_m_Na ;
    m_Na_ += (1. - p_m_Na) *  m_inf_Na;

    // advance state variable 'h' one timestep
    double p_h_Na = exp( -dt / tau_h_Na );
    h_Na_ *= p_h_Na ;
    h_Na_ += (1. - p_h_Na) *  h_inf_Na;

    // compute the conductance of the sodium channel
    double g_Na = gbar_Na_ * pow( m_Na_, 3 ) * h_Na_;

    // add to variables for numerical integration
    g_val += g_Na / 2.;
    i_val += g_Na * ( e_Na_ - v_comp / 2. );
  }

  return std::make_pair(g_val, i_val);
}
////////////////////////////////////////////////////////////////////////////////


// K channel ///////////////////////////////////////////////////////////////////
nest::K::K()
    : n_K_(0.0)
    , gbar_K_(0.0)
    , e_K_(-85.)
{
}
nest::K::K( const DictionaryDatum& channel_params )
    : n_K_(0.0)
    , gbar_K_(0.0)
    , e_K_(-85.)
{
    // update sodium channel parameters
    if( channel_params->known( "g_K" ) )
        gbar_K_ = getValue< double >( channel_params, "g_K" );
    if( channel_params->known( "e_Na" ) )
        e_K_ = getValue< double >( channel_params, "e_K" );
}

std::pair< double, double > nest::K::f_numstep( const double v_comp, const double dt)
{
  double g_val = 0., i_val = 0.;

  if (gbar_K_ > 1e-9)
  {
    // activation and timescale of state variable 'm'
    double n_inf_K = 0.02*(v_comp - 25.0)/((1.0 - exp((25.0 - v_comp)/9.0))*((-0.002)*(v_comp - 25.0)/(1.0 - exp((v_comp - 25.0)/9.0)) + 0.02*(v_comp - 25.0)/(1.0 - exp((25.0 - v_comp)/9.0))));
    double tau_n_K = 0.3115264797507788/((-0.002)*(v_comp - 25.0)/(1.0 - exp((v_comp - 25.0)/9.0)) + 0.02*(v_comp - 25.0)/(1.0 - exp((25.0 - v_comp)/9.0)));

    // advance state variable 'm' one timestep
    double p_n_K = exp(-dt / tau_n_K);
    n_K_ *= p_n_K;
    n_K_ += (1. - p_n_K) *  n_inf_K;

    // compute the conductance of the potassium channel
    double g_K = gbar_K_ * n_K_;

    // add to variables for numerical integration
    g_val += g_K / 2.;
    i_val += g_K * ( e_K_ - v_comp / 2. );
  }

  return std::make_pair(g_val, i_val);
}
////////////////////////////////////////////////////////////////////////////////


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


