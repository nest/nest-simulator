#include "etype.h"


nest::etype::init(double g_Na, double e_Na,
                  double g_K, double e_K)
{
    m_gbar_Na = g_Na; m_e_Na = e_Na;
    m_gbar_K = g_K; m_e_K = e_K;
}

nest::etype::f_numstep(const double v_comp, const double lag)
{
    /*
    Sodium channel
    */
    double g_Na;
    if (m_gbar_Na > 1e-9)
    {
        // activation and timescale of state variable 'm'
        double m_m_inf_Na = 0.182*(v_comp + 38.0)/((1.0 - exp((-v_comp - 38.0)/6.0))*((-0.124)*(v_comp + 38.0)/(1.0 - exp((v_comp + 38.0)/6.0)) + 0.182*(v_comp + 38.0)/(1.0 - exp((-v_comp - 38.0)/6.0))));
        double m_tau_m_Na = 0.33898305084745761/((-0.124)*(v_comp + 38.0)/(1.0 - exp((v_comp + 38.0)/6.0)) + 0.182*(v_comp + 38.0)/(1.0 - exp((-v_comp - 38.0)/6.0)));

        // activation and timescale of state variable 'h'
        double m_h_inf_K = -0.014999999999999999*(v_comp + 66.0)/((1.0 - exp((v_comp + 66.0)/6.0))*((-0.014999999999999999)*(v_comp + 66.0)/(1.0 - exp((v_comp + 66.0)/6.0)) + 0.014999999999999999*(v_comp + 66.0)/(1.0 - exp((-v_comp - 66.0)/6.0))));
        double m_tau_h_K = 0.33898305084745761/((-0.014999999999999999)*(v_comp + 66.0)/(1.0 - exp((v_comp + 66.0)/6.0)) + 0.014999999999999999*(v_comp + 66.0)/(1.0 - exp((-v_comp - 66.0)/6.0)));

        // advance state variable 'm' one timestep
        double p_m_Na = exp(-lag / m_tau_m_Na);
        m_m_Na *= p_m_Na ;
        m_m_Na += (1. - p_m_Na) *  m_m_inf_Na;

        // advance state variable 'h' one timestep
        double p_h_Na = exp(-lag / m_tau_h_Na);
        m_h_Na *= p_h_Na ;
        m_h_Na += (1. - p_h_Na) *  m_h_inf_Na;

        // compute the conductance of the sodium channel
        g_Na = m_gbar_Na * pow(m_m_Na, 3) * m_h_Na;
    }
    else
    {
        g_Na = 0.;
    }

    /*
    Potassium channel
    */
    double g_K;
    if (m_gbar_K > 1e-9)
    {
        // activation and timescale of state variable 'm'
        double m_m_inf_K = 1.0/(exp((18.699999999999999 - v_comp)/9.6999999999999993) + 1.0);
        double m_tau_m_K = 4.0/(exp((-v_comp - 46.560000000000002)/44.140000000000001) + 1.0);

        // advance state variable 'm' one timestep
        double p_m_K = exp(-lag / m_tau_m_K);
        m_m_K *= p_m_K;
        m_m_K += (1. - p_m_K) *  m_m_inf_K;

        // compute the conductance of the potassium channel
        g_K = m_gbar_K * pow(m_m_K, 4);
    }
    else
    {
        g_K = 0.;
    }

    // construct variables for integration, sums run over all ionchannels in etype
    double g_val = (g_Na + g_K) / 2. ;
    double i_val = g_Na * ( m_e_Na - v_comp / 2. ) + g_K * (m_e_K - v_comp / 2.) ;

    return std::make_pair(g_val, i_val);

}