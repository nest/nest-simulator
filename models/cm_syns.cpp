#include "cm_syns.h"


// spike handling for conductance windows //////////////////////////////////////
void nest::ConductanceWindow::handle(SpikeEvent& e)
{
  m_b_spikes.add_value(
    e.get_rel_delivery_steps(kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}
////////////////////////////////////////////////////////////////////////////////


// exponential conductance window //////////////////////////////////////////////
nest::ExpCond::ExpCond(){
  set_params( 5. ); // default conductance window has time-scale of 5 ms
}
nest::ExpCond::ExpCond(double tau){
  set_params( tau );
}

void nest::ExpCond::init()
{
  const double h = Time::get_resolution().get_ms();

  m_p = std::exp( -h / m_tau );
  m_g = 0.; m_g0 = 0.;

  m_b_spikes.clear();
};

void nest::ExpCond::set_params( double tau )
{
    m_tau = tau;
};

void nest::ExpCond::update( const long lag )
{
  // update conductance
  m_g *= m_p;
  // add spikes
  m_g += m_b_spikes.get_value( lag );
};
////////////////////////////////////////////////////////////////////////////////


// double exponential conductance window ///////////////////////////////////////
nest::Exp2Cond::Exp2Cond(){
  // default conductance window has rise-time of 2 ms and decay time of 5 ms
  set_params(.2, 5.);
}
nest::Exp2Cond::Exp2Cond(double tau_r, double tau_d){
  set_params(tau_r, tau_d);
}

void nest::Exp2Cond::init(){
  const double h = Time::get_resolution().get_ms();

  m_p_r = std::exp( -h / m_tau_r ); m_p_d = std::exp( -h / m_tau_d );
  m_g_r = 0.; m_g_d = 0.;
  m_g = 0.;

  m_b_spikes.clear();
};

void nest::Exp2Cond::set_params(double tau_r, double tau_d){
  m_tau_r = tau_r; m_tau_d = tau_d;
  // set the normalization
  double tp = (m_tau_r * m_tau_d) / (m_tau_d - m_tau_r) * std::log( m_tau_d / m_tau_r );
  m_norm = 1. / ( -std::exp( -tp / m_tau_r ) + std::exp( -tp / m_tau_d ) );
};

void nest::Exp2Cond::update( const long lag ){
  // update conductance
  m_g_r *= m_p_r; m_g_d *= m_p_d;
  // add spikes
  double s_val = m_b_spikes.get_value( lag ) * m_norm;
  m_g_r -= s_val;
  m_g_d += s_val;
  // compute synaptic conductance
  m_g = m_g_r + m_g_d;
};
////////////////////////////////////////////////////////////////////////////////


// driving force ///////////////////////////////////////////////////////////////
double nest::DrivingForce::f( double v ){
    return m_e_r - v;
};

double nest::DrivingForce::df_dv( double v ){
    return -1.;
};
////////////////////////////////////////////////////////////////////////////////


// NMDA synapse factor /////////////////////////////////////////////////////////
double nest::NMDA::f( double v ){
    return (m_e_r - v) / ( 1. + 0.3 * std::exp( -.1 * v ) );
};

double nest::NMDA::df_dv( double v ){
    return 0.03 * ( m_e_r - v ) * std::exp( -0.1 * v ) / std::pow( 0.3 * std::exp( -0.1*v ) + 1.0, 2 )
            - 1. / ( 0.3 * std::exp( -0.1*v ) + 1.0 );
};
////////////////////////////////////////////////////////////////////////////////


// functions for synapse integration ///////////////////////////////////////////
nest::Synapse::Synapse()
{
  // base voltage dependence for current based synapse with exponentially shaped
  // PCS's
  m_v_dep  = std::unique_ptr< VoltageDependence >( new VoltageDependence() );
  m_cond_w = std::unique_ptr< ExpCond >( new ExpCond() );
};

void nest::Synapse::handle( SpikeEvent& e )
{
  m_cond_w->handle( e );
};

void nest::Synapse::update( const long lag )
{
  m_cond_w->update( lag );
};

std::pair< double, double > nest::Synapse::f_numstep(double v_comp)
{
  // get conductances and voltage dependent factors from synapse
  double g_aux( m_cond_w->get_cond() );
  double f_aux     = m_v_dep->f(v_comp);
  double df_dv_aux = m_v_dep->df_dv(v_comp);
  // consturct values for integration step
  double g_val( -g_aux * df_dv_aux / 2. );
  double i_val(  g_aux * ( f_aux - df_dv_aux * v_comp / 2. ) );

  return std::make_pair( g_val, i_val );
};

// default AMPA synapse
nest::AMPASyn::AMPASyn() : Synapse()
{
  m_v_dep  = std::unique_ptr< DrivingForce >( new DrivingForce( 0. ) );
  m_cond_w = std::unique_ptr< Exp2Cond >( new Exp2Cond( .2, 3. ) );
};

// default GABA synapse
nest::GABASyn::GABASyn() : Synapse()
{
  m_v_dep  = std::unique_ptr< DrivingForce >( new DrivingForce( -80. ) );
  m_cond_w = std::unique_ptr< Exp2Cond >( new Exp2Cond( .2, 10. ) );
};

// default NMDA synapse
nest::NMDASyn::NMDASyn() : Synapse()
{
  m_v_dep = std::unique_ptr< NMDA >( new NMDA( 0. ) );
  m_cond_w = std::unique_ptr< Exp2Cond >( new Exp2Cond( .2, 43. ) );
};

// default AMPA+NMDA synapse
nest::AMPA_NMDASyn::AMPA_NMDASyn() : Synapse()
{
  m_nmda_ratio = 2.; // default nmda ratio of 2

  m_ampa = std::unique_ptr< AMPASyn >( new AMPASyn() );
  m_nmda = std::unique_ptr< NMDASyn >( new NMDASyn() );
};
nest::AMPA_NMDASyn::AMPA_NMDASyn( double nmda_ratio ) : Synapse()
{
  m_nmda_ratio = nmda_ratio;

  m_ampa = std::unique_ptr< AMPASyn >( new AMPASyn() );
  m_nmda = std::unique_ptr< NMDASyn >( new NMDASyn() );
};

void nest::AMPA_NMDASyn::handle( SpikeEvent& e )
{
  m_ampa->handle( e );
  m_nmda->handle( e );
};

void nest::AMPA_NMDASyn::update( const long lag )
{
  m_ampa->update( lag );
  m_nmda->update( lag );
};

std::pair< double, double > nest::AMPA_NMDASyn::f_numstep( double v_comp )
{
  std::pair< double, double > gf_1 = m_ampa->f_numstep( v_comp );
  std::pair< double, double > gf_2 = m_nmda->f_numstep( v_comp );

  return std::make_pair( gf_1.first  + m_nmda_ratio * gf_2.first,
                         gf_1.second + m_nmda_ratio * gf_2.second );
}

double nest::AMPA_NMDASyn::f( double v )
{
  return m_ampa->f( v ) + m_nmda_ratio * m_nmda->f( v );
}
////////////////////////////////////////////////////////////////////////////////
