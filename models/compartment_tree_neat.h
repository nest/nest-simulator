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
    // tree structure indices
    int m_index;
    int m_parent_index; // negative value means node is root
    std::vector< int > m_child_indices;
    // associated location
    int m_loc_index;
    // voltage variable
    double m_v;
    // electrical parameters
    double m_ca; // compartment capacitance [uF]
    double m_gc; // coupling conductance with parent (meaningless if root) [uS]
    double m_gl; // leak conductance of compartment [uS]
    double m_el; // leak current reversal potential [mV]
    // for numerical integration
    double m_ff = 0., m_gg = 0., m_hh = 0.;
    double m_ff_const, m_gg_const, m_hh_const;
    // passage counter
    int m_n_passed = 0.;

    // constructor, destructor
    CompNode();
    ~CompNode();
    // initialization
    void setSimConstants();
    void reset();
    // for convolution
    inline void gather_input(IODat in);
    inline IODat io();
    inline double calc_v(double v_in);
};


class CompTree{
private:
    /*
    structural data containers for the compartment model
    */
    // number of locations
    int m_n_loc;
    // std::vector of all nodes (first node should be root)
    std::vector< CompNode > m_nodes;

    // root node
    CompNode* m_root;
    // std::vector of pointers to nodes that are leafs
    std::vector< CompNode* > m_leafs;

    // for simulation
    double m_dt;

    //recursion function
    void solve_matrix_downsweep(CompNode* node_ptr,
                                std::vector< CompNode* >::iterator leaf_it);
    void solve_matrix_upsweep(CompNode& node, double vv);

    // set functions for initialization
    void set_leafs();
    void set_root();

public:
    // constructor, destructor
    CompTree();
    ~CompTree();

    // initialization functions for tree structure
    void add_node(int node_index, int parent_index, std::vector< int > child_indices,
                 int loc_index,
                 double ca, double gc,
                 double gl, double el);
    void init();

    // construct the numerical integration matrix and vector
    void construct_matrix(std::vector< double > i_in);
    // solve the matrix equation for next timestep voltage
    void solve_matrix();

    // print functions
    void print_tree();
};


} // namespace