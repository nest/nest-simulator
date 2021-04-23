#ifndef SYNAPSES_NEAT_H
#define SYNAPSES_NEAT_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <complex>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <time.h>
#include <memory>

#include "event.h"
#include "ring_buffer.h"
#include "nest_time.h"

namespace nest
{


// // conductance windows /////////////////////////////////////////////////////////
// class ConductanceWindow{
// protected:
//   double m_dt = 0.0;
//   // conductance or current, previous timestep
//   double m_g = 0.0, m_g0 = 0.0;

//   // spike buffer
//   RingBuffer m_b_spikes;

// public:
//   virtual void init(){};
//   virtual void reset(){};

//   virtual void set_params(){};
//   virtual void set_params(double tau){};
//   virtual void set_params(double tau_r, double tau_d){};

//   // update functions
//   virtual void update(const long lag){};
//   void handle(SpikeEvent& e);

//   double get_cond(){return m_g;};
// };

// class ExpCond: public ConductanceWindow{
// private:
//     // time scale window
//     double m_tau = 3.;
//     // propagator
//     double m_p = 0.;

// public:
//     ExpCond();
//     ExpCond(double tau);
//     ~ExpCond(){};

//     void init() override;
//     void reset() override {m_g = 0.0; m_g0 = 0.; };

//     void set_params(double tau) override;

//     void update( const long lag ) override;
// };

// class Exp2Cond: public ConductanceWindow{
// private:
//     // conductance g
//     double m_g_r = 0.0, m_g_d = 0.0;
//     // time scales window
//     double m_tau_r = .2, m_tau_d = 3.;
//     double m_norm;
//     // propagators
//     double m_p_r = 0.0, m_p_d = 0.0;

// public:
//     Exp2Cond();
//     Exp2Cond(double tau_r, double tau_d);
//     ~Exp2Cond(){};

//     void init() override;
//     void reset() override {m_g = 0.; m_g0 = 0.; m_p_r = 0.; m_p_d = 0.;};

//     void set_params(double tau_r, double tau_d) override;

//     void update( const long lag ) override;
// };
// ////////////////////////////////////////////////////////////////////////////////


// // voltage dependent factors ///////////////////////////////////////////////////
// class VoltageDependence{
// /*
// base class is used to implement a current based synapse
// */
// protected:
//     double m_e_r = 0.0; // reversal potential

// public:
//     // contructors
//     VoltageDependence(){m_e_r = 0.0;};
//     VoltageDependence(double e_r){m_e_r = e_r;};
//     // functions
//     double get_e_r(){return m_e_r;};
//     virtual double f(double v){return 1.0;};
//     virtual double df_dv(double v){return 0.0;};
// };

// class DrivingForce: public VoltageDependence{
// /*
// Overwrites base class to implement a conductance based synaspes
// */
// public:
//     DrivingForce( double e_r ) : VoltageDependence( e_r ){};
//     double f( double v ) override;
//     double df_dv( double v ) override;
// };

// class NMDA: public VoltageDependence{
// /*
// Overwrites base class to implement an NMDA synaspes
// */
// public:
//     NMDA( double e_r ) : VoltageDependence( e_r ){};
//     double f( double v ) override;
//     double df_dv( double v ) override;

// };
// ////////////////////////////////////////////////////////////////////////////////


// // synapses ////////////////////////////////////////////////////////////////////
// class Synapse{
// /*
// base class implements a current based synapse with exponential conductance
// window of 5 ms
// */
// protected:
//   // conductance windows and voltage dependencies used in synapse
//   std::unique_ptr< ConductanceWindow > m_cond_w;
//   std::unique_ptr< VoltageDependence > m_v_dep;

// public:
//   // constructor, destructor
//   Synapse();
//   ~Synapse(){};

//   virtual void init(){ m_cond_w->init(); };
//   // update functions
//   virtual void update( const long lag );

//   // for numerical integration
//   virtual std::pair< double, double > f_numstep( double v_comp, const long lag);

//   // other functions
//   virtual double f( double v ){ return m_v_dep->f( v ); };
// };

// /*
// Default synapse types defining a standard AMPA synapse, a standard GABA synapse
// and an AMPA+NMDA synapse
// */
// class AMPASyn : public Synapse{
// public:
//   // constructor, destructor
//   AMPASyn();
//   ~AMPASyn(){};
// };

// class GABASyn : public Synapse{
// public:
//   // constructor, destructor
//   GABASyn();
//   ~GABASyn(){};
// };

// class NMDASyn : public Synapse{
// public:
//   // constructor, destructor
//   NMDASyn();
//   ~NMDASyn(){};
// };
// class AMPA_NMDASyn : public Synapse{
// private:
//   double m_nmda_ratio;
//   std::unique_ptr< AMPASyn > m_ampa;
//   std::unique_ptr< NMDASyn > m_nmda;
// public:
//   // constructor, destructor
//   AMPA_NMDASyn();
//   AMPA_NMDASyn( double nmda_ratio );
//   ~AMPA_NMDASyn(){};

//   void init() override { m_ampa->init(); m_nmda->init(); };
//   // update functions
//   void update( const long lag ) override;
//   void handle( SpikeEvent& e ) override;

//   // for numerical integration
//   std::pair< double, double > f_numstep( double v_comp ) override;

//   // other functions
//   double f( double v ) override;
// };
// ////////////////////////////////////////////////////////////////////////////////



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
