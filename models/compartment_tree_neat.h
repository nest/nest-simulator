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
#include "synapses_neat.h"
#include "ionchannels_neat.h"

namespace nest{


struct IODat{
    // data container to communicate between nodes for inversion
    double g_val;
    double f_val;
};


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
    // tree structure indices
    CompNode* m_parent;
    std::vector< CompNode > m_children;
    // vector for synapses
    std::vector< std::shared_ptr< Synapse > > m_syns;
    // vector for ion channels
    std::vector< std::shared_ptr< IonChannel > > m_chans;
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
    CompNode(const long node_index, CompNode* parent,
	     const double ca, const double gc,
	     const double gl, const double el);
    ~CompNode(){};

    // initialization
    void init();

    // matrix construction
    void construct_matrix_element();
    void add_synapse_contribution( const long lag );
    void add_channel_contribution();

    // maxtrix inversion
    inline void gather_input( const IODat in );
    inline IODat io();
    inline double calc_v( const double v_in );
};

inline void nest::CompNode::gather_input( const IODat in)
{
    m_xx += in.g_val; m_yy += in.f_val;
};

inline nest::IODat nest::CompNode::io()
{
    IODat out;

    // include inputs from child nodes
    m_gg -= m_xx;
    m_ff -= m_yy;

    // output values
    out.g_val = m_hh * m_hh / m_gg;
    out.f_val = m_ff * m_hh / m_gg;

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
    CompNode* m_root;
    std::vector< CompNode* > m_nodes;
    std::vector< CompNode* > m_leafs;

    // timestep for simulation [ms]
    double m_dt;

    //recursion function
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
		   const double ca, const double gc,
		   const double gl, const double el);
    void init();

    // getters and setters
    CompNode* find_node( const long node_index );
    CompNode* find_node( const long node_index, CompNode* node );
    CompNode* find_node( const long node_index, CompNode* node, const long raise_flag );
    CompNode* get_root(){ return m_root; };
    std::vector< double > get_voltage() const;
    double get_node_voltage( const long node_index );

    // construct the numerical integration matrix and vector
    void construct_matrix( const long lag );
    void construct_matrix( const std::vector< double >& i_in, const long lag );
    // solve the matrix equation for next timestep voltage
    void solve_matrix();

    // print functions
    void print_tree() const;
};


} // namespace
