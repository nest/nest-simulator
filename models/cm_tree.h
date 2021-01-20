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
#include "ring_buffer.h"

// compartmental model
#include "cm_syns.h"
#include "cm_etype.h"

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


namespace nest{


class Compartment{
private:
    // aggragators for numerical integration
    double m_xx;
    double m_yy;

public:
    // compartment_index
    long m_index;
    // parent compartment index
    long m_p_index;
    // tree structure indices
    Compartment* m_parent;
    std::vector< Compartment > m_children;
    // vector for synapses
    std::vector< std::shared_ptr< Synapse > > m_syns;
    // etype
    EType m_etype;
    // buffer for currents
    RingBuffer m_currents;
    // voltage variable
    double m_v;
    // electrical parameters
    double m_ca; // compartment capacitance [uF]
    double m_gc; // coupling conductance with parent (meaningless if root) [uS]
    double m_gl; // leak conductance of compartment [uS]
    double m_el; // leak current reversal potential [mV]
    // for numerical integration
    double m_ff;
    double m_gg;
    double m_hh;
    // passage counter
    int m_n_passed;

    // constructor, destructor
    Compartment(const long compartment_index, const long parent_index);
    Compartment(const long compartment_index, const long parent_index,
	         const DictionaryDatum& compartment_params);
    ~Compartment(){};

    // initialization
    void init();

    // matrix construction
    void construct_matrix_element( const long lag );

    // maxtrix inversion
    inline void gather_input( const std::pair< double, double > in );
    inline std::pair< double, double > io();
    inline double calc_v( const double v_in );
}; // Compartment


/*
Short helper functions for solving the matrix equation. Can hopefully be inlined
*/
inline void
nest::Compartment::gather_input( const std::pair< double, double > in)
{
    m_xx += in.first; m_yy += in.second;
};
inline std::pair< double, double >
nest::Compartment::io()
{
    // include inputs from child compartments
    m_gg -= m_xx;
    m_ff -= m_yy;

    // output values
    double g_val( m_hh * m_hh / m_gg );
    double f_val( m_ff * m_hh / m_gg );

    return std::make_pair(g_val, f_val);
};
inline double
nest::Compartment::calc_v( const double v_in )
{
    // reset recursion variables
    m_xx = 0.0; m_yy = 0.0;

    // compute voltage
    m_v = (m_ff - v_in * m_hh) / m_gg;

    return m_v;
};


class CompTree{
private:
    /*
    structural data containers for the compartment model
    */
    Compartment m_root;
    std::vector< long > m_compartment_indices;
    std::vector< Compartment* > m_compartments;
    std::vector< Compartment* > m_leafs;

    // recursion functions for matrix inversion
    void solve_matrix_downsweep(Compartment* compartment_ptr,
                                std::vector< Compartment* >::iterator leaf_it);
    void solve_matrix_upsweep(Compartment* compartment, double vv);

    // set functions for initialization
    void set_compartments();
    void set_compartments( Compartment* compartment );
    void set_leafs();

public:
    // constructor, destructor
    CompTree();
    ~CompTree(){};

    // initialization functions for tree structure
    void add_compartment( const long compartment_index, const long parent_index,
                          const DictionaryDatum& compartment_params );
    void init();

    // get a compartment pointer from the tree
    Compartment* get_compartment( const long compartment_index );
    Compartment* get_compartment( const long compartment_index,
                                  Compartment* compartment,
                                  const long raise_flag );
    Compartment* get_root(){ return &m_root; };

    // get voltage values
    std::vector< double > get_voltage() const;
    double get_compartment_voltage( const long compartment_index );

    // construct the numerical integration matrix and vector
    void construct_matrix( const long lag );
    void construct_matrix( const std::vector< double >& i_in, const long lag );
    // solve the matrix equation for next timestep voltage
    void solve_matrix();

    // print function
    void print_tree() const;
}; // CompTree

} // namespace
