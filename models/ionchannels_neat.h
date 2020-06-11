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


namespace nest{


class IonChannel{
public:
    virtual void init( const double dt ){};

    virtual void reset(){};
    virtual void update(){};
    virtual void add_spike(){};
    virtual std::pair< double, double > f_numstep(const double v_comp){return std::make_pair(0., 0.);};
};


class FakePotassium : public IonChannel{
private:
    // time scale
    double m_tau = 10.;
    // single spike contribution to conductance
    double m_g_step = 10.;
    // conductance
    double m_g = 0.;
    double m_g0 = 0.;
    //reversal
    double m_e_r = -85.;
    // propagator
    double m_p = 0.;

public:
    FakePotassium();
    FakePotassium(double tau, double e_r, double g_max){
        m_tau = tau; m_e_r = e_r; m_g_step = g_max;
    };

    void init( const double dt ){
        m_p = std::exp(-dt / m_tau);
    };

    void reset(){m_g = 0.0; m_g0 = 0.;};
    void update(){m_g *= m_p;};

    void add_spike(){m_g += m_g_step;};

    std::pair< double, double > f_numstep(const double v_comp){
        double g_val = m_g0 / 2.;
        double i_val = (m_g0 + m_g) / 2. * (m_e_r - v_comp) + m_g0 * v_comp/2.;

        return std::make_pair(g_val, i_val);
    };
};

} // namespace