#ifndef SYNAPSES_NEAT_H
#define SYNAPSES_NEAT_H

#include <stdlib.h>

#include "ring_buffer.h"

namespace nest
{

class AMPA{
private:
  // user defined parameters
  double e_rev_ = 0.0; // mV
  double tau_r_ = 0.2, tau_d_ = 3.; // ms

  // assigned variables
  double g_norm_ = 1.0;

  // state variables
  double g_r_ = 0., g_d_ = 0.;

  // spike buffer
  std::shared_ptr< RingBuffer >  b_spikes_;

public:
  // constructor, destructor
  AMPA(std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params);
  ~AMPA(){};

  // initialization of the state variables
  void init()
  {
    g_r_ = 0.; g_d_ = 0.;
    b_spikes_->clear();
  };

  // numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const double dt, const long lag );
};


class GABA{
private:
  // user defined parameters
  double e_rev_ = 0.0; // mV
  double tau_r_ = 0.2, tau_d_ = 10.; // ms

  // assigned variables
  double g_norm_ = 1.0;

  // state variables
  double g_r_ = 0., g_d_ = 0.;

  // spike buffer
  std::shared_ptr< RingBuffer > b_spikes_;

public:
  // constructor, destructor
  GABA(std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params);
  ~GABA(){};

  // initialization of the state variables
  void init()
  {
    g_r_ = 0.; g_d_ = 0.;
    b_spikes_->clear();
  };

  // numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const double dt, const long lag );
};


class NMDA{
private:
  // user defined parameters
  double e_rev_ = 0.0; // mV
  double tau_r_ = 0.2, tau_d_ = 43.; // ms

  // assigned variables
  double g_norm_ = 1.0;

  // state variables
  double g_r_ = 0., g_d_ = 0.;

  // spike buffer
  std::shared_ptr< RingBuffer >  b_spikes_;

public:
  // constructor, destructor
  NMDA(std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params);
  ~NMDA(){};

  // initialization of the state variables
  void init(){
    g_r_ = 0.; g_d_ = 0.;
    b_spikes_->clear();
  };

  // numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const double dt, const long lag );

  // synapse specific funtions
  inline double NMDAsigmoid( double v_comp )
  {
    return 1. / ( 1. + 0.3 * std::exp( -.1 * v_comp ) );
  };
  inline double d_NMDAsigmoid_dv( double v_comp )
  {
    return 0.03 * std::exp( -0.1 * v_comp ) / std::pow( 0.3 * std::exp( -0.1*v_comp ) + 1.0, 2 );
  };
};


class AMPA_NMDA{
private:
  // user defined parameters
  double e_rev_ = 0.0; // mV
  double tau_r_AMPA_ = 0.2, tau_d_AMPA_ = 43.; // ms
  double tau_r_NMDA_ = 0.2, tau_d_NMDA_ = 43.; // ms
  double NMDA_ratio_ = 2.0;

  // assigned variables
  double g_norm_AMPA_ = 1.0;
  double g_norm_NMDA_ = 1.0;

  // state variables
  double g_r_AMPA_ = 0., g_d_AMPA_ = 0.;
  double g_r_NMDA_ = 0., g_d_NMDA_ = 0.;

  // spike buffer
  std::shared_ptr< RingBuffer >  b_spikes_;

public:
  // constructor, destructor
  AMPA_NMDA(std::shared_ptr< RingBuffer >  b_spikes, const DictionaryDatum& receptor_params);
  ~AMPA_NMDA(){};

  // initialization of the state variables
  void init()
  {
    g_r_AMPA_ = 0.; g_d_AMPA_ = 0.;
    g_r_NMDA_ = 0.; g_d_NMDA_ = 0.;
    b_spikes_->clear();
  };

  // numerical integration step
  std::pair< double, double > f_numstep( const double v_comp, const double dt, const long lag );

  // synapse specific funtions
  inline double NMDAsigmoid( double v_comp )
  {
    return 1. / ( 1. + 0.3 * std::exp( -.1 * v_comp ) );
  };
  inline double d_NMDAsigmoid_dv( double v_comp )
  {
    return 0.03 * std::exp( -0.1 * v_comp ) / std::pow( 0.3 * std::exp( -0.1*v_comp ) + 1.0, 2 );
  };
};


class CompartmentCurrents {
private:
  // synapses
  std::vector < AMPA > AMPA_syns_;
  std::vector < GABA > GABA_syns_;
  std::vector < NMDA > NMDA_syns_;
  std::vector < AMPA_NMDA > AMPA_NMDA_syns_;

public:
  void init(){
    // initialization of AMPA synapses
    for( auto syn_it = AMPA_syns_.begin();
         syn_it != AMPA_syns_.end();
         ++syn_it )
    {
      syn_it->init();

    }
    // initialization of GABA synapses
    for( auto syn_it = GABA_syns_.begin();
         syn_it != GABA_syns_.end();
         ++syn_it )
    {
      syn_it->init();
    }
    // initialization of NMDA synapses
    for( auto syn_it = NMDA_syns_.begin();
         syn_it != NMDA_syns_.end();
         ++syn_it )
    {
      syn_it->init();
    }
    // initialization of AMPA_NMDA synapses
    for( auto syn_it = AMPA_NMDA_syns_.begin();
         syn_it != AMPA_NMDA_syns_.end();
         ++syn_it )
    {
      syn_it->init();
    }
  }

  void add_synapse_with_buffer( const std::string& type, std::shared_ptr< RingBuffer > b_spikes, const DictionaryDatum& receptor_params )
  {
    if ( type == "AMPA" )
    {
      AMPA syn( b_spikes, receptor_params );
      AMPA_syns_.push_back( syn );
    }
    else if ( type == "GABA" )
    {
      GABA syn( b_spikes, receptor_params );
      GABA_syns_.push_back( syn );
    }
    else if ( type == "NMDA" )
    {
      NMDA syn( b_spikes, receptor_params );
      NMDA_syns_.push_back( syn );
    }
    else if ( type == "AMPA_NMDA" )
    {
      AMPA_NMDA syn( b_spikes, receptor_params );
      AMPA_NMDA_syns_.push_back( syn );
    }
    else
    {
      assert( false );
    }
  };

  std::pair< double, double > f_numstep( const double v_comp, const double dt, const long lag )
  {
    std::pair< double, double > gi(0., 0.);
    double g_val = 0.;
    double i_val = 0.;

    // contribution of AMPA synapses
    for( auto syn_it = AMPA_syns_.begin();
         syn_it != AMPA_syns_.end();
         ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, dt, lag );

      g_val += gi.first;
      i_val += gi.second;
    }
    // contribution of GABA synapses
    for( auto syn_it = GABA_syns_.begin();
         syn_it != GABA_syns_.end();
         ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, dt, lag );

      g_val += gi.first;
      i_val += gi.second;
    }
    // contribution of NMDA synapses
    for( auto syn_it = NMDA_syns_.begin();
         syn_it != NMDA_syns_.end();
         ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, dt, lag );

      g_val += gi.first;
      i_val += gi.second;
    }
    // contribution of AMPA_NMDA synapses
    for( auto syn_it = AMPA_NMDA_syns_.begin();
         syn_it != AMPA_NMDA_syns_.end();
         ++syn_it )
    {
      gi = syn_it->f_numstep( v_comp, dt, lag );

      g_val += gi.first;
      i_val += gi.second;
    }

    return std::make_pair(g_val, i_val);
  };
};

} // namespace

#endif /* #ifndef SYNAPSES_NEAT_H */
