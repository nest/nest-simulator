#ifndef GLIF_LIF_ASC_H
#define GLIF_LIF_ASC_H

#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

#include "dictdatum.h"

/* BeginDocumentation
Name: glif_lif_asc - Generalized leaky integrate and fire (GLIF) model 3 -
                     Leaky integrate and fire with after-spike currents model.

Description:

  glif_lif_r_asc_a_cond is an implementation of a generalized leaky integrate and fire (GLIF) model 3
  (i.e., leaky integrate and fire with after-spike currents model), described in [1].

Parameters:

  The following parameters can be set in the status dictionary.

  V_m               double - Membrane potential in mV
  V_th              double - Instantaneous threshold in mV.
  g                 double - Membrane conductance in nS.
  E_L               double - Resting membrane potential in mV.
  C_m               double - Capacitance of the membrane in pF.
  t_ref             double - Duration of refractory time in ms.
  V_reset           double - Reset potential of the membrane in mV.
  asc_init          double vector - Initial values of after-spike currents in pA.
  k                 double vector - After-spike current time constants in 1/ms (kj in Equation (3) in [1]).
  asc_amps          double vector - After-spike current amplitudes in pA (deltaIj in Equation (7) in [1]).
  r                 double vector - Current fraction following spike coefficients (fj in Equation (7) in [1]).
  V_dynamics_method string - Voltage dynamics (Equation (1) in [1]) solution methods:
                             'linear_forward_euler' - Linear Euler forward (RK1) to find next V_m value, or
                             'linear_exact' - Linear exact to find next V_m value.

References:
  [1] Teeter C, Iyer R, Menon V, Gouwens N, Feng D, Berg J, Szafer A,
      Cain N, Zeng H, Hawrylycz M, Koch C, & Mihalas S (2018)
      Generalized leaky integrate-and-fire models classify multiple neuron types.
      Nature Communications 9:709.

Author: Binghuang Cai and Kael Dai @ Allen Institute for Brain Science
*/

namespace nest
{

class glif_lif_asc : public nest::Archiving_Node
{
public:

  glif_lif_asc();

  glif_lif_asc( const glif_lif_asc& );

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

  // The next two classes need to be friends to access the State_ class/member
  friend class nest::RecordablesMap< glif_lif_asc >;
  friend class nest::UniversalDataLogger< glif_lif_asc >;


  struct Parameters_
  {
    double V_th_; // A constant spiking threshold in mV
    double G_; // membrane conductance in nS
    double E_L_; // resting potential in mV
    double C_m_; // capacitance in pF
    double t_ref_; // refractory time in ms
    double V_reset_; // Membrane voltage following spike in mV

    std::vector<double> asc_init_; // initial values of ASCurrents_ in pA
    std::vector<double> k_; // predefined time scale in 1/ms
    std::vector<double> asc_amps_; // in pA
    std::vector<double> r_; // coefficient
    std::string V_dynamics_method_; // voltage dynamic methods

    Parameters_();

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };


  struct State_
  {
    double V_m_;  // membrane potential in mV
    std::vector<double> ASCurrents_; // after-spike currents in pA
    double ASCurrents_sum_; // in sum of after-spike currents in pA

    double I_; // external current in pA

    State_();

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };


  struct Buffers_
  {
    Buffers_( glif_lif_asc& );
    Buffers_( const Buffers_&, glif_lif_asc& );

    nest::RingBuffer spikes_;   //!< Buffer incoming spikes through delay, as sum
    nest::RingBuffer currents_; //!< Buffer incoming currents through delay,

    //! Logger for all analog data
    nest::UniversalDataLogger< glif_lif_asc > logger_;
  };

  struct Variables_
  {
    double t_ref_remaining_; // counter during refractory period in ms
    double t_ref_total_; // total time of refractory period in ms
    int method_; // voltage dynamics solver method flag: 0-linear forward euler; 1-linear exact
  };

  double get_V_m_() const
  {
    return S_.V_m_;
  }

  double get_AScurrents_sum_() const
  {
    return S_.ASCurrents_[0];
  }

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  // Mapping of recordables names to access functions
  static nest::RecordablesMap< glif_lif_asc > recordablesMap_;
};

inline nest::port
nest::glif_lif_asc::send_test_event( nest::Node& target,
  nest::port receptor_type,
  nest::synindex,
  bool )
{
  nest::SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline nest::port
nest::glif_lif_asc::handles_test_event( nest::SpikeEvent&,
  nest::port receptor_type )
{
  if ( receptor_type != 0 ){
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline nest::port
nest::glif_lif_asc::handles_test_event( nest::CurrentEvent&,
  nest::port receptor_type )
{
  if ( receptor_type != 0 ){
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline nest::port
nest::glif_lif_asc::handles_test_event( nest::DataLoggingRequest& dlr,
  nest::port receptor_type )
{
  if ( receptor_type != 0 ){
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
glif_lif_asc::get_status( DictionaryDatum& d ) const
{
  // get our own parameter and state data
  P_.get( d );
  S_.get( d );

  // get information managed by parent class
  Archiving_Node::get_status( d );

  ( *d )[ nest::names::recordables ] = recordablesMap_.get_list();
}

inline void
glif_lif_asc::set_status( const DictionaryDatum& d )
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

#endif
