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
// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// 0Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


namespace nest{

class EType{
// Example e-type with a sodium and potassium channel
private:
    /*
    Sodium channel
    */
    // state variables sodium channel
    double m_m_Na, m_h_Na;
    // parameters sodium channel (maximal conductance, reversal potential)
    double m_gbar_Na, m_e_Na;

    /*
    Potassium channel
    */
    // state variables potassium channels
    double m_n_K;
    // parameters potassium channel (maximal conductance, reversal potential)
    double m_gbar_K, m_e_K;


public:
    // constructor, destructor
    EType();
    EType(const DictionaryDatum& compartment_params);
    ~EType(){};

    void add_spike(){};
    std::pair< double, double > f_numstep(const double v_comp, const double lag);
};

}//