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
    double m_xx = 0., m_yy = 0.;
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
    double m_v = 0.;
    // electrical parameters
    double m_ca; // compartment capacitance [uF]
    double m_gc; // coupling conductance with parent (meaningless if root) [uS]
    double m_gl; // leak conductance of compartment [uS]
    double m_el; // leak current reversal potential [mV]
    // for numerical integration
    double m_ff = 0., m_gg = 0., m_hh = 0.;
    // passage counter
    int m_n_passed = 0.;

    // constructor, destructor
    CompNode(long node_index, CompNode* parent,
            double ca, double gc,
            double gl, double el);
    ~CompNode(){};
    // initialization
    void init( const double dt );
    void add_synapse(std::shared_ptr< Synapse > syn);
    // matrix construction
    void construct_matrix_element();
    void add_synapse_contribution(const long lag);
    void add_channel_contribution();
    // maxtrix inversion
    inline void gather_input(IODat in);
    inline IODat io();
    inline double calc_v(double v_in);
};


class CompTree{
private:
    /*
    structural data containers for the compartment model
    */
    // root node
    CompNode m_root = CompNode(0, NULL, 1., 0., 1., 0.);
    // convenience std::vector of pointers to all nodes, depth first iteration
    std::vector< CompNode* > m_nodes;
    // std::vector of pointers to nodes that are leafs
    std::vector< CompNode* > m_leafs;

    // timestep for simulation [ms]
    double m_dt = 0.1;

    //recursion function
    void solve_matrix_downsweep(CompNode* node_ptr,
                                std::vector< CompNode* >::iterator leaf_it);
    void solve_matrix_upsweep(CompNode* node, double vv);

    // set functions for initialization
    void set_nodes();
    void set_nodes(CompNode* node);
    void set_leafs();

public:
    // constructor, destructor
    CompTree(){};
    ~CompTree(){};

    // initialization functions for tree structure
    void add_node(long node_index, long parent_index,
                 double ca, double gc,
                 double gl, double el);
    void init( const double dt );

    // getters and setters
    CompNode* find_node(long node_index);
    CompNode* find_node(long node_index, CompNode* node);
    std::vector< double > get_voltage();
    double get_node_voltage(long node_index);

    // construct the numerical integration matrix and vector
    void construct_matrix(const long lag);
    void construct_matrix(std::vector< double > i_in, const long lag);
    // solve the matrix equation for next timestep voltage
    void solve_matrix();

    // print functions
    void print_tree();
};


} // namespace
