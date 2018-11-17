#ifndef GLIF_LIF_R_ASC_A_COND_H
#define GLIF_LIF_R_ASC_A_COND_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

#include "dictdatum.h"

/* BeginDocumentation
Name: glif_lif_r_asc_a_cond - Generalized leaky integrate and fire (GLIF) model 5 -
                              Leaky integrate and fire with biologically defined
                              reset rules, after-spike currents and a voltage
                              dependent threshold model.

Description:

  glif_lif_r_asc_a_cond is an implementation of a generalized leaky integrate and fire (GLIF) model 5
  (i.e., leaky integrate and fire with biologically defined reset rules, after-spike currents
  and a voltage dependent threshold model) [1] with conductance-based synapses.
  Incoming spike events induce a post-synaptic change of conductance modeled
  by an alpha function [2]. The alpha function is normalized such that an event of weight 1.0
  results in a peak conductance change of 1 nS at t = tau_syn.  On the postsynapic side,
  there can be arbitrarily many synaptic time constants. This can be reached by
  specifying separate receptor ports, each for a different time constant.
  The port number has to match the respective "receptor_type" in the connectors.

Parameters:

  The following parameters can be set in the status dictionary.

  V_m               double - Membrane potential in mV
  V_th              double - Instantaneous threshold in mV.
  g                 double - Membrane conductance in nS.
  E_L               double - Resting membrane potential in mV.
  C_m               double - Capacitance of the membrane in pF.
  t_ref             double - Duration of refractory time in ms.
  a_spike           double - Threshold addition following spike in mV.
  b_spike           double - Spike-induced threshold time constant in 1/ms.
  a_reset           double - Voltage fraction coefficient following spike.
  b_reset           double - Voltage addition following spike in mV.
  asc_init          double vector - Initial values of after-spike currents in pA.
  k                 double vector - After-spike current time constants in 1/ms (kj in Equation (3) in [1]).
  asc_amps          double vector - After-spike current amplitudes in pA (deltaIj in Equation (7) in [1]).
  r                 double vector - Current fraction following spike coefficients (fj in Equation (7) in [1]).
  a_voltage         double - Adaptation index of threshold - A 'leak-conductance' for the voltage-dependent
                             component of the threshold in 1/ms (av in Equation (4) in [1]).
  b_voltage         double - Voltage-induced threshold time constant - Inverse of which is the time constant
                             of the voltage-dependent component of the threshold in 1/ms (bv in Equation (4) in [1]).
  tau_syn           double vector - Rise time constants of the synaptic alpha function in ms.
  E_rev             double vector - Reversal potential in mV.
  V_dynamics_method string - Voltage dynamics (Equation (1) in [1]) solution methods:
                             'linear_forward_euler' - Linear Euler forward (RK1) to find next V_m value, or
                             'linear_exact' - Linear exact to find next V_m value.

References:
  [1] Teeter C, Iyer R, Menon V, Gouwens N, Feng D, Berg J, Szafer A,
      Cain N, Zeng H, Hawrylycz M, Koch C, & Mihalas S (2018)
      Generalized leaky integrate-and-fire models classify multiple neuron types.
      Nature Communications 9:709.
  [2] Meffin, H., Burkitt, A. N., & Grayden, D. B. (2004). An analytical
      model for the large, fluctuating synaptic conductance state typical of
      neocortical neurons in vivo. J.  Comput. Neurosci., 16, 159-175.

Author: Binghuang Cai and Kael Dai @ Allen Institute for Brain Science
*/

namespace nest
{

extern "C" int glif_lif_r_asc_a_cond_dynamics( double, const double*, double*, void* );

class glif_lif_r_asc_a_cond : public nest::Archiving_Node
{
public:

  glif_lif_r_asc_a_cond();

  glif_lif_r_asc_a_cond( const glif_lif_r_asc_a_cond& );

  ~glif_lif_r_asc_a_cond();

  using nest::Node::handle;
  using nest::Node::handles_test_event;

  nest::port send_test_event( nest::Node&, nest::port, nest::synindex, bool );

  void handle( nest::SpikeEvent& );
  void handle( nest::CurrentEvent& );
  void handle( nest::DataLoggingRequest& );

  nest::port handles_test_event( nest::SpikeEvent&, nest::port );
  nest::port handles_test_event( nest::CurrentEvent&, nest::port );
  nest::port handles_test_event( nest::DataLoggingRequest&, nest::port );

  bool is_off_grid() const  // uses off_grid events
  {
    return true;
  }

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  //! Reset parameters and state of neuron.

  //! Reset state of neuron.
  void init_state_( const Node& proto );

  //! Reset internal buffers of neuron.
  void init_buffers_();

  //! Initialize auxiliary quantities, leave parameters and state untouched.
  void calibrate();

  //! Take neuron through given time interval
  void update( nest::Time const&, const long, const long );

  // make dynamics function quasi-member
  friend int glif_lif_r_asc_a_cond_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class nest::RecordablesMap< glif_lif_r_asc_a_cond >;
  friend class nest::UniversalDataLogger< glif_lif_r_asc_a_cond >;


  struct Parameters_
  {
    double th_inf_; // infinity threshold in mV
    double G_; // membrane conductance in nS
    double E_L_; // resting potential in mV
    double C_m_; // capacitance in pF
    double t_ref_; // refractory time in ms

    double a_spike_; // threshold additive constant following reset in mV
    double b_spike_; //spike induced threshold in 1/ms
    double voltage_reset_a_; //voltage fraction following reset coefficient
    double voltage_reset_b_; // voltage additive constant following reset in mV
    double a_voltage_; // a 'leak-conductance' for the voltage-dependent component of the threshold in 1/ms
    double b_voltage_; // inverse of which is the time constant of the voltage-dependent component of the threshold in 1/ms

    std::vector<double> asc_init_; // initial values of ASCurrents_ in pA
    std::vector<double> k_; // predefined time scale in 1/ms
    std::vector<double> asc_amps_; // in pA
    std::vector<double> r_; // coefficient
    std::vector< double > tau_syn_; // synaptic port time constants in ms
    std::vector< double > E_rev_; // reversal potential in mV
    //std::string V_dynamics_method_; // voltage dynamic methods

    // boolean flag which indicates whether the neuron has connections
    bool has_connections_;

    size_t n_receptors_() const; //!< Returns the size of tau_syn_
    size_t n_ASCurrents_() const; //!< Returns the size of after spike currents

    Parameters_();

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };


  struct State_
  {
    double V_m_; // membrane potential in mV
    double ASCurrents_sum_; // in pA
    double threshold_; // voltage threshold in mV

    //! Symbolic indices to the elements of the state vector y
    enum StateVecElems
    {
      V_M = 0,
      ASC,
      DG_SYN,
      G_SYN,
      STATE_VECTOR_MIN_SIZE
    };

    static const size_t NUMBER_OF_FIXED_STATES_ELEMENTS = 1;        // V_M
    static const size_t NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR = 2; // DG_SYN, G_SYN

    std::vector< double > y_; //!< neuron state

    State_( const Parameters_& );
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum&, const Parameters_&) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };


  struct Buffers_
  {
    Buffers_( glif_lif_r_asc_a_cond& );
    Buffers_( const Buffers_&, glif_lif_r_asc_a_cond& );

    std::vector< nest::RingBuffer > spikes_;   //!< Buffer incoming spikes through delay, as sum
    nest::RingBuffer currents_; //!< Buffer incoming currents through delay,

    //! Logger for all analog data
    nest::UniversalDataLogger< glif_lif_r_asc_a_cond > logger_;

    /* GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
    double step_;            //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;

  };

  struct Variables_
  {
    double t_ref_remaining_; // counter during refractory period, in ms
    double t_ref_total_; // total time of refractory period, in ms

    double last_spike_; // threshold spike component in mV
    double last_voltage_; // threshold voltage component in mV

    /** Amplitude of the synaptic conductance.
        This value is chosen such that an event of weight 1.0 results in a peak conductance of 1 nS
        at t = tau_syn.
    */
    std::vector< double > CondInitialValues_;

    unsigned int receptor_types_size_;

  };

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  // Mapping of recordables names to access functions
  static nest::RecordablesMap< glif_lif_r_asc_a_cond > recordablesMap_;
};


inline size_t
nest::glif_lif_r_asc_a_cond::Parameters_::n_receptors_() const
{
  return tau_syn_.size();
}

inline size_t
nest::glif_lif_r_asc_a_cond::Parameters_::n_ASCurrents_() const
{
  return k_.size();
}

inline nest::port
nest::glif_lif_r_asc_a_cond::send_test_event( nest::Node& target,
  nest::port receptor_type,
  nest::synindex,
  bool )
{
  nest::SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline nest::port
nest::glif_lif_r_asc_a_cond::handles_test_event( nest::CurrentEvent&,
  nest::port receptor_type )
{
  if ( receptor_type != 0 ){
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline nest::port
nest::glif_lif_r_asc_a_cond::handles_test_event( nest::DataLoggingRequest& dlr,
  nest::port receptor_type )
{
  if ( receptor_type != 0 ){
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
glif_lif_r_asc_a_cond::get_status( DictionaryDatum& d ) const
{
  // get our own parameter and state data
  P_.get( d );
  S_.get( d, P_);

  // get information managed by parent class
  Archiving_Node::get_status( d );

  ( *d )[ nest::names::recordables ] = recordablesMap_.get_list();
}

inline void
glif_lif_r_asc_a_cond::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, ptmp );   // throws if BadProperty

  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace nest

#endif // HAVE_GSL
#endif
