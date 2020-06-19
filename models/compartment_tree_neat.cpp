#include "compartment_tree_neat.h"


// compartment node functions //////////////////////////////////////////////////
nest::CompNode::CompNode( const long node_index, const long parent_index,
			  const double ca, const double gc,
			  const double gl, const double el)
  : m_xx( 0.0 )
  , m_yy( 0.0 )
  , m_dt( 0.1 )
  , m_index( node_index )
  , m_p_index( parent_index )
  , m_parent( nullptr )
  , m_v( 0.0 )
  , m_ca( ca )
  , m_gc( gc )
  , m_gl( gl )
  , m_el( el )
  , m_ff( 0.0 )
  , m_gg( 0.0 )
  , m_hh( 0.0 )
  , m_n_passed( 0 )
{
  // m_children.resize( 0 );
  m_syns.resize( 0 );
  m_chans.resize( 0 );
};

void nest::CompNode::init()
{
    m_v = m_el;

    for( auto  syn_it = m_syns.begin(); syn_it != m_syns.end(); ++syn_it )
    {
        (*syn_it)->init();
    }

    for( auto chan_it = m_chans.begin(); chan_it != m_chans.end(); ++chan_it )
    {
        (*chan_it)->init();
    }

    // initialize the buffer
    m_currents.clear();
}

// for matrix construction
void nest::CompNode::construct_matrix_element()
{
   const double dt = Time::get_resolution().get_ms();

    // matrix diagonal element
    m_gg = m_ca / dt + m_gl / 2.;

    if( m_parent != nullptr )
    {
        m_gg += m_gc / 2.;
        // matrix off diagonal element
        m_hh = -m_gc / 2.;
    }

    for( auto child_it = m_children.begin(); child_it != m_children.end(); ++child_it )
    {
        m_gg += (*child_it).m_gc / 2.;
    }

    // right hand side
    m_ff += m_ca / dt * m_v - m_gl * (m_v / 2. - m_el);

    if( m_parent != nullptr )
    {
        m_ff -= m_gc * (m_v - m_parent->m_v) / 2.;
    }

    for( auto child_it = m_children.begin(); child_it != m_children.end(); ++child_it )
    {
        m_ff -= (*child_it).m_gc * (m_v - (*child_it).m_v) / 2.;
    }
}

void nest::CompNode::add_input_current( const long lag )
{
    m_ff += m_currents.get_value( lag );
}

void nest::CompNode::add_synapse_contribution( const long lag )
{
    std::pair< double, double > gf_syn(0., 0.);

    for( auto syn_it = m_syns.begin(); syn_it != m_syns.end(); ++syn_it )
    {


        (*syn_it)->update(lag);
        gf_syn = (*syn_it)->f_numstep(m_v);

        m_gg += gf_syn.first;
        m_ff += gf_syn.second;
    }
}

void nest::CompNode::add_channel_contribution()
{
    std::pair< double, double > gf_chan(0., 0.);

    for( auto chan_it = m_chans.begin(); chan_it != m_chans.end(); ++chan_it )
    {
        (*chan_it)->update();
        gf_chan = (*chan_it)->f_numstep(m_v);

        m_gg += gf_chan.first;
        m_ff += gf_chan.second;
    }
}
////////////////////////////////////////////////////////////////////////////////

// compartment tree functions //////////////////////////////////////////////////

nest::CompTree::CompTree()
  : m_root( 0, -1, 1., 1., 1., 1. )
  , m_dt( 0.1 )
{
  m_nodes.resize( 0 );
  m_leafs.resize( 0 );
}

/*
Add a node to the tree structure via the python interface
root shoud have -1 as parent index. Add root node first. Assumes parent of node
is already added
*/
void nest::CompTree::add_node( const long node_index, const long parent_index,
			       const double ca, const double gc,
			       const double gl, const double el )
{
    CompNode* node = new CompNode( node_index, parent_index,
				   ca, gc,
				   gl, el );

    if( parent_index >= 0 )
    {
        CompNode* parent = find_node( parent_index );
        parent->m_children.push_back( *node );
    }
    else
    {
        m_root = *node;
    }

    m_node_indices.push_back(node_index);

    set_nodes();
};

nest::CompNode* nest::CompTree::find_node( const long node_index ){
    return find_node( node_index, get_root() );
}

nest::CompNode* nest::CompTree::find_node( const long node_index, CompNode* node ){
    return find_node( node_index, get_root(), 1 );
}

nest::CompNode* nest::CompTree::find_node( const long node_index, CompNode* node,
                                           const long raise_flag)
{
    CompNode* r_node = nullptr;

    if( node->m_index == node_index )
    {
        r_node = node;
    }
    else
    {
        auto child_it = node->m_children.begin();
        while( !r_node && child_it != node->m_children.end() )
        {
            r_node = find_node( node_index, &(*child_it), 0 );
            ++child_it;
        }
    }

    if( !r_node && raise_flag )
    {
        std::ostringstream err_msg;
        err_msg << "Node index " << node_index << " not in tree";
        throw BadProperty(err_msg.str());
    }

    return r_node;
}

// initialization functions
void nest::CompTree::init()
{
    set_nodes();
    set_leafs();

    // initialize the nodes
    for( auto node_it = m_nodes.begin(); node_it != m_nodes.end(); ++node_it )
    {
        ( *node_it )->m_parent = find_node( ( *node_it )->m_p_index, &m_root, 0);
        ( *node_it )->init();
    }
}

void nest::CompTree::set_nodes()
{
    m_nodes.clear();

    for( auto node_idx_it = m_node_indices.begin(); node_idx_it != m_node_indices.end(); ++node_idx_it )
    {
        m_nodes.push_back( find_node( *node_idx_it ) );
    }

}

void nest::CompTree::set_leafs()
{
    m_leafs.clear();
    for( auto node_it = m_nodes.begin(); node_it != m_nodes.end(); ++node_it )
    {
        if( int((*node_it)->m_children.size()) == 0 )
	{
            m_leafs.push_back( *node_it );
        }
    }
};

// getters and setters
std::vector< double > nest::CompTree::get_voltage() const
{
    std::vector< double > v_comp;
    for( auto node_it = m_nodes.cbegin();
	 node_it != m_nodes.cend(); ++node_it )
    {
      v_comp.push_back( (*node_it)->m_v );
    }
    return v_comp;
}

// getters and setters
double nest::CompTree::get_node_voltage( const long node_index )
{
    const CompNode* node = find_node( node_index );
    return node->m_v;
}

// construct the matrix equation to be solved
void nest::CompTree::construct_matrix( const long lag )
{
    std::vector< double > i_in((int)m_nodes.size(), 0.);
    construct_matrix(i_in, lag);
}

void nest::CompTree::construct_matrix( const std::vector< double >& i_in, const long lag )
{
    assert( i_in.size() == m_nodes.size() );

    // temporary implementation of current input
    for( size_t ii=0; ii != i_in.size(); ++ii )
    {
        m_nodes[ ii ]->m_ff = i_in[ ii ];
    }

    // TODO avoid recomputing unnessecary terms every time-step
    for( auto node_it = m_nodes.begin(); node_it != m_nodes.end(); ++node_it )
    {
        (*node_it)->construct_matrix_element();
        (*node_it)->add_input_current( lag );
        (*node_it)->add_synapse_contribution(lag);
        (*node_it)->add_channel_contribution();
    }
};

// solve matrix with O(n) algorithm
void nest::CompTree::solve_matrix()
{
    std::vector< CompNode* >::iterator leaf_it = m_leafs.begin();

    // start the down sweep (puts to zero the sub diagonal matrix elements)
    solve_matrix_downsweep(m_leafs[0], leaf_it);

    // do up sweep to set voltages
    solve_matrix_upsweep(&m_root, 0.0);
};

void nest::CompTree::solve_matrix_downsweep( CompNode* node,
					     std::vector< CompNode* >::iterator leaf_it )
{
    // compute the input output transformation at node
    IODat output = node->io();

    // move on to the parent layer
    if( node->m_parent != nullptr )
    {
        CompNode* parent = node->m_parent;
        // gather input from child layers
        parent->gather_input(output);
        // move on to next nodes
        ++parent->m_n_passed;
        if(parent->m_n_passed == int(parent->m_children.size()))
	{
            parent->m_n_passed = 0;
            // move on to next node
            solve_matrix_downsweep(parent, leaf_it);
        }
	else
	{
            // start at next leaf
            ++leaf_it;
            if(leaf_it != m_leafs.end())
	    {
	      solve_matrix_downsweep(*leaf_it, leaf_it);
	    }
        }
    }
};

void nest::CompTree::solve_matrix_upsweep( CompNode* node, double vv )
{
    // compute node voltage
    vv = node->calc_v(vv);
    // move on to child nodes
    for( auto child_it = node->m_children.begin(); child_it != node->m_children.end(); ++child_it )
    {
        solve_matrix_upsweep(&(*child_it), vv);
    }
};

void nest::CompTree::print_tree() const
{
    // loop over all nodes
    std::printf(">>> NEAST tree with %d compartments <<<\n", int(m_nodes.size()));
    for(int ii=0; ii<int(m_nodes.size()); ++ii)
    {
        CompNode* node = m_nodes[ii];
        std::cout << "    Compartment " << node->m_index << ": ";
        std::cout << "C_m = " << node->m_ca << " nF, ";
        std::cout << "g_L = " << node->m_gl << " uS, ";
        std::cout << "e_L = " << node->m_el << " mV, ";
        if(node->m_parent != nullptr)
        {
            std::cout << "Parent " << node->m_parent->m_index << " --> ";
            std::cout << "g_c = " << node->m_gc << " uS, ";
        // std::cout << "Child nodes: " << vec2string(node.m_child_indices) << ", ";
        // std::cout << "Location indices: " << vec2string(node.m_loc_indices) << " ";
        // std::cout << "(new: " << vec2string(node.m_newloc_indices) << ")" << endl;
        }
        std::cout << std::endl;
    }
    // std::cout << "--- leafs ---" << std::endl;
    // for(int ii=0; ii<int(m_leafs.size()); ++ii){
    //     CompNode* node = m_leafs[ii];
    //     std::cout << "    Compartment " << node->m_index << ": ";
    //     std::cout << "C_m = " << node->m_ca << " nF, ";
    //     std::cout << "g_L = " << node->m_gl << " uS, ";
    //     std::cout << "e_L = " << node->m_el << " mV, ";
    //     if(node->m_parent != nullptr)
    //     {
    //         std::cout << "Parent " << node->m_parent->m_index << " --> ";
    //         std::cout << "g_c = " << node->m_gc << " uS, ";
    //     }
        // std::cout << std::endl;
    // }
    std::cout << std::endl;
};
////////////////////////////////////////////////////////////////////////////////
