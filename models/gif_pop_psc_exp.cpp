/*
 *  gif_pop_psc_exp.cpp
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

/* Point process population model with exponential postsynaptic currents and
 * adaptation */

/* [1]: Line numbers in comments refer to the algorithm pseudocode in
        Figures 11 and 12 of the paper
        Schwalger T, Deger M, Gerstner W (2017)
        Towards a theory of cortical columns: From spiking neurons to
        interacting neural populations of finite size.
        PLoS Comput Biol 13(4): e1005507.
        https://doi.org/10.1371/journal.pcbi.1005507 */

#include "gif_pop_psc_exp.h"
#include "universal_data_logger_impl.h"
#include "compose.hpp"

#ifdef HAVE_GSL

namespace nest
{
/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< gif_pop_psc_exp > gif_pop_psc_exp::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< gif_pop_psc_exp >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &gif_pop_psc_exp::get_V_m_ );
  insert_( names::n_events, &gif_pop_psc_exp::get_n_events_ );
  insert_( names::E_sfa, &gif_pop_psc_exp::get_E_sfa_ );
  insert_( names::mean, &gif_pop_psc_exp::get_mean_ );
  insert_( names::I_syn_ex, &gif_pop_psc_exp::get_I_syn_ex_ );
  insert_( names::I_syn_in, &gif_pop_psc_exp::get_I_syn_in_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::gif_pop_psc_exp::Parameters_::Parameters_()
  : N_( 100 )          // 1
  , tau_m_( 20. )      // ms
  , c_m_( 250. )       // pF
  , t_ref_( 4. )       // ms
  , lambda_0_( 10. )   // 1/s
  , Delta_V_( 2. )     // mV
  , len_kernel_( -1 )  // time steps
  , I_e_( 0. )         // pA
  , V_reset_( 0. )     // mV
  , V_T_star_( 15. )   // mV
  , E_L_( 0. )         // mV
  , tau_syn_ex_( 3.0 ) // ms
  , tau_syn_in_( 6.0 ) // ms
  , BinoRand_( true )  // bool
{
  tau_sfa_.clear();
  q_sfa_.clear();
  tau_sfa_.push_back( 300.0 ); // ms
  q_sfa_.push_back( 0.5 );     // mV
}

nest::gif_pop_psc_exp::State_::State_()
  : y0_( 0.0 )
  , I_syn_ex_( 0.0 )
  , I_syn_in_( 0.0 )
  , V_m_( 0.0 )
  , n_expect_( 0.0 )
  , theta_hat_( 0.0 ) // initialization value has no effect for theta_hat_
  , n_spikes_( 0 )
  , initialized_( false )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::gif_pop_psc_exp::Parameters_::get( DictionaryDatum& d ) const
{
  def< long >( d, names::N, N_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::lambda_0, lambda_0_ );
  def< double >( d, names::Delta_V, Delta_V_ );
  def< long >( d, names::len_kernel, len_kernel_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::V_T_star, V_T_star_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::tau_syn_ex, tau_syn_ex_ );
  def< double >( d, names::tau_syn_in, tau_syn_in_ );
  def< bool >( d, "BinoRand", BinoRand_ );

  ArrayDatum tau_sfa_list_ad( tau_sfa_ );
  def< ArrayDatum >( d, names::tau_sfa, tau_sfa_list_ad );

  ArrayDatum q_sfa_list_ad( q_sfa_ );
  def< ArrayDatum >( d, names::q_sfa, q_sfa_list_ad );
}

void
nest::gif_pop_psc_exp::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< long >( d, names::N, N_ );
  updateValue< double >( d, names::tau_m, tau_m_ );
  updateValue< double >( d, names::C_m, c_m_ );
  updateValue< double >( d, names::lambda_0, lambda_0_ );
  updateValue< double >( d, names::Delta_V, Delta_V_ );
  updateValue< long >( d, names::len_kernel, len_kernel_ );
  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::V_reset, V_reset_ );
  updateValue< double >( d, names::V_T_star, V_T_star_ );
  updateValue< double >( d, names::E_L, E_L_ );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::tau_syn_ex, tau_syn_ex_ );
  updateValue< double >( d, names::tau_syn_in, tau_syn_in_ );
  updateValue< bool >( d, "BinoRand", BinoRand_ );

  updateValue< std::vector< double > >( d, names::tau_sfa, tau_sfa_ );
  updateValue< std::vector< double > >( d, names::q_sfa, q_sfa_ );


  if ( tau_sfa_.size() != q_sfa_.size() )
  {
    throw BadProperty( String::compose(
      "'tau_sfa' and 'q_sfa' need to have the same dimension.\nSize of "
      "tau_sfa: %1\nSize of q_sfa: %2",
      tau_sfa_.size(),
      q_sfa_.size() ) );
  }

  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( tau_m_ <= 0 )
  {
    throw BadProperty(
      "The membrane time constants must be strictly positive." );
  }

  if ( tau_syn_ex_ <= 0 or tau_syn_in_ <= 0 )
  {
    throw BadProperty(
      "The synaptic time constants must be strictly positive." );
  }

  for ( size_t i = 0; i < tau_sfa_.size(); ++i )
  {
    if ( tau_sfa_[ i ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }

  if ( N_ <= 0 )
  {
    throw BadProperty( "Number of neurons must be positive." );
  }

  if ( lambda_0_ < 0 )
  {
    throw BadProperty( "lambda_0 must be positive." );
  }

  if ( Delta_V_ <= 0 )
  {
    throw BadProperty( "Delta_V must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Absolute refractory period cannot be negative." );
  }
}

void
nest::gif_pop_psc_exp::State_::get( DictionaryDatum& d,
  const Parameters_& ) const
{
  def< double >( d, names::V_m, V_m_ );         // Filtered version of input
  def< long >( d, names::n_events, n_spikes_ ); // Number of generated spikes
  def< double >( d, names::E_sfa, theta_hat_ ); // Adaptive threshold potential
  def< double >( d, names::mean, n_expect_ );
  def< double >( d, names::I_syn_ex, I_syn_ex_ );
  def< double >( d, names::I_syn_in, I_syn_in_ );
}

void
nest::gif_pop_psc_exp::State_::set( const DictionaryDatum& d,
  const Parameters_& )
{
  updateValue< double >( d, names::V_m, V_m_ );
  updateValue< double >( d, names::I_syn_ex, I_syn_ex_ );
  updateValue< double >( d, names::I_syn_in, I_syn_in_ );
  initialized_ =
    false; // vectors of the state should be initialized with new parameter set.
}

nest::gif_pop_psc_exp::Buffers_::Buffers_( gif_pop_psc_exp& n )
  : logger_( n )
{
}

nest::gif_pop_psc_exp::Buffers_::Buffers_( const Buffers_&, gif_pop_psc_exp& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::gif_pop_psc_exp::gif_pop_psc_exp()
  : Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::gif_pop_psc_exp::gif_pop_psc_exp( const gif_pop_psc_exp& n )
  : Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::gif_pop_psc_exp::init_state_( const Node& proto )
{
  const gif_pop_psc_exp& pr = downcast< gif_pop_psc_exp >( proto );
  S_ = pr.S_;
}

void
nest::gif_pop_psc_exp::init_buffers_()
{
  B_.ex_spikes_.clear(); //!< includes resize
  B_.in_spikes_.clear();
  B_.currents_.clear();
  B_.logger_.reset();
}


void
nest::gif_pop_psc_exp::calibrate()
{
  if ( P_.tau_sfa_.size() == 0 )
  {
    throw BadProperty( "Time constant array should not be empty. " );
  }

  if ( P_.q_sfa_.size() == 0 )
  {
    throw BadProperty( "Adaptation value array should not be empty. " );
  }

  B_.logger_.init();

  V_.h_ = Time::get_resolution().get_ms();
  V_.rng_ = kernel().rng_manager.get_rng( get_thread() );
  V_.min_double_ = std::numeric_limits< double >::min();
  V_.R_ = P_.tau_m_ / P_.c_m_; // membrane resistance

  // membrane integration constants
  V_.P22_ = std::exp( -V_.h_ / P_.tau_m_ );
  V_.P20_ = P_.tau_m_ / P_.c_m_ * ( 1 - V_.P22_ );

  // constants needed for integrating exponential pscs
  V_.P11_ex_ = std::exp( -V_.h_ / P_.tau_syn_ex_ );
  V_.P11_in_ = std::exp( -V_.h_ / P_.tau_syn_in_ );

  // initializing internal state
  if ( not S_.initialized_ )
  {
    // relaxation time of refractory kernel. This sets the length of the spike
    // history buffer n_ and other internal variables.

    if ( P_.len_kernel_ < 1 )
    {
      // a value smaller than one signals that the kernel length is to be
      // automatically chosen based on the remaining parameters.
      P_.len_kernel_ = get_history_size();
    }

    V_.k_ref_ = Time( Time::ms( ( P_.t_ref_ ) ) ).get_steps();

    // initialize state variables
    V_.lambda_free_ = 0.;
    V_.n_.clear();
    V_.m_.clear();
    V_.v_.clear();
    V_.u_.clear();
    V_.lambda_.clear();
    V_.theta_.clear();
    V_.theta_tld_.clear();

    // Procedure InitPopulations, see Fig. 11 of [1]
    for ( int k = 0; k < P_.len_kernel_; ++k )
    {
      V_.n_.push_back( 0 );      // line 3 of [1]
      V_.m_.push_back( 0 );      // line 3 of [1]
      V_.v_.push_back( 0 );      // line 3 of [1]
      V_.u_.push_back( 0 );      // line 3 of [1]
      V_.lambda_.push_back( 0 ); // line 3 of [1]

      const double theta_tmp = adaptation_kernel( P_.len_kernel_ - k );
      V_.theta_.push_back( theta_tmp ); // line 4 of [1]
      V_.theta_tld_.push_back( P_.Delta_V_
        * ( 1. - std::exp( -theta_tmp / P_.Delta_V_ ) )
        / static_cast< double >( P_.N_ ) ); // line 5 of [1]
    }

    V_.n_[ P_.len_kernel_ - 1 ] =
      static_cast< double >( P_.N_ ); // InitPopulations, line 7 of [1]
    V_.m_[ P_.len_kernel_ - 1 ] =
      static_cast< double >( P_.N_ ); // InitPopulations, line 7 of [1]

    // InitPopulations, line 8 of [1]
    V_.x_ = 0.;
    V_.z_ = 0.;
    V_.k0_ = 0; // rotating index has to start at 0

    // lines 9-10 of [1]: Variables y (and h) are initialized with other State
    // variables.

    // initialize adaptation variables
    V_.g_.clear();
    V_.Q30_.clear();
    V_.Q30K_.clear();

    for ( size_t k = 0; k < P_.tau_sfa_.size(); ++k )
    {
      // multiply by tau_sfa here because [1] defines J as product
      // of J and tau_sfa.
      V_.Q30K_.push_back( P_.q_sfa_[ k ] * P_.tau_sfa_[ k ]
        * std::exp( -V_.h_ * P_.len_kernel_ / P_.tau_sfa_[ k ] ) );
      V_.Q30_.push_back( std::exp( -V_.h_ / P_.tau_sfa_[ k ] ) );
      V_.g_.push_back( 0.0 );
    }

    S_.initialized_ = true;
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */


inline double
nest::gif_pop_psc_exp::escrate( const double x )
{
  return P_.lambda_0_ * std::exp( x / P_.Delta_V_ );
}


inline long
nest::gif_pop_psc_exp::draw_poisson( const double n_expect_ )
{
  // Draw Poisson random number of spikes
  // If n_expect_ is too large, the random numbers might get bad. So we use
  // N_ in case of excessive rates.
  long n_t_;
  if ( n_expect_ > P_.N_ )
  {
    n_t_ = P_.N_;
  }
  else if ( n_expect_ > V_.min_double_ )
  {
    // if the probability of any spike at all (1-exp(-lambda)) is
    // indistinguishable from that of one spike (lambda * exp(-lambda)),
    // we draw a Bernoulli random number instead of Poisson.
    if ( 1. - ( n_expect_ + 1. ) * std::exp( -n_expect_ ) > V_.min_double_ )
    {
      V_.poisson_dev_.set_lambda( n_expect_ );
      n_t_ = V_.poisson_dev_.ldev( V_.rng_ );
    }
    else
    {
      n_t_ = static_cast< long >( V_.rng_->drand() < n_expect_ );
    }

    // in case the number of spikes exceeds N, we clip it to prevent
    // runaway activity
    if ( n_t_ > P_.N_ )
    {
      n_t_ = P_.N_;
    }
    // in case the number of spikes is negative, we clip it to
    // prevent problems downstream. This should not happen.
    if ( n_t_ < 0 )
    {
      n_t_ = 0;
    }
  }
  else
  {
    n_t_ = 0;
  }
  return n_t_;
}


inline long
nest::gif_pop_psc_exp::draw_binomial( const double n_expect_ )
{
  double p_bino_ = n_expect_ / P_.N_;
  if ( p_bino_ >= 1. )
  {
    return P_.N_;
  }
  else if ( p_bino_ <= 0. )
  {
    return 0;
  }
  else
  {
    V_.bino_dev_.set_p_n( p_bino_, P_.N_ );
  }
  return V_.bino_dev_.ldev( V_.rng_ );
}


inline double
nest::gif_pop_psc_exp::adaptation_kernel( const int k )
{
  // this function computes the value of the sum of exponentials adaptation
  // kernel at a time lag given by k time steps.
  // See below Eq. (87) of [1]. There is no division by tau here because
  // theta_tmp must be in units voltage just as q_sfa_.
  double theta_tmp = 0.;
  for ( size_t j = 0; j < P_.tau_sfa_.size(); ++j )
  {
    theta_tmp += P_.q_sfa_[ j ] * std::exp( -k * V_.h_ / P_.tau_sfa_[ j ] );
  }
  return theta_tmp;
}


inline int
nest::gif_pop_psc_exp::get_history_size()
{
  // This function automatically determines a suitable history kernel size,
  // see [1], Eq. (86) and Fig 11, Procedure GetHistoryLength.
  double tmax = 20000.; // ms, maximum automatic kernel length

  int k = tmax / V_.h_;
  int kmin = 5 * P_.tau_m_ / V_.h_;
  while ( ( adaptation_kernel( k ) / P_.Delta_V_ < 0.1 ) and ( k > kmin ) )
  {
    k--;
  }
  if ( k * V_.h_ <= P_.t_ref_ )
  {
    k = ( int ) ( P_.t_ref_ / V_.h_ ) + 1;
  }
  return k;
}


void
nest::gif_pop_psc_exp::update( Time const& origin,
  const long from,
  const long to )
{
  assert(
    to >= 0 and ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    // main update routine, see Fig. 11 of [1]
    double h_tot_;
    // this is the membrane and synapse update method of [1]
    h_tot_ = ( P_.I_e_ + S_.y0_ ) * V_.P20_ + P_.E_L_; // line 6 of [1]

    // get the input spikes from the buffers
    // We are getting spike numbers weighted with synaptic weights here,
    // but [1] uses A(t), which implies division by J, N and dt, which we do
    // not know here (e.g. J is stored externally to the model in NEST).
    // However, these rescalings are undone below,
    // so the quantities used here are equivalent.
    double JNA_ex = B_.ex_spikes_.get_value( lag ) / V_.h_;
    double JNA_in = B_.in_spikes_.get_value( lag ) / V_.h_;

    // rescale inputs to voltage scale used in [1]
    JNA_ex *= P_.tau_syn_ex_ / P_.c_m_;
    JNA_in *= P_.tau_syn_in_ / P_.c_m_;

    // translate synaptic currents into [1]'s definition
    double JNy_ex = S_.I_syn_ex_ / P_.c_m_;
    double JNy_in = S_.I_syn_in_ / P_.c_m_;

    // membrane update (line 10 of [1])
    const double h_ex_tmpvar =
      ( P_.tau_syn_ex_ * V_.P11_ex_ * ( JNy_ex - JNA_ex )
        - V_.P22_ * ( P_.tau_syn_ex_ * JNy_ex - P_.tau_m_ * JNA_ex ) );
    const double h_in_tmpvar =
      ( P_.tau_syn_in_ * V_.P11_in_ * ( JNy_in - JNA_in )
        - V_.P22_ * ( P_.tau_syn_in_ * JNy_in - P_.tau_m_ * JNA_in ) );
    const double h_ex =
      P_.tau_m_ * ( JNA_ex + h_ex_tmpvar / ( P_.tau_syn_ex_ - P_.tau_m_ ) );
    const double h_in =
      P_.tau_m_ * ( JNA_in + h_in_tmpvar / ( P_.tau_syn_in_ - P_.tau_m_ ) );
    h_tot_ += h_ex + h_in;

    // update EPSCs & IPSCs (line 11 of [1])
    JNy_ex = JNA_ex + ( JNy_ex - JNA_ex ) * V_.P11_ex_;
    JNy_in = JNA_in + ( JNy_in - JNA_in ) * V_.P11_in_;

    // store the updated currents, translated back
    S_.I_syn_ex_ = JNy_ex * P_.c_m_;
    S_.I_syn_in_ = JNy_in * P_.c_m_;

    // Set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // begin procedure update population, see Fig. 12 of [1]
    double W_ = 0, X_ = 0, Y_ = 0, Z_ = 0; // line 2 of [1]
    S_.theta_hat_ = P_.V_T_star_;          // line 2, initialize theta

    S_.V_m_ = ( S_.V_m_ - P_.E_L_ ) * V_.P22_ + h_tot_; // line 3 of [1]

    // compute free adaptation state
    for ( size_t j = 0; j < P_.tau_sfa_.size(); ++j ) // line 4 of [1]
    {
      const double g_j_tmp = ( 1. - V_.Q30_[ j ] ) * V_.n_[ V_.k0_ ]
        / ( static_cast< double >( P_.N_ ) * V_.h_ );
      V_.g_[ j ] = V_.g_[ j ] * V_.Q30_[ j ] + g_j_tmp; // line 5 of [1]
      S_.theta_hat_ += V_.Q30K_[ j ] * V_.g_[ j ];      // line 6 of [1]
    }

    // compute free escape rate
    double lambda_tld = escrate( S_.V_m_ - S_.theta_hat_ ); // line 8 of [1]
    const double P_free =
      1 - std::exp( -0.0005 * ( V_.lambda_free_ + lambda_tld )
            * V_.h_ );                                // line 9 of [1]
    V_.lambda_free_ = lambda_tld;                     // line 10
    S_.theta_hat_ -= V_.n_[ 0 ] * V_.theta_tld_[ 0 ]; // line 11

    for ( int k_marked = 0; k_marked < P_.len_kernel_; ++k_marked )
    {
      X_ += V_.m_[ k_marked ]; // line 12 of [1]
    }

    // use a local theta_hat to reserve S_.theta_hat_ for the free threshold,
    // which is a recordable
    double theta_hat_ = S_.theta_hat_;

    // line 13 of [1]
    for ( int k_marked = 0; k_marked < P_.len_kernel_ - V_.k_ref_; ++k_marked )
    {
      int k = ( V_.k0_ + k_marked ) % P_.len_kernel_;          // line 14 of [1]
      const double theta = V_.theta_[ k_marked ] + theta_hat_; // line 15
      theta_hat_ += V_.n_[ k ] * V_.theta_tld_[ k_marked ];    // line 16
      V_.u_[ k ] = ( V_.u_[ k ] - P_.E_L_ ) * V_.P22_ + h_tot_; // line 17
      lambda_tld = escrate( V_.u_[ k ] - theta );               // line 18
      double P_lambda_ = 0.0005 * ( lambda_tld + V_.lambda_[ k ] ) * V_.h_;
      if ( P_lambda_ > 0.01 )
      {
        P_lambda_ = 1. - std::exp( -P_lambda_ ); // line 20 of [1]
      }
      V_.lambda_[ k ] = lambda_tld; // line 21 of [1]
      Y_ += P_lambda_ * V_.v_[ k ]; // line 22
      Z_ += V_.v_[ k ];             // line 23
      W_ += P_lambda_ * V_.m_[ k ]; // line 24

      const double ompl = ( 1. - P_lambda_ );
      V_.v_[ k ] = ompl * ompl * V_.v_[ k ] + P_lambda_ * V_.m_[ k ];
      V_.m_[ k ] = ompl * V_.m_[ k ]; // line 26 of [1]
    }                                 // line 27 of [1]

    double P_Lambda_;
    if ( ( Z_ + V_.z_ ) > 0.0 )
    {
      P_Lambda_ = ( Y_ + P_free * V_.z_ ) / ( Z_ + V_.z_ ); // line 28 of [1]
    }
    else
    {
      P_Lambda_ = 0.0;
    }

    // finally compute expected number of spikes and draw a random number
    S_.n_expect_ =
      W_ + P_free * V_.x_ + P_Lambda_ * ( P_.N_ - X_ - V_.x_ ); // line 29
    if ( P_.BinoRand_ )
    {
      S_.n_spikes_ = draw_binomial( S_.n_expect_ );
    }
    else
    {
      S_.n_spikes_ = draw_poisson( S_.n_expect_ );
    }

    // line 31 of [1]: update z
    const double ompf = ( 1 - P_free );
    V_.z_ = ompf * ompf * V_.z_ + V_.x_ * P_free + V_.v_[ V_.k0_ ];
    // line 32 of [1]: update x
    V_.x_ = V_.x_ * ompf + V_.m_[ V_.k0_ ];

    V_.n_[ V_.k0_ ] = S_.n_spikes_; // line 33 of [1]
    V_.m_[ V_.k0_ ] = S_.n_spikes_; // line 33
    V_.v_[ V_.k0_ ] = 0;            // line 34
    V_.u_[ V_.k0_ ] = P_.V_reset_;  // line 35
    V_.lambda_[ V_.k0_ ] = 0.;      // line 36

    // end procedure update population

    // back in Fig 11 of [1], main update procedure

    // shift rotating index
    V_.k0_ = ( V_.k0_ + 1 ) % P_.len_kernel_; // line 17 of [1]

    // end of main update routine, Fig. 11

    // Voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );

    // test if S_.n_spikes_>0, generate spike and send
    // this number as the multiplicity parameter
    if ( S_.n_spikes_ > 0 ) // Are there any spikes?
    {
      SpikeEvent* se;
      se = new SpikeEvent;
      se->set_multiplicity( S_.n_spikes_ );
      kernel().event_delivery_manager.send( *this, *se, lag );
    }
  }
}

void
gif_pop_psc_exp::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double s = e.get_weight() * e.get_multiplicity();

  if ( s > 0.0 )
  {
    B_.ex_spikes_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      s );
  }
  else
  {
    B_.in_spikes_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      s );
  }
}

void
nest::gif_pop_psc_exp::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // Add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::gif_pop_psc_exp::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace

#endif /* HAVE_GSL */
