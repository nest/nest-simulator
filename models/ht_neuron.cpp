/*
 *  ht_neuron.cpp
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

#include "ht_neuron.h"

#ifdef HAVE_GSL_1_11

#include "universal_data_logger_impl.h"

#include <cmath>

namespace nest{
  
  // Could be a param, but fixed by Hill-Tononi. We need
  // it several places, thus we put it here. It's a hack.
  const double_t KNa_D_EQ = 0.001;

  RecordablesMap<ht_neuron> ht_neuron::recordablesMap_;

  template <>
  void RecordablesMap<ht_neuron>::create()
  {
    insert_(names::V_m, &ht_neuron::get_y_elem_<ht_neuron::State_::VM>);
    insert_(Name("Theta"), &ht_neuron::get_y_elem_<ht_neuron::State_::THETA>);
    insert_(Name("g_AMPA"), &ht_neuron::get_y_elem_<ht_neuron::State_::G_AMPA>);
    insert_(Name("g_NMDA"), &ht_neuron::get_y_elem_<ht_neuron::State_::G_NMDA>);
    insert_(Name("g_GABAA"), &ht_neuron::get_y_elem_<ht_neuron::State_::G_GABA_A>);
    insert_(Name("g_GABAB"), &ht_neuron::get_y_elem_<ht_neuron::State_::G_GABA_B>); 
    insert_(Name("r_potassium"), &ht_neuron::get_r_potassium_);
    insert_(Name("g_spike"), &ht_neuron::get_g_spike_);
    insert_(Name("I_NaP"), &ht_neuron::get_I_NaP_); 
    insert_(Name("I_KNa"), &ht_neuron::get_I_KNa_); 
    insert_(Name("I_T"), &ht_neuron::get_I_T_); 
    insert_(Name("I_h"), &ht_neuron::get_I_h_); 
 }

/* ---------------------------------------------------------------- 
 * Iteration function
 * ---------------------------------------------------------------- */

  extern "C"
  inline int ht_neuron_dynamics (double, const double y[], double f[], 
				void* pnode)
  { 
    // shorthand
    typedef nest::ht_neuron::State_ S;

    // get access to node so we can almost work as in a member class
    assert(pnode);
    nest::ht_neuron& node =  *(reinterpret_cast<nest::ht_neuron*>(pnode));

    // easier access to membrane potential
    const double_t& V = y[S::VM];

    // Synaptic channels    
    double_t I_syn = 0;

    // Calculate sum of all synaptic channels.
    // Sign convention: For each current, write I = - g * ( V - E )
    //    then dV/dt ~ Sum(I) 
    // NMDA has instantaneous de-blocking thru sigmoidal function, see Lumer et al (1997)
    I_syn += - y[S::G_AMPA] * (V - node.P_.AMPA_E_rev);
    I_syn += - y[S::G_NMDA] * (V - node.P_.NMDA_E_rev) 
      / ( 1 + std::exp((node.P_.NMDA_Vact - V) / node.P_.NMDA_Sact) );
    I_syn += - y[S::G_GABA_A] * (V - node.P_.GABA_A_E_rev);
    I_syn += - y[S::G_GABA_B] * (V - node.P_.GABA_B_E_rev);
    
    // The spike current is only activate immediately after a spike.
    const double_t I_spike = node.S_.g_spike_ 
      ? - (V - node.P_.E_K) / node.P_.Tau_spike
      : 0;
    
    // leak currents
    const double_t I_Na = - node.P_.g_NaL*(V - node.P_.E_Na);
    const double_t I_K  = - node.P_.g_KL*(V - node.P_.E_K);
    
    // intrinsic currents
    // I_Na(p), m_inf^3 according to Compte et al, J Neurophysiol 2003 89:2707
    const double_t INaP_thresh = -55.7;
    const double_t INaP_slope = 7.7;
    const double_t m_inf_NaP = 1.0 / (1.0 + std::exp(-(V- INaP_thresh)/INaP_slope));
    node.S_.I_NaP_ = - node.P_.NaP_g_peak * std::pow(m_inf_NaP, 3.0) * (V - node.P_.NaP_E_rev);

    // I_DK
    const double_t d_half = 0.25;
    const double_t m_inf_KNa = 1.0 / (1.0 + std::pow(d_half/y[S::IKNa_D], 3.5));
    node.S_.I_KNa_ = - node.P_.KNa_g_peak * m_inf_KNa * (V - node.P_.KNa_E_rev);
    
    // I_T
    const double_t m_inf_T = 1.0/(1.0 + std::exp(-(V+59.0)/6.2));
    const double_t h_inf_T = 1.0/(1.0 + std::exp((V + 83.0)/4));
    node.S_.I_T_ = - node.P_.T_g_peak * y[S::IT_m] * y[S::IT_m] * y[S::IT_h] * (V - node.P_.T_E_rev);

    // I_h
    const double_t I_h_Vthreshold = -75.0;
    const double_t m_inf_h = 1.0/(1.0 + std::exp((V - I_h_Vthreshold)/5.5));
    node.S_.I_h_ = - node.P_.h_g_peak * y[S::Ih_m] * (V - node.P_.h_E_rev);

    // delta V
    f[S::VM] = (I_Na + I_K + I_syn + node.S_.I_NaP_ + node.S_.I_KNa_ + node.S_.I_T_
		+ node.S_.I_h_ + node.B_.I_stim_)/node.P_.Tau_m + I_spike;

    // delta Theta
    f[S::THETA] = -(y[S::THETA] - node.P_.Theta_eq)/node.P_.Tau_theta;

    // Synaptic channels

    // AMPA
    f[S::DG_AMPA] = -y[S::DG_AMPA]/node.P_.AMPA_Tau_1;
    f[S::G_AMPA] = y[S::DG_AMPA] - y[S::G_AMPA]/node.P_.AMPA_Tau_2;

    // NMDA
    f[S::DG_NMDA] = -y[S::DG_NMDA]/node.P_.NMDA_Tau_1;
    f[S::G_NMDA] = y[S::DG_NMDA] - y[S::G_NMDA]/node.P_.NMDA_Tau_2;

    // GABA_A
    f[S::DG_GABA_A] = -y[S::DG_GABA_A]/node.P_.GABA_A_Tau_1;
    f[S::G_GABA_A] = y[S::DG_GABA_A] - y[S::G_GABA_A]/node.P_.GABA_A_Tau_2;

    // GABA_B
    f[S::DG_GABA_B] = -y[S::DG_GABA_B]/node.P_.GABA_B_Tau_1;
    f[S::G_GABA_B] = y[S::DG_GABA_B] - y[S::G_GABA_B]/node.P_.GABA_B_Tau_2;

    // I_KNa
    const double_t D_influx_peak = 0.025;
    const double_t tau_D = 1250.0;  // yes, 1.25s
    const double_t D_thresh = -10.0;
    const double_t D_slope = 5.0;
    const double_t D_influx = 1.0/(1.0 + std::exp(-(V-D_thresh)/D_slope));

    // equation modified from y[](1-D_eq) to (y[]-D_eq), since we'd not
    // be converging to equilibrium otherwise
    f[S::IKNa_D] = D_influx_peak * D_influx - (y[S::IKNa_D]-KNa_D_EQ)/tau_D;

    // I_T
    const double_t tau_m_T = 0.22/(std::exp(-(V + 132.0)/16.7)+std::exp((V + 16.8)/18.2)) + 0.13;
    const double_t tau_h_T = 8.2 + (56.6 + 0.27 * std::exp((V + 115.2)/5.0))/(1.0 + std::exp((V + 86.0)/3.2));
    f[S::IT_m] = (m_inf_T - y[S::IT_m]) / tau_m_T;
    f[S::IT_h] = (h_inf_T - y[S::IT_h]) / tau_h_T;

    // I_h
    const double_t tau_m_h = 1.0/(std::exp(-14.59 - 0.086 * V) + std::exp(-1.87 + 0.0701 * V));
    f[S::Ih_m] = (m_inf_h - y[S::Ih_m]) / tau_m_h;

    return GSL_SUCCESS;
  }
 
  /* ---------------------------------------------------------------- 
   * Default constructors defining default parameters and state
   * ---------------------------------------------------------------- */
  
  nest::ht_neuron::Parameters_::Parameters_()
    : E_Na           ( 30.0 ), // 30 mV
      E_K            (-90.0 ), // -90 mV
      g_NaL          (  0.2 ), // 0.2
      g_KL           (  1.0 ), // 1.0 - 1.85
      Tau_m          ( 16.0 ), // ms
      Theta_eq       (-51.0 ), // mV
      Tau_theta      (  2.0 ), // ms
      Tau_spike      (  1.75), // ms
      t_spike        (  2.0 ), // ms
      AMPA_g_peak    (  0.1 ),
      AMPA_Tau_1     (  0.5 ), // ms
      AMPA_Tau_2     (  2.4 ), // ms
      AMPA_E_rev     (  0.0 ), // mV
      NMDA_g_peak    (  0.075 ),
      NMDA_Tau_1     (  4.0 ), // ms
      NMDA_Tau_2     ( 40.0 ), // ms
      NMDA_E_rev     (  0.0 ), // mV
      NMDA_Vact      (-58.0 ), // mV
      NMDA_Sact      (  2.5 ), // mV
      GABA_A_g_peak  (  0.33),
      GABA_A_Tau_1   (  1.0 ), // ms
      GABA_A_Tau_2   (  7.0 ), // ms
      GABA_A_E_rev   (-70.0 ), // mV
      GABA_B_g_peak  (  0.0132 ),
      GABA_B_Tau_1   ( 60.0 ), // ms
      GABA_B_Tau_2   (200.0 ), // ms
      GABA_B_E_rev   (-90.0 ), // mV
      NaP_g_peak     (  1.0 ),
      NaP_E_rev      ( 30.0 ), // mV
      KNa_g_peak     (  1.0 ),
      KNa_E_rev      (-90.0 ), // mV
      T_g_peak       (  1.0 ),
      T_E_rev        (  0.0 ), // mV
      h_g_peak       (  1.0 ),
      h_E_rev        (-40.0 )  // mV
  {}

  nest::ht_neuron::State_::State_()
    : r_potassium_(0),
      g_spike_(false),
      I_NaP_(0.0),
      I_KNa_(0.0),
      I_T_(0.0),
      I_h_(0.0)
  {
    for ( size_t i = 0 ; i < STATE_VEC_SIZE ; ++i )
      y_[i] = 0;
    y_[IKNa_D] = KNa_D_EQ;
  }

  nest::ht_neuron::State_::State_(const Parameters_& p)
    : r_potassium_ (0),
      g_spike_     (false),
      I_NaP_(0.0),
      I_KNa_(0.0),
      I_T_(0.0),
      I_h_(0.0)
  {
    y_[VM] = (p.g_NaL*p.E_Na+p.g_KL*p.E_K)/(p.g_NaL+p.g_KL);
    y_[THETA] = p.Theta_eq;

    for ( size_t i = 2 ; i < STATE_VEC_SIZE ; ++i )
      y_[i] = 0.0;

    y_[IKNa_D] = KNa_D_EQ;
  }

  nest::ht_neuron::State_::State_(const State_& s)
    : r_potassium_(s.r_potassium_),
      g_spike_(s.g_spike_),
      I_NaP_(s.I_NaP_),
      I_KNa_(s.I_KNa_),
      I_T_(s.I_T_),
      I_h_(s.I_h_)
  {
    for(size_t i = 0; i < STATE_VEC_SIZE; ++i)
      y_[i] = s.y_[i];
  }

  nest::ht_neuron::State_& nest::ht_neuron::State_::operator=(const State_& s)
  {
    if(this == &s)
      {
	return *this;
      }

    r_potassium_ = s.r_potassium_;
    g_spike_ = s.g_spike_;
    I_NaP_ = s.I_NaP_;
    I_KNa_ = s.I_KNa_;
    I_T_   = s.I_T_;
    I_h_   = s.I_h_;

    for(size_t i = 0; i < STATE_VEC_SIZE; ++i)
      y_[i] = s.y_[i];
    
    return *this;
  }

  nest::ht_neuron::State_::~State_()
  {
  }
  
  /* ---------------------------------------------------------------- 
   * Parameter and state extractions and manipulation functions
   * ---------------------------------------------------------------- */

  void nest::ht_neuron::Parameters_::get(DictionaryDatum &d) const
  {
    def<double_t>(d, "E_Na",          E_Na);
    def<double_t>(d, "E_K",           E_K);
    def<double_t>(d, "g_NaL",         g_NaL);
    def<double_t>(d, "g_KL",          g_KL);
    def<double_t>(d, "Tau_m",         Tau_m);
    def<double_t>(d, "Theta_eq",      Theta_eq);
    def<double_t>(d, "Tau_theta",     Tau_theta);
    def<double_t>(d, "Tau_spike",     Tau_spike);
    def<double_t>(d, "spike_duration",       t_spike);
    def<double_t>(d, "AMPA_g_peak",   AMPA_g_peak);
    def<double_t>(d, "AMPA_Tau_1",    AMPA_Tau_1);
    def<double_t>(d, "AMPA_Tau_2",    AMPA_Tau_2);
    def<double_t>(d, "AMPA_E_rev",    AMPA_E_rev);
    def<double_t>(d, "NMDA_g_peak",   NMDA_g_peak);
    def<double_t>(d, "NMDA_Tau_1",    NMDA_Tau_1);
    def<double_t>(d, "NMDA_Tau_2",    NMDA_Tau_2);
    def<double_t>(d, "NMDA_E_rev",    NMDA_E_rev);
    def<double_t>(d, "NMDA_Vact",     NMDA_Vact);
    def<double_t>(d, "NMDA_Sact",     NMDA_Sact);
    def<double_t>(d, "GABA_A_g_peak", GABA_A_g_peak);
    def<double_t>(d, "GABA_A_Tau_1",  GABA_A_Tau_1);
    def<double_t>(d, "GABA_A_Tau_2",  GABA_A_Tau_2);
    def<double_t>(d, "GABA_A_E_rev",  GABA_A_E_rev);
    def<double_t>(d, "GABA_B_g_peak", GABA_B_g_peak);
    def<double_t>(d, "GABA_B_Tau_1",  GABA_B_Tau_1);
    def<double_t>(d, "GABA_B_Tau_2",  GABA_B_Tau_2);
    def<double_t>(d, "GABA_B_E_rev",  GABA_B_E_rev);
    def<double_t>(d, "NaP_g_peak",    NaP_g_peak);
    def<double_t>(d, "NaP_E_rev",     NaP_E_rev);
    def<double_t>(d, "KNa_g_peak",    KNa_g_peak);
    def<double_t>(d, "KNa_E_rev",     KNa_E_rev);
    def<double_t>(d, "T_g_peak",      T_g_peak);
    def<double_t>(d, "T_E_rev",       T_E_rev);
    def<double_t>(d, "h_g_peak",      h_g_peak);
    def<double_t>(d, "h_E_rev",       h_E_rev);
  } 

  void nest::ht_neuron::Parameters_::set(const DictionaryDatum& d)
  {
    updateValue<double_t>(d, "E_Na",          E_Na);
    updateValue<double_t>(d, "E_K",           E_K);
    updateValue<double_t>(d, "g_NaL",         g_NaL);
    updateValue<double_t>(d, "g_KL",          g_KL);
    updateValue<double_t>(d, "Tau_m",         Tau_m);
    updateValue<double_t>(d, "Theta_eq",      Theta_eq);
    updateValue<double_t>(d, "Tau_theta",     Tau_theta);
    updateValue<double_t>(d, "Tau_spike",     Tau_spike);
    updateValue<double_t>(d, "spike_duration",       t_spike);
    updateValue<double_t>(d, "AMPA_g_peak",   AMPA_g_peak);
    updateValue<double_t>(d, "AMPA_Tau_1",    AMPA_Tau_1);
    updateValue<double_t>(d, "AMPA_Tau_2",    AMPA_Tau_2);
    updateValue<double_t>(d, "AMPA_E_rev",    AMPA_E_rev);
    updateValue<double_t>(d, "NMDA_g_peak",   NMDA_g_peak);
    updateValue<double_t>(d, "NMDA_Tau_1",    NMDA_Tau_1);
    updateValue<double_t>(d, "NMDA_Tau_2",    NMDA_Tau_2);
    updateValue<double_t>(d, "NMDA_E_rev",    NMDA_E_rev);
    updateValue<double_t>(d, "NMDA_Vact",     NMDA_Vact);
    updateValue<double_t>(d, "NMDA_Sact",     NMDA_Sact);
    updateValue<double_t>(d, "GABA_A_g_peak", GABA_A_g_peak);
    updateValue<double_t>(d, "GABA_A_Tau_1",  GABA_A_Tau_1);
    updateValue<double_t>(d, "GABA_A_Tau_2",  GABA_A_Tau_2);
    updateValue<double_t>(d, "GABA_A_E_rev",  GABA_A_E_rev);
    updateValue<double_t>(d, "GABA_B_g_peak", GABA_B_g_peak);
    updateValue<double_t>(d, "GABA_B_Tau_1",  GABA_B_Tau_1);
    updateValue<double_t>(d, "GABA_B_Tau_2",  GABA_B_Tau_2);
    updateValue<double_t>(d, "GABA_B_E_rev",  GABA_B_E_rev);
    updateValue<double_t>(d, "NaP_g_peak",    NaP_g_peak);
    updateValue<double_t>(d, "NaP_E_rev",     NaP_E_rev);
    updateValue<double_t>(d, "KNa_g_peak",    KNa_g_peak);
    updateValue<double_t>(d, "KNa_E_rev",     KNa_E_rev);
    updateValue<double_t>(d, "T_g_peak",      T_g_peak);
    updateValue<double_t>(d, "T_E_rev",       T_E_rev);
    updateValue<double_t>(d, "h_g_peak",      h_g_peak);
    updateValue<double_t>(d, "h_E_rev",       h_E_rev);
  }

  void nest::ht_neuron::State_::get(DictionaryDatum &d) const
  {
    def<double_t>(d, "V_m",     y_[VM]); // Membrane potential
    def<double_t>(d, "Theta",   y_[THETA]); // Threshold
  }

  void nest::ht_neuron::State_::set(const DictionaryDatum& d, 
				   const Parameters_&)  
  {
    updateValue<double_t>(d, "V_m",       y_[VM]);
    updateValue<double_t>(d, "Theta",     y_[THETA]);
  }

  nest::ht_neuron::Buffers_::Buffers_(ht_neuron& n)
    : logger_(n),
      spike_inputs_(std::vector<RingBuffer>(SUP_SPIKE_RECEPTOR-1)),
      s_(0),
      c_(0),
      e_(0)
  {
  // Initialization of the remaining members is deferred to
  // init_buffers_().
  }

  nest::ht_neuron::Buffers_::Buffers_(const Buffers_&, ht_neuron& n)
    : logger_(n),
      spike_inputs_(std::vector<RingBuffer>(SUP_SPIKE_RECEPTOR-1)),
      s_(0),
      c_(0),
      e_(0)
  {
    // Initialization of the remaining members is deferred to
    // init_buffers_().
  }

  /* ---------------------------------------------------------------- 
   * Default and copy constructor for node, and destructor
   * ---------------------------------------------------------------- */

  nest::ht_neuron::ht_neuron()
    : Archiving_Node(), 
      P_(), 
      S_(P_),
      B_(*this)
  {
    recordablesMap_.create();
  }

  nest::ht_neuron::ht_neuron(const ht_neuron& n)
    : Archiving_Node(n), 
      P_(n.P_), 
      S_(n.S_),
      B_(n.B_,*this)
  {}

  nest::ht_neuron::~ht_neuron()
  {
    // GSL structs may not be initialized, so we need to protect destruction.
    if ( B_.e_ ) gsl_odeiv_evolve_free(B_.e_);
    if ( B_.c_ ) gsl_odeiv_control_free(B_.c_);
    if ( B_.s_ ) gsl_odeiv_step_free(B_.s_);
  }
  
  /* ---------------------------------------------------------------- 
   * Node initialization functions
   * ---------------------------------------------------------------- */
  
  void nest::ht_neuron::init_state_(const Node& proto)
  {
    const ht_neuron& pr = downcast<ht_neuron>(proto);
    S_ = pr.S_;
  }

  void nest::ht_neuron::init_buffers_()
  {
    // Reset spike buffers.
    for(std::vector<RingBuffer>::iterator it = B_.spike_inputs_.begin();
	it != B_.spike_inputs_.end(); ++it)
      {
	it->clear(); // include resize
      }

    B_.currents_.clear();  // include resize

    B_.logger_.reset();

    Archiving_Node::clear_history();

    B_.step_ = Time::get_resolution().get_ms();
    B_.IntegrationStep_ = B_.step_;

    static const gsl_odeiv_step_type* T1 = gsl_odeiv_step_rkf45;
  
    if ( B_.s_ == 0 )
      B_.s_ = gsl_odeiv_step_alloc (T1, State_::STATE_VEC_SIZE);
    else 
      gsl_odeiv_step_reset(B_.s_);
    
    if ( B_.c_ == 0 )  
      B_.c_ = gsl_odeiv_control_y_new (1e-3, 0.0);
    else
      gsl_odeiv_control_init(B_.c_, 1e-3, 0.0, 1.0, 0.0);
    
    if ( B_.e_ == 0 )  
      B_.e_ = gsl_odeiv_evolve_alloc(State_::STATE_VEC_SIZE);
    else 
      gsl_odeiv_evolve_reset(B_.e_);
  
    B_.sys_.function  = ht_neuron_dynamics; 
    B_.sys_.jacobian  = 0;
    B_.sys_.dimension = State_::STATE_VEC_SIZE;
    B_.sys_.params    = reinterpret_cast<void*>(this);

    B_.I_stim_ = 0.0;
  }

  nest::double_t nest::ht_neuron::get_synapse_constant(nest::double_t Tau_1, 
						      nest::double_t Tau_2,
						      nest::double_t g_peak)
  {
    // Factor used to account for the missing 1/((1/Tau_2)-(1/Tau_1)) term
    // in the ht_neuron_dynamics integration of the synapse terms. 
    // See: Exact digital simulation of time-invariant linear systems
    // with applications to neuronal modeling, Rotter and Diesmann, 
    // section 3.1.2.
    nest::double_t exact_integration_adjustment = (1/Tau_2)-(1/Tau_1);
    
    nest::double_t t_peak = (Tau_2*Tau_1)*std::log(Tau_2/Tau_1)/(Tau_2-Tau_1);
    nest::double_t normalisation_factor = 1/(std::exp(-t_peak/Tau_1) - std::exp(-t_peak/Tau_2));
    
    return g_peak*normalisation_factor*exact_integration_adjustment;
  }

  void nest::ht_neuron::calibrate()
  {
    B_.logger_.init();  // ensures initialization in case mm connected after Simulate

    // NOTE: code below initializes conductance step size for incoming pulses.
    // Variable and function names need to be changed!
    V_.cond_steps_.resize(SUP_SPIKE_RECEPTOR-1);

    V_.cond_steps_[AMPA-1] = 
      get_synapse_constant(P_.AMPA_Tau_1, P_.AMPA_Tau_2, P_.AMPA_g_peak);

    V_.cond_steps_[NMDA-1] = 
      get_synapse_constant(P_.NMDA_Tau_1, P_.NMDA_Tau_2, P_.NMDA_g_peak);

    V_.cond_steps_[GABA_A-1] = 
      get_synapse_constant(P_.GABA_A_Tau_1, P_.GABA_A_Tau_2, P_.GABA_A_g_peak); 

    V_.cond_steps_[GABA_B-1] = 
      get_synapse_constant(P_.GABA_B_Tau_1, P_.GABA_B_Tau_2, P_.GABA_B_g_peak);

    V_.PotassiumRefractoryCounts_ = Time(Time::ms(P_.t_spike)).get_steps();

    assert(V_.PotassiumRefractoryCounts_ >= 0);  // since t_spike_ >= 0, this can only fail in error
  }

  void nest::ht_neuron::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d);
    Archiving_Node::get_status(d);

    DictionaryDatum receptor_type = new Dictionary();

    (*receptor_type)["AMPA"] = AMPA;
    (*receptor_type)["NMDA"] = NMDA;
    (*receptor_type)["GABA_A"] = GABA_A;
    (*receptor_type)["GABA_B"] = GABA_B;

    (*d)["receptor_types"] = receptor_type;
    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  void nest::ht_neuron::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors
    ptmp.set(d);                       // throws if BadProperty
    State_      stmp = S_;  // temporary copy in case of errors
    stmp.set(d, ptmp);                 // throws if BadProperty

    // We now know that (ptmp, stmp) are consistent. We do not 
    // write them back to (P_, S_) before we are also sure that 
    // the properties to be set in the parent class are internally 
    // consistent.
    Archiving_Node::set_status(d);

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;
    S_ = stmp;
  }

  /* ---------------------------------------------------------------- 
   * Update and spike handling functions
   * ---------------------------------------------------------------- */

  void ht_neuron::update(Time const & origin, const long_t from, 
			const long_t to)
  {
    assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
    assert(from < to);
    
    for ( long_t lag = from ; lag < to ; ++lag )
      {
	double tt = 0.0; // it's all relative!
	
	// adaptive step integration
	while ( tt < B_.step_ )
	{
	  const int status = gsl_odeiv_evolve_apply(B_.e_, B_.c_, B_.s_, 
				 &B_.sys_,              // system of ODE
				 &tt,                   // from t...
				  B_.step_,             // ...to t=t+h
				 &B_.IntegrationStep_ , // integration window (written on!)
				  S_.y_);	        // neuron state

	  if ( status != GSL_SUCCESS )
	    throw GSLSolverFailure(get_name(), status);
	}

	// Deactivate potassium current after spike time have expired
	if( S_.r_potassium_ && --S_.r_potassium_ == 0 )
	    S_.g_spike_ = false; // Deactivate potassium current.

	// Add new spikes to node state array
	for(size_t i = 0; i < B_.spike_inputs_.size(); ++i )
	  S_.y_[2+2*i] += V_.cond_steps_[i] * B_.spike_inputs_[i].get_value(lag);

	// A spike is generated when the membrane potential (V) exceeds
	// the threshold (Theta).
	if( !S_.g_spike_ && S_.y_[State_::VM] >= S_.y_[State_::THETA] )
	  {
	    // Set V and Theta to the sodium reversal potential.
	    S_.y_[State_::VM] = P_.E_Na;
	    S_.y_[State_::THETA] = P_.E_Na;

	    // Activate fast potassium current. Drives the 	    
	    // membrane potential towards the potassium reversal
	    // potential (activate only if duration is non-zero).
	    S_.g_spike_ = V_.PotassiumRefractoryCounts_ > 0;
	    S_.r_potassium_ = V_.PotassiumRefractoryCounts_;

	    set_spiketime(Time::step(origin.get_steps()+lag+1));
	    
	    SpikeEvent se;
	    network()->send(*this, se, lag);
	  }

	// set new input current
	B_.I_stim_ = B_.currents_.get_value(lag);

	B_.logger_.record_data(origin.get_steps()+lag);
      }
  }
  
  void nest::ht_neuron::handle(SpikeEvent & e)
  {
    assert(e.get_delay() > 0);
    assert(e.get_rport() < static_cast<int_t>(B_.spike_inputs_.size()));

    B_.spike_inputs_[e.get_rport()].
      add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
		e.get_weight() * e.get_multiplicity() );
  }

  void nest::ht_neuron::handle(CurrentEvent& e)
  {
    assert(e.get_delay() > 0);

    const double_t I=e.get_current();
    const double_t w=e.get_weight();

    // add weighted current; HEP 2002-10-04
    B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), 
			   w * I);
  }

  void nest::ht_neuron::handle(DataLoggingRequest& e)
  {
    B_.logger_.handle(e);
  }
}

#endif //HAVE_GSL
