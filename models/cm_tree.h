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


class CompNode{
private:
    // aggragators for numberical integrations
    double m_xx;
    double m_yy;
    // time step
    double m_dt;

public:
    // node_index
    long m_index;
    // parent node index
    long m_p_index;
    // tree structure indices
    CompNode* m_parent;
    std::vector< CompNode > m_children;
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
    CompNode(const long node_index, const long parent_index);
    CompNode(const long node_index, const long parent_index,
	         const DictionaryDatum& compartment_params);
    ~CompNode(){};

    // initialization
    void init();

    // matrix construction
    void construct_matrix_element( const long lag );

    // maxtrix inversion
    inline void gather_input( const std::pair< double, double > in );
    inline std::pair< double, double > io();
    inline double calc_v( const double v_in );
}; // CompNode

inline void nest::CompNode::gather_input( const std::pair< double, double > in)
{
    m_xx += in.first; m_yy += in.second;
};

inline std::pair< double, double > nest::CompNode::io()
{
    // include inputs from child nodes
    m_gg -= m_xx;
    m_ff -= m_yy;

    // // output values
    // out.first = m_hh * m_hh / m_gg;
    // out.second = m_ff * m_hh / m_gg;

    std::pair< double, double > out(m_hh * m_hh / m_gg, m_ff * m_hh / m_gg);

    return out;
};

inline double nest::CompNode::calc_v( const double v_in )
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
    CompNode m_root;
    std::vector< long > m_node_indices;
    std::vector< CompNode* > m_nodes;
    std::vector< CompNode* > m_leafs;

    // recursion functions for matrix inversion
    void solve_matrix_downsweep(CompNode* node_ptr,
                                std::vector< CompNode* >::iterator leaf_it);
    void solve_matrix_upsweep(CompNode* node, double vv);

    // set functions for initialization
    void set_nodes();
    void set_nodes( CompNode* node );
    void set_leafs();

public:
    // constructor, destructor
    CompTree();
    ~CompTree(){};

    // initialization functions for tree structure
    void add_node( const long node_index, const long parent_index,
                   const DictionaryDatum& compartment_params );
    void init();

    // get a node pointer from the tree
    CompNode* find_node( const long node_index );
    CompNode* find_node( const long node_index, CompNode* node );
    CompNode* find_node( const long node_index, CompNode* node, const long raise_flag );
    CompNode* get_root(){ return &m_root; };

    // get voltage values
    std::vector< double > get_voltage() const;
    double get_node_voltage( const long node_index );

    // construct the numerical integration matrix and vector
    void construct_matrix( const long lag );
    void construct_matrix( const std::vector< double >& i_in, const long lag );
    // solve the matrix equation for next timestep voltage
    void solve_matrix();

    // print function
    void print_tree() const;
}; // CompTree

} // namespace
