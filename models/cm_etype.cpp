#include "cm_etype.h"


nest::EType::EType()
    // sodium channel
    : m_Na_(0.0)
    , h_Na_(0.0)
    , gbar_Na_(0.0)
    , e_Na_(0.0)
    // potassium channel
    , n_K_(0.0)
    , gbar_K_(0.0)
    , e_K_(0.0)
{}
nest::EType::EType(const DictionaryDatum& compartment_params)
    // sodium channel
    : m_Na_(0.0)
    , h_Na_(0.0)
    , gbar_Na_(0.0)
    , e_Na_(50.)
    // potassium channel
    , n_K_(0.0)
    , gbar_K_(0.0)
    , e_K_(-85.)
{
    // update sodium channel parameters
    if( compartment_params->known( "g_Na" ) )
        gbar_Na_ = getValue< double >( compartment_params, "g_Na" );
    if( compartment_params->known( "e_Na" ) )
        e_Na_ = getValue< double >( compartment_params, "e_Na" );
    // update potassium channel parameters
    if( compartment_params->known( "g_K" ) )
        gbar_K_ = getValue< double >( compartment_params, "g_K" );
    if( compartment_params->known( "e_K" ) )
        e_K_ = getValue< double >( compartment_params, "e_K" );

}

std::pair< double, double > nest::EType::f_numstep(const double v_comp, const double dt)
{
    double g_val = 0., i_val = 0.;

    /*
    Sodium channel
    */
    if (gbar_Na_ > 1e-9)
    {
        // activation and timescale of state variable 'm'
        double m_inf_Na = (0.182*v_comp + 6.3723659999999995)/((1.0 - 0.020438532058318047*exp(-0.1111111111111111*v_comp))*((-0.124*v_comp - 4.3416119999999996)/(1.0 - 48.927192870146527*exp(0.1111111111111111*v_comp)) + (0.182*v_comp + 6.3723659999999995)/(1.0 - 0.020438532058318047*exp(-0.1111111111111111*v_comp))));
        double tau_m_Na = 0.3115264797507788/((-0.124*v_comp - 4.3416119999999996)/(1.0 - 48.927192870146527*exp(0.1111111111111111*v_comp)) + (0.182*v_comp + 6.3723659999999995)/(1.0 - 0.020438532058318047*exp(-0.1111111111111111*v_comp)));

        // activation and timescale of state variable 'h'
        double h_inf_Na = 1.0/(exp(0.16129032258064516*v_comp + 10.483870967741936) + 1.0);
        double tau_h_Na = 0.3115264797507788/((-0.0091000000000000004*v_comp - 0.68261830000000012)/(1.0 - 3277527.8765015295*exp(0.20000000000000001*v_comp)) + (0.024*v_comp + 1.200312)/(1.0 - 4.5282043263959816e-5*exp(-0.20000000000000001*v_comp)));

        // advance state variable 'm' one timestep
        double p_m_Na = exp(-dt / tau_m_Na);
        m_Na_ *= p_m_Na ;
        m_Na_ += (1. - p_m_Na) *  m_inf_Na;

        // advance state variable 'h' one timestep
        double p_h_Na = exp(-dt / tau_h_Na);
        h_Na_ *= p_h_Na ;
        h_Na_ += (1. - p_h_Na) *  h_inf_Na;

        // compute the conductance of the sodium channel
        double g_Na = gbar_Na_ * pow(m_Na_, 3) * h_Na_;

        // add to variables for numerical integration
        g_val += g_Na / 2.;
        i_val += g_Na * ( e_Na_ - v_comp / 2. );
    }

    /*
    Potassium channel
    */
    if (gbar_K_ > 1e-9)
    {
        // activation and timescale of state variable 'm'
        double n_inf_K = 0.02*(v_comp - 25.0)/((1.0 - exp((25.0 - v_comp)/9.0))*((-0.002)*(v_comp - 25.0)/(1.0 - exp((v_comp - 25.0)/9.0)) + 0.02*(v_comp - 25.0)/(1.0 - exp((25.0 - v_comp)/9.0))));
        double tau_n_K = 0.3115264797507788/((-0.002)*(v_comp - 25.0)/(1.0 - exp((v_comp - 25.0)/9.0)) + 0.02*(v_comp - 25.0)/(1.0 - exp((25.0 - v_comp)/9.0)));

        // advance state variable 'm' one timestep
        double p_n_K = exp(-dt / tau_n_K);
        n_K_ *= p_n_K;
        n_K_ += (1. - p_n_K) *  n_inf_K;

        // compute the conductance of the potassium channel
        double g_K = gbar_K_ * n_K_;

        // add to variables for numerical integration
        g_val += g_K / 2.;
        i_val += g_K * ( e_K_ - v_comp / 2. );
    }

    return std::make_pair(g_val, i_val);

}