#include "cm_tree.h"


// compartment compartment functions ///////////////////////////////////////////
nest::Compartment::Compartment( const long compartment_index,
                                const long parent_index )
  : m_xx( 0.0 )
  , m_yy( 0.0 )
  , m_index( compartment_index )
  , m_p_index( parent_index )
  , m_parent( nullptr )
  , m_v( 0.0 )
  , m_ca( 0.0)
  , m_gc( 0.0)
  , m_gl( 0.0 )
  , m_el( 0.0 )
  , m_ff( 0.0 )
  , m_gg( 0.0 )
  , m_hh( 0.0 )
  , m_n_passed( 0 )
{
  m_syns.resize( 0 );
  m_etype = EType();
};
nest::Compartment::Compartment( const long compartment_index,
                                const long parent_index,
			                    const DictionaryDatum& compartment_params )
  : m_xx( 0.0 )
  , m_yy( 0.0 )
  , m_index( compartment_index )
  , m_p_index( parent_index )
  , m_parent( nullptr )
  , m_v( 0.0 )
  , m_ca( getValue< double >( compartment_params, "C_m" ) )
  , m_gc( getValue< double >( compartment_params, "g_c" ) )
  , m_gl( getValue< double >( compartment_params, "g_L" ) )
  , m_el( getValue< double >( compartment_params, "E_L" ) )
  , m_ff( 0.0 )
  , m_gg( 0.0 )
  , m_hh( 0.0 )
  , m_n_passed( 0 )
{
  m_syns.resize( 0 );
  m_etype = EType( compartment_params );
};

void nest::Compartment::init()
{
    m_v = m_el;

    for( auto  syn_it = m_syns.begin(); syn_it != m_syns.end(); ++syn_it )
    {
        (*syn_it)->init();
    }

    // initialize the buffer
    m_currents.clear();
}

// for matrix construction
void nest::Compartment::construct_matrix_element( const long lag )
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

    // add the channel contribution
    std::pair< double, double > gf_chan = m_etype.f_numstep(m_v, dt);
    m_gg += gf_chan.first;
    m_ff += gf_chan.second;

    // add synapse contribution
    std::pair< double, double > gf_syn(0., 0.);

    for( auto syn_it = m_syns.begin(); syn_it != m_syns.end(); ++syn_it )
    {
        (*syn_it)->update(lag);
        gf_syn = (*syn_it)->f_numstep(m_v);

        m_gg += gf_syn.first;
        m_ff += gf_syn.second;
    }

    // add input current
    m_ff += m_currents.get_value( lag );
}
////////////////////////////////////////////////////////////////////////////////


// compartment tree functions //////////////////////////////////////////////////
nest::CompTree::CompTree()
  : m_root( 0, -1)
{
  m_compartments.resize( 0 );
  m_leafs.resize( 0 );
}

/*
Add a compartment to the tree structure via the python interface
root shoud have -1 as parent index. Add root compartment first.
Assumes parent of compartment is already added
*/
void nest::CompTree::add_compartment( const long compartment_index,
                                      const long parent_index,
			                          const DictionaryDatum& compartment_params)
{
    Compartment* compartment = new Compartment( compartment_index, parent_index,
                				   compartment_params);

    if( parent_index >= 0 )
    {
        Compartment* parent = find_compartment( parent_index );
        parent->m_children.push_back( *compartment );
    }
    else
    {
        m_root = *compartment;
    }

    m_compartment_indices.push_back(compartment_index);

    set_compartments();
};

/*
Find the compartment corresponding to the provided index in the tree.

The overloaded functions looks only in the subtree of the provided compartment.
It is only used for recursion.
*/
nest::Compartment* nest::CompTree::find_compartment( const long compartment_index )
{
    return find_compartment( compartment_index, get_root(), 1 );
}
nest::Compartment* nest::CompTree::find_compartment( const long compartment_index, Compartment* compartment,
                                           const long raise_flag)
{
    Compartment* r_compartment = nullptr;

    if( compartment->m_index == compartment_index )
    {
        r_compartment = compartment;
    }
    else
    {
        auto child_it = compartment->m_children.begin();
        while( !r_compartment && child_it != compartment->m_children.end() )
        {
            r_compartment = find_compartment( compartment_index, &(*child_it), 0 );
            ++child_it;
        }
    }

    if( !r_compartment && raise_flag )
    {
        std::ostringstream err_msg;
        err_msg << "Node index " << compartment_index << " not in tree";
        throw BadProperty(err_msg.str());
    }

    return r_compartment;
}

// initialization functions
void nest::CompTree::init()
{
    set_compartments();
    set_leafs();

    // initialize the compartments
    for( auto compartment_it = m_compartments.begin();
         compartment_it != m_compartments.end();
         ++compartment_it )
    {
        ( *compartment_it )->m_parent = find_compartment( ( *compartment_it )->m_p_index, &m_root, 0);
        ( *compartment_it )->init();
    }
}

/*
Creates a vector of compartment pointers, organized in the order in which they were
added by `add_compartment()`
*/
void nest::CompTree::set_compartments()
{
    m_compartments.clear();

    for( auto compartment_idx_it = m_compartment_indices.begin();
         compartment_idx_it != m_compartment_indices.end();
         ++compartment_idx_it )
    {
        m_compartments.push_back( find_compartment( *compartment_idx_it ) );
    }

}

/*
Creates a vector of compartment pointers of compartments that are also leafs of the tree.
*/
void nest::CompTree::set_leafs()
{
    m_leafs.clear();
    for( auto compartment_it = m_compartments.begin();
         compartment_it != m_compartments.end();
         ++compartment_it )
    {
        if( int((*compartment_it)->m_children.size()) == 0 )
	{
            m_leafs.push_back( *compartment_it );
        }
    }
};

/*
Returns vector of voltage values, indices correspond to compartments in `m_compartments`
*/
std::vector< double > nest::CompTree::get_voltage() const
{
    std::vector< double > v_comp;
    for( auto compartment_it = m_compartments.cbegin();
         compartment_it != m_compartments.cend();
         ++compartment_it )
    {
        v_comp.push_back( (*compartment_it)->m_v );
    }
    return v_comp;
}

/*
Return voltage of single compartment voltage, indicated by the compartment_index
*/
double nest::CompTree::get_compartment_voltage( const long compartment_index )
{
    const Compartment* compartment = find_compartment( compartment_index );
    return compartment->m_v;
}

/*
Construct the matrix equation to be solved to advance the model one timestep
*/
void nest::CompTree::construct_matrix( const long lag )
{
    std::vector< double > i_in((int)m_compartments.size(), 0.);
    construct_matrix(i_in, lag);
}
void nest::CompTree::construct_matrix( const std::vector< double >& i_in, const long lag )
{
    assert( i_in.size() == m_compartments.size() );

    // temporary implementation of current input
    for( size_t ii=0; ii != i_in.size(); ++ii )
    {
        m_compartments[ ii ]->m_ff = i_in[ ii ];
    }

    // TODO avoid recomputing unnessecary terms every time-step
    for( auto compartment_it = m_compartments.begin();
         compartment_it != m_compartments.end();
         ++compartment_it )
    {
        (*compartment_it)->construct_matrix_element( lag );
    }
};

/*
Solve matrix with O(n) algorithm
*/
void nest::CompTree::solve_matrix()
{
    std::vector< Compartment* >::iterator leaf_it = m_leafs.begin();

    // start the down sweep (puts to zero the sub diagonal matrix elements)
    solve_matrix_downsweep(m_leafs[0], leaf_it);

    // do up sweep to set voltages
    solve_matrix_upsweep(&m_root, 0.0);
};
void nest::CompTree::solve_matrix_downsweep( Compartment* compartment,
					     std::vector< Compartment* >::iterator leaf_it )
{
    // compute the input output transformation at compartment
    std::pair< double, double > output = compartment->io();

    // move on to the parent layer
    if( compartment->m_parent != nullptr )
    {
        Compartment* parent = compartment->m_parent;
        // gather input from child layers
        parent->gather_input(output);
        // move on to next compartments
        ++parent->m_n_passed;
        if(parent->m_n_passed == int(parent->m_children.size()))
	{
            parent->m_n_passed = 0;
            // move on to next compartment
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
void nest::CompTree::solve_matrix_upsweep( Compartment* compartment, double vv )
{
    // compute compartment voltage
    vv = compartment->calc_v(vv);
    // move on to child compartments
    for( auto child_it = compartment->m_children.begin(); child_it != compartment->m_children.end(); ++child_it )
    {
        solve_matrix_upsweep(&(*child_it), vv);
    }
};

/*
Print the tree graph
*/
void nest::CompTree::print_tree() const
{
    // loop over all compartments
    std::printf(">>> CM tree with %d compartments <<<\n", int(m_compartments.size()));
    for(int ii=0; ii<int(m_compartments.size()); ++ii)
    {
        Compartment* compartment = m_compartments[ii];
        std::cout << "    Compartment " << compartment->m_index << ": ";
        std::cout << "C_m = " << compartment->m_ca << " nF, ";
        std::cout << "g_L = " << compartment->m_gl << " uS, ";
        std::cout << "e_L = " << compartment->m_el << " mV, ";
        if(compartment->m_parent != nullptr)
        {
            std::cout << "Parent " << compartment->m_parent->m_index << " --> ";
            std::cout << "g_c = " << compartment->m_gc << " uS, ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
};
////////////////////////////////////////////////////////////////////////////////
