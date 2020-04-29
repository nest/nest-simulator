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
nest::ExpCond::ExpCond(){
  set_params(5.); // default conductance window has time-scale of 5 ms
}
nest::ExpCond::ExpCond(double tau){
  set_params(tau);
}


void nest::ExpCond::init(){
  m_dt = Time::get_resolution().get_ms();

  m_p = std::exp(-m_dt / m_tau);
  m_g = 0.; m_g0 = 0.;

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
  m_g0 = m_g;
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
  m_dt = Time::get_resolution().get_ms();

  m_p_r = std::exp(-m_dt / m_tau_r); m_p_d = std::exp(-m_dt / m_tau_d);
  m_g_r = 0.; m_g_d = 0.;
  m_g = 0.; m_g0 = 0.;

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
    m_dt = dt;
    m_p_r = std::exp(-dt / m_tau_r); m_p_d = std::exp(-dt / m_tau_d);
  }
  // update conductance
  m_g_r *= m_p_r; m_g_d *= m_p_d;
  m_g0 = m_g;
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
    return 0.03 * (m_e_r - v) * std::exp(-0.1 * v) / std::pow(0.3 * std::exp(-0.1*v) + 1.0, 2)
            - 1. / (0.3 * std::exp(-0.1*v) + 1.0);
};
////////////////////////////////////////////////////////////////////////////////


// functions for synapse integration ///////////////////////////////////////////
nest::Synapse::Synapse(int comp_ind){
  m_comp_ind = comp_ind;
  // base voltage dependence for current based synapse with exponentially shaped
  // PCS's
  m_v_dep = new VoltageDependence();
  m_cond_w = new ExpCond();

  // VoltageDependence* v_dep = new VoltageDependence();
  // ConductanceWindow* cond_w = new ExpCond();

  // m_v_dep.push_back(v_dep);
  // m_cond_w.push_back(cond_w);
};

void nest::Synapse::handle(SpikeEvent& e){
  // for(std::vector< ConductanceWindow* >::iterator cond_w_it = m_cond_w.begin();
  //     cond_w_it != m_cond_w.end(); cond_w_it++){
  //   (*cond_w_it)->handle(e);
  // }
  m_cond_w->handle(e);
};

void nest::Synapse::update(const long lag){
  // for(std::vector< ConductanceWindow* >::iterator cond_w_it = m_cond_w.begin();
  //     cond_w_it != m_cond_w.end(); cond_w_it++){
  //   (*cond_w_it)->update(lag);
  // }
  m_cond_w->update(lag);
};

std::pair< double, double > nest::Synapse::f_numstep(std::vector< double >& v_comp){
  // double g_val = 0., i_val = 0.;
  // std::pair< double, double > g_aux(0., 0.);
  // double f_aux = 0., df_dv_aux = 0.;
  // for(int ii = 0; ii < int(m_v_dep.size()); ii++){
  //   // get conductances and voltage dependent factors from synapse
  //   g_aux     = m_cond_w[ii]->get_cond_pair();
  //   f_aux     = -m_v_dep[ii]->f(v_comp[m_comp_ind]);
  //   df_dv_aux = -m_v_dep[ii]->df_dv(v_comp[m_comp_ind]);
  //   // consturct values for integration step
  //   g_val += g_aux.first * df_dv_aux / 2.;
  //   i_val += (g_aux.first + g_aux.second) / 2. * f_aux - g_aux.first * df_dv_aux / 2.;
  // }
  double g_val = 0., i_val = 0.;
  // get conductances and voltage dependent factors from synapse
  std::pair< double, double > g_aux = m_cond_w->get_cond_pair();
  double f_aux     = -m_v_dep->f(v_comp[m_comp_ind]);
  double df_dv_aux = -m_v_dep->df_dv(v_comp[m_comp_ind]);
  // consturct values for integration step
  g_val += g_aux.first * df_dv_aux / 2.;
  i_val += (g_aux.first + g_aux.second) / 2. * f_aux - \
           g_aux.first * df_dv_aux * v_comp[m_comp_ind] / 2.;

  return std::make_pair(g_val, i_val);
};

// default AMPA synapse
nest::AMPASyn::AMPASyn(int comp_ind) : Synapse(comp_ind){
  m_comp_ind = comp_ind;

  m_v_dep = new DrivingForce(0.);
  m_cond_w = new Exp2Cond(.2, 3.);

  // VoltageDependence* m_v_dep = new DrivingForce(0.);
  // ConductanceWindow* m_cond_w = new Exp2Cond(.2, 3.);

  // m_v_dep.push_back(v_dep);
  // m_cond_w.push_back(cond_w);
};

// default GABA synapse
nest::GABASyn::GABASyn(int comp_ind) : Synapse(comp_ind){
  m_comp_ind = comp_ind;

  m_v_dep = new DrivingForce(-80.);
  m_cond_w = new Exp2Cond(.2, 10.);

  // VoltageDependence* v_dep = new DrivingForce(-80.);
  // ConductanceWindow* cond_w = new Exp2Cond(.2, 10.);

  // m_v_dep.push_back(v_dep);
  // m_cond_w.push_back(cond_w);
};

// default NMDA synapse
nest::NMDASyn::NMDASyn(int comp_ind) : Synapse(comp_ind){
  m_comp_ind = comp_ind;

  m_v_dep = new NMDA(0.);
  m_cond_w = new Exp2Cond(.2, 43.);
};

// default AMPA+NMDA synapse
nest::AMPA_NMDASyn::AMPA_NMDASyn(int comp_ind) : Synapse(comp_ind){
  AMPA_NMDASyn(comp_ind, 2.); // default nmda ratio of 2
};
nest::AMPA_NMDASyn::AMPA_NMDASyn(int comp_ind, double nmda_ratio) : Synapse(comp_ind){
  m_comp_ind = comp_ind;
  m_nmda_ratio = nmda_ratio;

  m_ampa = new AMPASyn(comp_ind);
  m_nmda = new NMDASyn(comp_ind);
};

void nest::AMPA_NMDASyn::handle(SpikeEvent& e){
  // possible bug, spike should go to both synapses
  m_ampa->handle(e);
  m_nmda->handle(e);
};

void nest::AMPA_NMDASyn::update(const long lag){
  m_ampa->update(lag);
  m_nmda->update(lag);
};

std::pair< double, double > nest::AMPA_NMDASyn::f_numstep(std::vector< double >& v_comp){
  std::pair< double, double > gf_1 = m_ampa->f_numstep(v_comp);
  std::pair< double, double > gf_2 = m_nmda->f_numstep(v_comp);

  return std::make_pair(gf_1.first + m_nmda_ratio * gf_2.first,
                        gf_1.second + m_nmda_ratio * gf_2.second);
}

////////////////////////////////////////////////////////////////////////////////
