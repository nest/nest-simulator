#include "compartment_tree_neat.h"


// compartment node functions //////////////////////////////////////////////////
nest::CompNode::CompNode(
            long node_index, CompNode* parent,
            double ca, double gc,
            double gl, double el){
    // tree structure
    m_index = node_index;
    m_parent = parent;
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
    m_v = (m_ff - v_in * m_hh) / m_gg;
    return m_v;
};
////////////////////////////////////////////////////////////////////////////////


// compartment tree functions //////////////////////////////////////////////////
/*
Add a node to the tree structure via the python interface
root shoud have -1 as parent index. Add root node first. Assumes parent of node
is already added
*/
void nest::CompTree::add_node(
                long node_index, long parent_index,
                double ca, double gc,
                double gl, double el){
    CompNode* parent;
    if(parent_index >= 0){
        parent = find_node(parent_index);
    } else {
        parent = NULL;
    }

    CompNode* node = new CompNode(node_index, parent,
                                  ca, gc,
                                  gl, el);

    if(parent_index >= 0){
        parent->m_children.push_back(*node);
    } else {
        m_root = *node;
    }
};

/*
find a node with given index in the three structure. Returns NULL pointer if
index is node found
*/
nest::CompNode* nest::CompTree::find_node(long node_index){
    nest::CompNode* r_node = NULL;

    if(node_index == m_root.m_index)
        r_node = &m_root;
    else
        r_node = find_node(node_index, &m_root);

    return r_node;
}
nest::CompNode* nest::CompTree::find_node(long node_index, CompNode* node){
    nest::CompNode* r_node = NULL;
    auto child_it = node->m_children.begin();
    long found = 0;

    while(found != 1 && child_it != node->m_children.end()){
        if(child_it->m_index == node_index){
            r_node = &(*child_it);
        } else {
            found = 1;
            r_node = find_node(node_index, &(*child_it));
        }
        child_it++;
    }

    return r_node;
};

// initialization functions
void nest::CompTree::init(){
    m_dt = Time::get_resolution().get_ms();

    set_nodes();
    set_leafs();

    // initialize compartment voltages at their respective el
    for(std::vector< CompNode* >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        (*node_it)->m_v = (*node_it)->m_el;
    }
}
void nest::CompTree::set_nodes(){
    m_nodes.clear();
    set_nodes(&m_root);
}
void nest::CompTree::set_nodes(CompNode* node){
    m_nodes.push_back(node);

    for(auto child_it = node->m_children.begin();
        child_it != node->m_children.end(); child_it++){
        set_nodes(&(*child_it));
    }
}
void nest::CompTree::set_leafs(){
    m_leafs.clear();
    for(std::vector< CompNode* >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        if(int((*node_it)->m_children.size()) == 0){
            m_leafs.push_back(*node_it);
        }
    }
};

// getters and setters
std::vector< double > nest::CompTree::get_voltage(){
    std::vector< double > v_comp;
    for(std::vector< CompNode* >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        v_comp.push_back((*node_it)->m_v);
    }
    return v_comp;
}

// construct the matrix equation to be solved
void nest::CompTree::construct_matrix(std::vector< double > i_in){
    assert(i_in.size() == m_nodes.size());
    double dt = Time::get_resolution().get_ms();

    // temporary implementation of current input
    for(long ii=0; ii != long(i_in.size()); ii++){
        m_nodes[ii]->m_ff = i_in[ii];
    }

    // TODO avoid recomputing unnessecary terms every time-step
    for(std::vector< CompNode* >::iterator node_it = m_nodes.begin();
        node_it != m_nodes.end(); node_it++){
        // renaming for brevity
        CompNode* node = *node_it;
        CompNode* parent = node->m_parent;

        // matrix diagonal element
        node->m_gg = node->m_ca / dt + node->m_gl / 2.;
        if(parent != NULL){
            node->m_gg += node->m_gc / 2.;
            // matrix off diagonal element
            node->m_hh = -node->m_gc / 2.;
        }
        for(std::vector< CompNode >::iterator child_it = node->m_children.begin();
            child_it != node->m_children.end(); child_it++){
            node->m_gg += (*child_it).m_gc / 2.;
        }
        // right hand side
        node->m_ff += node->m_ca / dt * node->m_v -
                      node->m_gl * (node->m_v / 2. - node->m_el);
        if(parent != NULL){
            node->m_ff -= node->m_gc * (node->m_v - parent->m_v) / 2.;
        }
        for(std::vector< CompNode >::iterator child_it = node->m_children.begin();
            child_it != node->m_children.end(); child_it++){
            node->m_ff -= node->m_gc * (node->m_v - (*child_it).m_v) / 2.;
        }
    } // for
};
void nest::CompTree::add_synapse_contribution(int comp_ind, std::pair< double, double > gf_syn){
  m_nodes[comp_ind]->m_gg += gf_syn.first;
  m_nodes[comp_ind]->m_ff += gf_syn.second;
}

// solve matrix with O(n) algorithm
void nest::CompTree::solve_matrix(){
    std::vector< CompNode* >::iterator leaf_it = m_leafs.begin();
    // start the down sweep (puts to zero the sub diagonal matrix elements)
    solve_matrix_downsweep(m_leafs[0], leaf_it);
    // do up sweep to set voltages
    solve_matrix_upsweep(&m_root, 0.0);
};
void nest::CompTree::solve_matrix_downsweep(CompNode* node,
                             std::vector< CompNode* >::iterator leaf_it){
    // compute the input output transformation at node
    IODat output = node->io();
    // move on to the parent layer
    if(node->m_parent != NULL){
        CompNode* parent = node->m_parent;
        // gather input from child layers
        parent->gather_input(output);
        // move on to next nodes
        parent->m_n_passed++;
        if(parent->m_n_passed == int(parent->m_children.size())){
            parent->m_n_passed = 0;
            // move on to next node
            solve_matrix_downsweep(parent, leaf_it);
        } else {
            // start at next leaf
            leaf_it++;
            if(leaf_it != m_leafs.end())
                solve_matrix_downsweep(*leaf_it, leaf_it);
        }
    }
};
void nest::CompTree::solve_matrix_upsweep(CompNode* node, double vv){
    // compute node voltage
    vv = node->calc_v(vv);
    // move on to child nodes
    for(std::vector< CompNode >::iterator child_it = node->m_children.begin();
        child_it != node->m_children.end(); child_it++){
        solve_matrix_upsweep(&(*child_it), vv);
    }
};

void nest::CompTree::print_tree(){
    // loop over all nodes
    std::cout << "--- Root " << m_root.m_index << std::endl;
    std::printf(">>> Tree with %d compartments <<<\n", int(m_nodes.size()));
    for(int ii=0; ii<int(m_nodes.size()); ii++){
        CompNode* node = m_nodes[ii];
        std::cout << "Node " << node->m_index << ", ";
        if(node->m_parent != NULL)
            std::cout << "Parent node: " << node->m_parent->m_index << ", ";
        // std::cout << "Child nodes: " << vec2string(node.m_child_indices) << ", ";
        // std::cout << "Location indices: " << vec2string(node.m_loc_indices) << " ";
        // std::cout << "(new: " << vec2string(node.m_newloc_indices) << ")" << endl;
        std::cout << std::endl;
    }
    std::cout << std::endl;
};
////////////////////////////////////////////////////////////////////////////////
