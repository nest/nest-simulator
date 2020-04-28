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

#include "ring_buffer.h"

namespace nest
{


// conductance windows /////////////////////////////////////////////////////////
class ConductanceWindow{
protected:
  double m_dt = 0.0;
  // conductance g
  double m_g = 0.0;

  // spike buffer
  RingBuffer m_b_spikes;

public:
  virtual void init(){};
  virtual void reset(){};

  virtual void set_params(){};
  virtual void set_params(double tau){};
  virtual void set_params(double tau_r, double tau_d){};

  // update functions
  virtual void update( const long lag ){};
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
    void init() override;
    void reset() override {m_g = 0.0;};

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
    void init() override;
    void reset() override {m_g = 0.; m_p_r = 0.; m_p_d = 0.;};

    void set_params(double tau_r, double tau_d) override;

    void update( const long lag ) override;
};
////////////////////////////////////////////////////////////////////////////////


// voltage dependent factors////////////////////////////////////////////////////
class VoltageDependence{
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
public:
    DrivingForce(double e_r) : VoltageDependence(e_r){};
    double f(double v) override;
    double df_dv(double v) override;
};

class NMDA: public VoltageDependence{
public:
    NMDA(double e_r) : VoltageDependence(e_r){};
    double f(double v) override;
    double df_dv(double v) override;

};
////////////////////////////////////////////////////////////////////////////////

} // namespace
