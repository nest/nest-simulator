/*
 *  cm_compartmentcurrents.h
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
#ifndef CM_COMPARTMENTCURRENTS_H
#define CM_COMPARTMENTCURRENTS_H

#include <stdlib.h>

#include "logging.h"
#include "logging_manager.h"
#include "ring_buffer.h"

namespace nest
{


/**
 * Channel taken from the following .mod file:
 * https://senselab.med.yale.edu/ModelDB/ShowModel?model=140828&file=/Branco_2010/mod.files/na.mod#tabs-2
 *
 * Info in .mod file:
 * > Sodium channel, Hodgkin-Huxley style kinetics.
 * >
 * > Kinetics were fit to data from Huguenard et al. (1988) and Hamill et
 * > al. (1991)
 * >
 * > ...
 * >
 * > Author: Zach Mainen, Salk Institute, 1994, zach@salk.edu
 */
class Na
{
private:
  //! state variables sodium channel
  double m_Na_;
  double h_Na_;

  //! user-defined parameters sodium channel (maximal conductance, reversal potential)
  double gbar_Na_; // uS
  double e_Na_;    // mV

  //! temperature factor for reaction rates
  double q10_;

public:
  Na( double v_comp );
  explicit Na( double v_comp, const DictionaryDatum& channel_params );
  ~Na() {};

  void init_statevars( double v_init );
  void pre_run_hook() {};

  //! make the state variables of this channel accessible
  void append_recordables( std::map< Name, double* >* recordables, const long compartment_idx );

  //! compute state variable time scale and asymptotic values
  std::pair< double, double > compute_statevar_m( double v_comp );
  std::pair< double, double > compute_statevar_h( double v_comp );

  //! advance channel by one numerical integration step
  std::pair< double, double > f_numstep( const double v_comp );
};


/**
 * Channel taken from the following .mod file:
 * https://senselab.med.yale.edu/ModelDB/ShowModel?model=140828&file=/Branco_2010/mod.files/kv.mod#tabs-2
 *
 * Info in .mod file:
 * > Potassium channel, Hodgkin-Huxley style kinetics
 * > Kinetic rates based roughly on Sah et al. and Hamill et al. (1991)
 * >
 * > Author: Zach Mainen, Salk Institute, 1995, zach@salk.edu
 */
class K
{
private:
  //! state variables potassium channel
  double n_K_ = 0.0;

  //! user-defined parameters potassium channel (maximal conductance, reversal potential)
  double gbar_K_; // uS
  double e_K_;    // mV

  //! temperature factor for reaction rates
  double q10_;

public:
  K( double v_comp );
  explicit K( double v_comp, const DictionaryDatum& channel_params );
  ~K() {};

  void init_statevars( double v_init );
  void pre_run_hook() {};

  //! make the state variables of this channel accessible
  void append_recordables( std::map< Name, double* >* recordables, const long compartment_idx );

  //! compute state variable time scale and asymptotic values
  std::pair< double, double > compute_statevar_n( double v_comp );

  //! advance channel by one numerical integration step
  std::pair< double, double > f_numstep( const double v_comp );
};


class AMPA
{
private:
  //!  global synapse index
  long syn_idx = 0;

  //!  state variables
  double g_r_AMPA_;
  double g_d_AMPA_;

  //!  user defined parameters
  double e_rev_; // mV
  double tau_r_; // ms
  double tau_d_; // ms

  //!  assigned variables
  double g_norm_;

  //!  propagators
  double prop_r_;
  double prop_d_;

  //!  spike buffer
  RingBuffer* b_spikes_;

public:
  explicit AMPA( const long syn_index );
  AMPA( const long syn_index, const DictionaryDatum& receptor_params );
  ~AMPA() {};

  long
  get_syn_idx()
  {
    return syn_idx;
  };

  //! initialization of the state variables
  void
  pre_run_hook()
  {
    const double dt = Time::get_resolution().get_ms();
    // construct propagators
    prop_r_ = std::exp( -dt / tau_r_ );
    prop_d_ = std::exp( -dt / tau_d_ );

    b_spikes_->clear();
  };

  //! make the state variables of this receptor accessible
  void append_recordables( std::map< Name, double* >* recordables );

  //! associate the receptor with a spike buffer
  void
  set_buffer_ptr( std::vector< RingBuffer >& syn_buffers )
  {
    b_spikes_ = &syn_buffers[ syn_idx ];
  };

  //! advance receptor by one numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const long lag );
};


class GABA
{
private:
  //! global synapse index
  long syn_idx = 0;

  //! state variables
  double g_r_GABA_;
  double g_d_GABA_;

  //! user defined parameters
  double e_rev_; // mV
  double tau_r_; // ms
  double tau_d_; // ms

  //! assigned variables
  double g_norm_;

  //! propagators
  double prop_r_;
  double prop_d_;

  //! spike buffer
  RingBuffer* b_spikes_;

public:
  explicit GABA( const long syn_index );
  GABA( const long syn_index, const DictionaryDatum& receptor_params );
  ~GABA() {};

  long
  get_syn_idx()
  {
    return syn_idx;
  };

  //! initialization of the state variables
  void
  pre_run_hook()
  {
    const double dt = Time::get_resolution().get_ms();
    // construct propagators
    prop_r_ = std::exp( -dt / tau_r_ );
    prop_d_ = std::exp( -dt / tau_d_ );

    b_spikes_->clear();
  };

  //! make the state variables of this receptor accessible
  void append_recordables( std::map< Name, double* >* recordables );

  //! associate the receptor with a spike buffer
  void
  set_buffer_ptr( std::vector< RingBuffer >& syn_buffers )
  {
    b_spikes_ = &syn_buffers[ syn_idx ];
  };

  //! advance receptor by one numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const long lag );
};


class NMDA
{
private:
  //! global synapse index
  long syn_idx = 0;

  //! state variables
  double g_r_NMDA_;
  double g_d_NMDA_;

  //! user defined parameters
  double e_rev_; // mV
  double tau_r_; // ms
  double tau_d_; // ms

  //! assigned variables
  double g_norm_;

  //! propagators
  double prop_r_;
  double prop_d_;

  //! spike buffer
  RingBuffer* b_spikes_;

public:
  explicit NMDA( const long syn_index );
  NMDA( const long syn_index, const DictionaryDatum& receptor_params );
  ~NMDA() {};

  long
  get_syn_idx()
  {
    return syn_idx;
  };

  //! initialization of the state variables
  void
  pre_run_hook()
  {
    const double dt = Time::get_resolution().get_ms();
    // construct propagators
    prop_r_ = std::exp( -dt / tau_r_ );
    prop_d_ = std::exp( -dt / tau_d_ );

    b_spikes_->clear();
  };

  //! make the state variables of this receptor accessible
  void append_recordables( std::map< Name, double* >* recordables );

  //! associate the receptor with a spike buffer
  void
  set_buffer_ptr( std::vector< RingBuffer >& syn_buffers )
  {
    b_spikes_ = &syn_buffers[ syn_idx ];
  };

  //! advance receptor by one numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const long lag );

  //! synapse specific function (NMDA nonlinearity)
  inline std::pair< double, double >
  NMDA_sigmoid__and__d_NMDAsigmoid_dv( double v_comp )
  {
    double exp__v_comp = std::exp( -.1 * v_comp );
    double denom = 1. + 0.3 * exp__v_comp;

    return std::make_pair( 1. / denom, 0.03 * exp__v_comp / std::pow( denom, 2 ) );
  }
};


class AMPA_NMDA
{
private:
  //! global synapse index
  long syn_idx = 0;

  //! state variables
  double g_r_AN_AMPA_;
  double g_d_AN_AMPA_;
  double g_r_AN_NMDA_;
  double g_d_AN_NMDA_;

  //! user defined parameters
  double e_rev_;      // mV
  double tau_r_AMPA_; // ms
  double tau_d_AMPA_; // ms
  double tau_r_NMDA_; // ms
  double tau_d_NMDA_; // ms
  double NMDA_ratio_;

  //! assigned variables
  double g_norm_AMPA_;
  double g_norm_NMDA_;

  //! propagators
  double prop_r_AMPA_;
  double prop_d_AMPA_;
  double prop_r_NMDA_;
  double prop_d_NMDA_;

  //! spike buffer
  RingBuffer* b_spikes_;

public:
  // constructor, destructor
  explicit AMPA_NMDA( const long syn_index );
  AMPA_NMDA( const long syn_index, const DictionaryDatum& receptor_params );
  ~AMPA_NMDA() {};

  long
  get_syn_idx()
  {
    return syn_idx;
  };

  //! initialization of the state variables
  void
  pre_run_hook()
  {
    const double dt = Time::get_resolution().get_ms();
    prop_r_AMPA_ = std::exp( -dt / tau_r_AMPA_ );
    prop_d_AMPA_ = std::exp( -dt / tau_d_AMPA_ );
    prop_r_NMDA_ = std::exp( -dt / tau_r_NMDA_ );
    prop_d_NMDA_ = std::exp( -dt / tau_d_NMDA_ );

    b_spikes_->clear();
  };

  //! make the state variables of this receptor accessible
  void append_recordables( std::map< Name, double* >* recordables );

  //! associate the receptor with a spike buffer
  void
  set_buffer_ptr( std::vector< RingBuffer >& syn_buffers )
  {
    b_spikes_ = &syn_buffers[ syn_idx ];
  };

  //! advance receptor by one numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const long lag );

  //! synapse specific function (NMDA nonlinearity)
  inline std::pair< double, double >
  NMDA_sigmoid__and__d_NMDAsigmoid_dv( double v_comp )
  {
    double exp__v_comp = std::exp( -.1 * v_comp );
    double denom = 1. + 0.3 * exp__v_comp;

    return std::make_pair( 1. / denom, 0.03 * exp__v_comp / std::pow( denom, 2 ) );
  }
};


class CompartmentCurrents
{
private:
  // ion channels
  Na Na_chan_;
  K K_chan_;
  // synapses
  std::vector< AMPA > AMPA_syns_;
  std::vector< GABA > GABA_syns_;
  std::vector< NMDA > NMDA_syns_;
  std::vector< AMPA_NMDA > AMPA_NMDA_syns_;

public:
  CompartmentCurrents( double v_comp );
  CompartmentCurrents( double v_comp, const DictionaryDatum& channel_params );
  ~CompartmentCurrents() {};

  void
  pre_run_hook()
  {
    // calibrate ion channels
    Na_chan_.pre_run_hook();
    K_chan_.pre_run_hook();

    // calibrate AMPA synapses
    for ( auto syn_it = AMPA_syns_.begin(); syn_it != AMPA_syns_.end(); ++syn_it )
    {
      syn_it->pre_run_hook();
    }
    // calibrate GABA synapses
    for ( auto syn_it = GABA_syns_.begin(); syn_it != GABA_syns_.end(); ++syn_it )
    {
      syn_it->pre_run_hook();
    }
    // calibrate NMDA synapses
    for ( auto syn_it = NMDA_syns_.begin(); syn_it != NMDA_syns_.end(); ++syn_it )
    {
      syn_it->pre_run_hook();
    }
    // calibrateialization of AMPA_NMDA synapses
    for ( auto syn_it = AMPA_NMDA_syns_.begin(); syn_it != AMPA_NMDA_syns_.end(); ++syn_it )
    {
      syn_it->pre_run_hook();
    }
  }

  void
  add_synapse( const std::string& type, const long syn_idx )
  {
    if ( type == "AMPA" )
    {
      AMPA syn( syn_idx );
      AMPA_syns_.push_back( syn );
    }
    else if ( type == "GABA" )
    {
      GABA syn( syn_idx );
      GABA_syns_.push_back( syn );
    }
    else if ( type == "NMDA" )
    {
      NMDA syn( syn_idx );
      NMDA_syns_.push_back( syn );
    }
    else if ( type == "AMPA_NMDA" )
    {
      AMPA_NMDA syn( syn_idx );
      AMPA_NMDA_syns_.push_back( syn );
    }
    else
    {
      assert( false );
    }
  };
  void
  add_synapse( const std::string& type, const long syn_idx, const DictionaryDatum& receptor_params )
  {
    receptor_params->clear_access_flags();

    if ( type == "AMPA" )
    {
      AMPA syn( syn_idx, receptor_params );
      AMPA_syns_.push_back( syn );
    }
    else if ( type == "GABA" )
    {
      GABA syn( syn_idx, receptor_params );
      GABA_syns_.push_back( syn );
    }
    else if ( type == "NMDA" )
    {
      NMDA syn( syn_idx, receptor_params );
      NMDA_syns_.push_back( syn );
    }
    else if ( type == "AMPA_NMDA" )
    {
      AMPA_NMDA syn( syn_idx, receptor_params );
      AMPA_NMDA_syns_.push_back( syn );
    }
    else
    {
      assert( false );
    }

    ALL_ENTRIES_ACCESSED( *receptor_params, "receptor_params", "Unread dictionary entries: " );
  };

  void
  add_receptor_info( ArrayDatum& ad, const long compartment_index )
  {
    // receptor info for AMPA synapses
    for ( auto syn_it = AMPA_syns_.begin(); syn_it != AMPA_syns_.end(); ++syn_it )
    {
      DictionaryDatum dd = DictionaryDatum( new Dictionary );
      def< long >( dd, names::receptor_idx, syn_it->get_syn_idx() );
      def< long >( dd, names::comp_idx, compartment_index );
      def< std::string >( dd, names::receptor_type, "AMPA" );
      ad.push_back( dd );
    }
    // receptor info for GABA synapses
    for ( auto syn_it = GABA_syns_.begin(); syn_it != GABA_syns_.end(); ++syn_it )
    {
      DictionaryDatum dd = DictionaryDatum( new Dictionary );
      def< long >( dd, names::receptor_idx, syn_it->get_syn_idx() );
      def< long >( dd, names::comp_idx, compartment_index );
      def< std::string >( dd, names::receptor_type, "GABA" );
      ad.push_back( dd );
    }
    // receptor info for NMDA synapses
    for ( auto syn_it = NMDA_syns_.begin(); syn_it != NMDA_syns_.end(); ++syn_it )
    {
      DictionaryDatum dd = DictionaryDatum( new Dictionary );
      def< long >( dd, names::receptor_idx, syn_it->get_syn_idx() );
      def< long >( dd, names::comp_idx, compartment_index );
      def< std::string >( dd, names::receptor_type, "NMDA" );
      ad.push_back( dd );
    }
    // receptor info for AMPA_NMDA synapses
    for ( auto syn_it = AMPA_NMDA_syns_.begin(); syn_it != AMPA_NMDA_syns_.end(); ++syn_it )
    {
      DictionaryDatum dd = DictionaryDatum( new Dictionary );
      def< long >( dd, names::receptor_idx, syn_it->get_syn_idx() );
      def< long >( dd, names::comp_idx, compartment_index );
      def< std::string >( dd, names::receptor_type, "AMPA_NMDA" );
      ad.push_back( dd );
    }
  };

  void
  set_syn_buffers( std::vector< RingBuffer >& syn_buffers )
  {
    // syn_buffers for AMPA synapses
    for ( auto syn_it = AMPA_syns_.begin(); syn_it != AMPA_syns_.end(); ++syn_it )
    {
      syn_it->set_buffer_ptr( syn_buffers );
    }
    // syn_buffers for GABA synapses
    for ( auto syn_it = GABA_syns_.begin(); syn_it != GABA_syns_.end(); ++syn_it )
    {
      syn_it->set_buffer_ptr( syn_buffers );
    }
    // syn_buffers for NMDA synapses
    for ( auto syn_it = NMDA_syns_.begin(); syn_it != NMDA_syns_.end(); ++syn_it )
    {
      syn_it->set_buffer_ptr( syn_buffers );
    }
    // syn_buffers for AMPA_NMDA synapses
    for ( auto syn_it = AMPA_NMDA_syns_.begin(); syn_it != AMPA_NMDA_syns_.end(); ++syn_it )
    {
      syn_it->set_buffer_ptr( syn_buffers );
    }
  }

  std::map< Name, double* >
  get_recordables( const long compartment_idx )
  {

    std::map< Name, double* > recordables;

    // recordables sodium channel
    Na_chan_.append_recordables( &recordables, compartment_idx );
    // recordables potassium channel
    K_chan_.append_recordables( &recordables, compartment_idx );

    // recordables AMPA synapses
    for ( auto syn_it = AMPA_syns_.begin(); syn_it != AMPA_syns_.end(); syn_it++ )
    {
      syn_it->append_recordables( &recordables );
    }
    // recordables GABA synapses
    for ( auto syn_it = GABA_syns_.begin(); syn_it != GABA_syns_.end(); syn_it++ )
    {
      syn_it->append_recordables( &recordables );
    }
    // recordables NMDA synapses
    for ( auto syn_it = NMDA_syns_.begin(); syn_it != NMDA_syns_.end(); syn_it++ )
    {
      syn_it->append_recordables( &recordables );
    }
    // recordables AMPA_NMDA synapses
    for ( auto syn_it = AMPA_NMDA_syns_.begin(); syn_it != AMPA_NMDA_syns_.end(); syn_it++ )
    {
      syn_it->append_recordables( &recordables );
    }

    return recordables;
  };

  std::pair< double, double >
  f_numstep( const double v_comp, const long lag )
  {
    std::pair< double, double > gi( 0., 0. );
    double g_val = 0.;
    double i_val = 0.;

    // contribution of Na channel
    gi = Na_chan_.f_numstep( v_comp );

    g_val += gi.first;
    i_val += gi.second;

    // contribution of K channel
    gi = K_chan_.f_numstep( v_comp );

    g_val += gi.first;
    i_val += gi.second;

    // contribution of AMPA synapses
    for ( auto syn_it = AMPA_syns_.begin(); syn_it != AMPA_syns_.end(); ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, lag );

      g_val += gi.first;
      i_val += gi.second;
    }
    // contribution of GABA synapses
    for ( auto syn_it = GABA_syns_.begin(); syn_it != GABA_syns_.end(); ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, lag );

      g_val += gi.first;
      i_val += gi.second;
    }
    // contribution of NMDA synapses
    for ( auto syn_it = NMDA_syns_.begin(); syn_it != NMDA_syns_.end(); ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, lag );

      g_val += gi.first;
      i_val += gi.second;
    }
    // contribution of AMPA_NMDA synapses
    for ( auto syn_it = AMPA_NMDA_syns_.begin(); syn_it != AMPA_NMDA_syns_.end(); ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, lag );

      g_val += gi.first;
      i_val += gi.second;
    }

    return std::make_pair( g_val, i_val );
  };
};

} // namespace

#endif /* #ifndef CM_COMPARTMENTCURRENTS_H */
