#include "synapses_neat.h"

// spike handling for conductance windows //////////////////////////////////////
void nest::ConductanceWindow::handle(SpikeEvent& e){
  assert( e.get_delay_steps() > 0 );

  m_b_spikes.add_value(
    e.get_rel_delivery_steps(kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}
////////////////////////////////////////////////////////////////////////////////


// exponential conductance window //////////////////////////////////////////////
void nest::ExpCond::init(){
  m_dt = Time::get_resolution().get_ms();

  m_p = std::exp(-m_dt / m_tau);
  m_g = 0.;

  m_b_spikes.clear();
};

void nest::ExpCond::set_params(double tau){
    m_tau = tau;
};

void nest::ExpCond::update( const long lag ){
  const double dt = Time::get_resolution().get_ms();
  if(abs(dt - m_dt) > 1.0e-9){
    m_dt = dt;
    m_p = std::exp(-dt / m_tau);
  }
  // update conductance
  m_g *= m_p;
  // add spikes
  m_g += m_b_spikes.get_value( lag );
};
////////////////////////////////////////////////////////////////////////////////


// double exponential conductance window ///////////////////////////////////////
void nest::Exp2Cond::init(){
  m_dt = Time::get_resolution().get_ms();

  m_p_r = std::exp(-m_dt / m_tau_r); m_p_d = std::exp(-m_dt / m_tau_d);
  m_g_r = 0.; m_g_d = 0.;
  m_g = 0.;

  m_b_spikes.clear();
};

void nest::Exp2Cond::set_params(double tau_r, double tau_d){
    m_tau_r = tau_r; m_tau_d = tau_d;
    // set the normalization
    double tp = (m_tau_r * m_tau_d) / (m_tau_d - m_tau_r) * std::log(m_tau_d / m_tau_r);
    m_norm = 1. / (-std::exp(-tp / m_tau_r) + std::exp(-tp / m_tau_d));
};

void nest::Exp2Cond::update( const long lag ){
  const double dt = Time::get_resolution().get_ms();
  if(abs(dt - m_dt) > 1.0e-9){
    m_p_r = std::exp(-dt / m_tau_r); m_p_d = std::exp(-dt / m_tau_d);
  }
  // update conductance
  m_g_r *= m_p_r; m_g_d *= m_p_d;
  m_g = m_g_r + m_g_d;
  // add spikes
  m_g_r -= m_norm * m_b_spikes.get_value( lag );
  m_g_d += m_norm * m_b_spikes.get_value( lag );
};
////////////////////////////////////////////////////////////////////////////////


// driving force ///////////////////////////////////////////////////////////////
double nest::DrivingForce::f(double v){
    return m_e_r - v;
};

double nest::DrivingForce::df_dv(double v){
    return -1.;
};
////////////////////////////////////////////////////////////////////////////////


// NMDA synapse factor /////////////////////////////////////////////////////////
double nest::NMDA::f(double v){
    return (m_e_r - v) / (1. + 0.3 * std::exp(-.1 * v));
};

double nest::NMDA::df_dv(double v){
    return 0.03 * (m_e_r - v) * std::exp(-0.1 * v) / pow(0.3 * std::exp(-0.1*v) + 1.0, 2)
            - 1. / (0.3 * std::exp(-0.1*v) + 1.0);
};
////////////////////////////////////////////////////////////////////////////////
