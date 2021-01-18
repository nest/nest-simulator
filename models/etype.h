#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <complex>
#include <tuple>
#include <numeric>
#include <cmath>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <time.h>

#include "nest_time.h"


namespace nest{

class EType{
// Example e-type with a sodium and potassium channel
private:
    /*
    Sodium channel
    */
    // parameters sodium channel (maximal conductance, reversal potential)
    double m_gbar_Na, m_e_Na;
    // state variables sodium channel
    double m_m_Na, m_h_Na;

    /*
    Potassium channel
    */
    // parameters potassium channel (maximal conductance, reversal potential)
    double m_gbar_K, m_e_K;
    // state variables potassium channels
    double m_m_K;


public:
    void init(double g_Na, double e_Na,
              double g_K, double e_K);
    std::pair< double, double > f_numstep(const double v_comp);

    virtual void reset(){};
    virtual void update(){};
    virtual void add_spike(){};
    virtual std::pair< double, double > f_numstep( const double v_comp ){ return std::make_pair( 0., 0. ); };
};

}//