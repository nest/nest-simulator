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


// conductance windows /////////////////////////////////////////////////////////
class ConductanceWindow{
protected:
  double m_dt = 0.0;
  // conductance or current, previous timestep
  double m_g = 0.0, m_g0 = 0.0;

  // spike buffer
  RingBuffer m_b_spikes;

public:
  virtual void init(){};
  virtual void reset(){};

  virtual void set_params(){};
  virtual void set_params(double tau){};
  virtual void set_params(double tau_r, double tau_d){};

  // update functions
  virtual void update(const long lag){};
  void handle(SpikeEvent& e);

  double get_cond(){return m_g;};
};

class ExpCond: public ConductanceWindow{
private:
    // time scale window
    double m_tau = 3.;
    // propagator
    double m_p = 0.;

public:
    ExpCond();
    ExpCond(double tau);
    ~ExpCond(){};

    void init() override;
    void reset() override {m_g = 0.0; m_g0 = 0.; };

    void set_params(double tau) override;

    void update( const long lag ) override;
};

class Exp2Cond: public ConductanceWindow{
private:
    // conductance g
    double m_g_r = 0.0, m_g_d = 0.0;
    // time scales window
    double m_tau_r = .2, m_tau_d = 3.;
    double m_norm;
    // propagators
    double m_p_r = 0.0, m_p_d = 0.0;

public:
    Exp2Cond();
    Exp2Cond(double tau_r, double tau_d);
    ~Exp2Cond(){};

    void init() override;
    void reset() override {m_g = 0.; m_g0 = 0.; m_p_r = 0.; m_p_d = 0.;};

    void set_params(double tau_r, double tau_d) override;

    void update( const long lag ) override;
};
////////////////////////////////////////////////////////////////////////////////


// voltage dependent factors ///////////////////////////////////////////////////
class VoltageDependence{
/*
base class is used to implement a current based synapse
*/
protected:
    double m_e_r = 0.0; // reversal potential

public:
    // contructors
    VoltageDependence(){m_e_r = 0.0;};
    VoltageDependence(double e_r){m_e_r = e_r;};
    // functions
    double get_e_r(){return m_e_r;};
    virtual double f(double v){return 1.0;};
    virtual double df_dv(double v){return 0.0;};
};

class DrivingForce: public VoltageDependence{
/*
Overwrites base class to implement a conductance based synaspes
*/
public:
    DrivingForce( double e_r ) : VoltageDependence( e_r ){};
    double f( double v ) override;
    double df_dv( double v ) override;
};

class NMDA: public VoltageDependence{
/*
Overwrites base class to implement an NMDA synaspes
*/
public:
    NMDA( double e_r ) : VoltageDependence( e_r ){};
    double f( double v ) override;
    double df_dv( double v ) override;

};
////////////////////////////////////////////////////////////////////////////////


// synapses ////////////////////////////////////////////////////////////////////
class Synapse{
/*
base class implements a current based synapse with exponential conductance
window of 5 ms
*/
protected:
  // conductance windows and voltage dependencies used in synapse
  std::unique_ptr< ConductanceWindow > m_cond_w;
  std::unique_ptr< VoltageDependence > m_v_dep;

public:
  // constructor, destructor
  Synapse();
  ~Synapse(){};

  virtual void init(){ m_cond_w->init(); };
  // update functions
  virtual void update( const long lag );
  virtual void handle( SpikeEvent& e );

  // for numerical integration
  virtual std::pair< double, double > f_numstep( double v_comp );

  // other functions
  virtual double f( double v ){ return m_v_dep->f( v ); };
};

/*
Default synapse types defining a standard AMPA synapse, a standard GABA synapse
and an AMPA+NMDA synapse
*/
class AMPASyn : public Synapse{
public:
  // constructor, destructor
  AMPASyn();
  ~AMPASyn(){};
};

class GABASyn : public Synapse{
public:
  // constructor, destructor
  GABASyn();
  ~GABASyn(){};
};

class NMDASyn : public Synapse{
public:
  // constructor, destructor
  NMDASyn();
  ~NMDASyn(){};
};
class AMPA_NMDASyn : public Synapse{
private:
  double m_nmda_ratio;
  std::unique_ptr< AMPASyn > m_ampa;
  std::unique_ptr< NMDASyn > m_nmda;
public:
  // constructor, destructor
  AMPA_NMDASyn();
  AMPA_NMDASyn( double nmda_ratio );
  ~AMPA_NMDASyn(){};

  void init() override { m_ampa->init(); m_nmda->init(); };
  // update functions
  void update( const long lag ) override;
  void handle( SpikeEvent& e ) override;

  // for numerical integration
  std::pair< double, double > f_numstep( double v_comp ) override;

  // other functions
  double f( double v ) override;
};
////////////////////////////////////////////////////////////////////////////////

} // namespace

#endif /* #ifndef SYNAPSES_NEAT_H */
