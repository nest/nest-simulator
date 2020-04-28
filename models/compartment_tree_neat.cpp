#include "compartment_tree_neat.h"


// compartment node functions //////////////////////////////////////////////////
nest::CompNode::CompNode(
            int node_index, int parent_index, std::vector< int > child_indices,
            double ca, double gc,
            double gl, double el){
    // tree structure
    m_index = node_index;
    m_parent_index = parent_index;
    m_child_indices = child_indices;
    // electrical parameters
    m_ca = ca; m_gc = gc;
    m_gl = gl; m_el = el;
};

// functions for matrix inversion
inline void nest::CompNode::gather_input(IODat in){
    m_xx += in.g_val; m_yy += in.f_val;
};
inline nest::IODat nest::CompNode::io(){
    IODat out;
    // include inputs from child nodes
    m_gg -= m_xx;
    m_ff -= m_yy;
    // output values
    out.g_val = m_hh * m_hh / m_gg;
    out.f_val = m_ff * m_hh / m_gg;
    return out;
};
inline double nest::CompNode::calc_v(double v_in){
    // reset recursion variables
    m_xx = 0.0; m_yy = 0.0;
    // compute voltage
    m_v = m_ff - v_in * m_hh / m_gg;
    return m_v;
};
////////////////////////////////////////////////////////////////////////////////


// compartment tree functions //////////////////////////////////////////////////
void nest::CompTree::add_node(
                int node_index, int parent_index, std::vector< int > child_indices,
                double ca, double gc,
                double gl, double el){
    /*
    Add a node to the tree structure via the pyhthon interface
    leafs should have [-1] as child indices
    root shoud have -1 as parent index
    */
    CompNode node(node_index, parent_index, child_indices,
                  ca, gc,
                  gl, el);
    // store the node
    m_nodes.push_back(node);
};

void nest::CompTree::init(){
    m_dt = Time::get_resolution().get_ms();

    // initialize compartment voltages at their respective el
    for(std::vector< CompNode >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        node_it->m_v = node_it->m_el;
    }

    set_leafs();
    set_root();
}

void nest::CompTree::set_leafs(){
    m_leafs.clear();
    for(std::vector< CompNode >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        if((*node_it).m_child_indices[0] == -1){
            m_leafs.push_back(&(*node_it));
        }
    }
};
void nest::CompTree::set_root(){
    int found = 0;
    for(std::vector< CompNode >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        if(node_it->m_parent_index < 0){
            found = 1;
            m_root = &(*node_it);
        }
    }
    assert(found == 1);
};

// construct the matrix equation to be solved
void nest::CompTree::construct_matrix(std::vector< double > i_in){
    assert(i_in.size() == m_nodes.size());
    double dt = Time::get_resolution().get_ms();

    // TODO avoid recomputing unnessecary terms every time-step
    for(int ii = 0; ii < int(m_nodes.size()); ii++){
        // renaming for brevity
        int p_ind = m_nodes[ii].m_parent_index;
        std::vector< int > *c_inds = &m_nodes[ii].m_child_indices;

        // matrix diagonal element
        m_nodes[ii].m_gg = m_nodes[ii].m_ca / dt +
                           m_nodes[ii].m_gl / 2. +
                           m_nodes[ii].m_gc / 2.;
        for(std::vector< int >:: iterator jj = c_inds->begin(); jj != c_inds->end(); jj++){
            m_nodes[ii].m_gg += m_nodes[*jj].m_gc / 2.;
        }
        // matrix off diagonal element
        m_nodes[ii].m_hh = -m_nodes[ii].m_gc / 2.;

        // right hand side
        m_nodes[ii].m_ff = m_nodes[ii].m_ca / dt * m_nodes[ii].m_v -
                           m_nodes[ii].m_gl * (m_nodes[ii].m_v / 2. - m_nodes[ii].m_el) -
                           m_nodes[ii].m_gc * (m_nodes[ii].m_v - m_nodes[p_ind].m_v) / 2.;
        for(std::vector< int >:: iterator jj = c_inds->begin(); jj != c_inds->end(); jj++){
            m_nodes[ii].m_ff += m_nodes[ii].m_gc * (m_nodes[ii].m_v - m_nodes[*jj].m_v) / 2.;
        }
    } // for
};

// solve matrix with O(n) algorithm
void nest::CompTree::solve_matrix(){
    std::vector< CompNode* >::iterator leaf_it = m_leafs.begin();
    // start the down sweep (puts to zero the sub diagonal matrix elements)
    solve_matrix_downsweep(m_leafs[0], leaf_it);
    // do up sweep to set voltages
    solve_matrix_upsweep(*m_root, 0.0);
};
void nest::CompTree::solve_matrix_downsweep(CompNode* node_ptr,
                             std::vector< CompNode* >::iterator leaf_it){
    // compute the input output transformation at node
    IODat output = node_ptr->io();
    // move on to the parent layer
    if(node_ptr->m_parent_index != -1){
        CompNode* pnode_ptr = &m_nodes[node_ptr->m_parent_index];
        // gather input from child layers
        pnode_ptr->gather_input(output);
        // move on to next nodes
        pnode_ptr->m_n_passed++;
        if(pnode_ptr->m_n_passed == int(pnode_ptr->m_child_indices.size())){
            pnode_ptr->m_n_passed = 0;
            // move on to next node
            solve_matrix_downsweep(pnode_ptr, leaf_it);
        } else {
            // start at next leaf
            leaf_it++;
            if(leaf_it != m_leafs.end())
                solve_matrix_downsweep(*leaf_it, leaf_it);
        }
    }
};
void nest::CompTree::solve_matrix_upsweep(CompNode& node, double vv){
    // compute node voltage
    vv = node.calc_v(vv);
    // move on to child nodes
    for(std::vector< int >:: iterator ii = node.m_child_indices.begin();
        ii != node.m_child_indices.end(); ii++){
        if(*ii != -1)
            solve_matrix_upsweep(m_nodes[*ii], vv);
    }
};

void nest::CompTree::print_tree(){
    // loop over all nodes
    std::printf(">>> Tree with %d compartments <<<\n", int(m_nodes.size()));
    for(int ii=0; ii<int(m_nodes.size()); ii++){
        CompNode &node = m_nodes[ii];
        std::cout << "Node " << node.m_index << ", ";
        std::cout << "Parent node: " << node.m_parent_index << ", ";
        // std::cout << "Child nodes: " << vec2string(node.m_child_indices) << ", ";
        // std::cout << "Location indices: " << vec2string(node.m_loc_indices) << " ";
        // std::cout << "(new: " << vec2string(node.m_newloc_indices) << ")" << endl;
    }
    std::cout << std::endl;
};
////////////////////////////////////////////////////////////////////////////////