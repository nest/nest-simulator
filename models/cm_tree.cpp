/*
 *  cm_tree.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "cm_tree.h"

#include "logging.h"
#include "logging_manager.h"


nest::Compartment::Compartment( const long compartment_index, const long parent_index )
  : xx_( 0.0 )
  , yy_( 0.0 )
  , comp_index( compartment_index )
  , p_index( parent_index )
  , parent( nullptr )
  , ca( 1.0 )
  , gc( 0.01 )
  , gl( 0.1 )
  , el( -70. )
  , v_comp( el )
  , gg0( 0.0 )
  , ca__div__dt( 0.0 )
  , gl__div__2( 0.0 )
  , gc__div__2( 0.0 )
  , gl__times__el( 0.0 )
  , ff( 0.0 )
  , gg( 0.0 )
  , hh( 0.0 )
  , n_passed( 0 )
  , compartment_currents( v_comp )
{
  compartment_currents = CompartmentCurrents( v_comp );
}

nest::Compartment::Compartment( const long compartment_index,
  const long parent_index,
  const DictionaryDatum& compartment_params )
  : xx_( 0.0 )
  , yy_( 0.0 )
  , comp_index( compartment_index )
  , p_index( parent_index )
  , parent( nullptr )
  , ca( 1.0 )
  , gc( 0.01 )
  , gl( 0.1 )
  , el( -70. )
  , v_comp( el )
  , gg0( 0.0 )
  , ca__div__dt( 0.0 )
  , gl__div__2( 0.0 )
  , gc__div__2( 0.0 )
  , gl__times__el( 0.0 )
  , ff( 0.0 )
  , gg( 0.0 )
  , hh( 0.0 )
  , n_passed( 0 )
  , compartment_currents( v_comp )
{
  compartment_params->clear_access_flags();

  updateValue< double >( compartment_params, names::C_m, ca );
  updateValue< double >( compartment_params, names::g_C, gc );
  updateValue< double >( compartment_params, names::g_L, gl );
  updateValue< double >( compartment_params, names::e_L, el );
  updateValue< double >( compartment_params, names::v_comp, v_comp );

  compartment_currents = CompartmentCurrents( v_comp, compartment_params );

  ALL_ENTRIES_ACCESSED( *compartment_params, "compartment_params", "Unread dictionary entries: " );
}

void
nest::Compartment::pre_run_hook()
{
  compartment_currents.pre_run_hook();

  const double dt = Time::get_resolution().get_ms();
  ca__div__dt = ca / dt;
  gl__div__2 = gl / 2.;
  gg0 = ca__div__dt + gl__div__2;
  gc__div__2 = gc / 2.;
  gl__times__el = gl * el;

  // initialize the buffer
  currents.clear();
}

std::map< Name, double* >
nest::Compartment::get_recordables()
{
  std::map< Name, double* > recordables = compartment_currents.get_recordables( comp_index );

  recordables.insert( recordables.begin(), recordables.end() );
  recordables[ Name( "v_comp" + std::to_string( comp_index ) ) ] = &v_comp;

  return recordables;
}

// for matrix construction
void
nest::Compartment::construct_matrix_element( const long lag )
{
  // matrix diagonal element
  gg = gg0;

  if ( parent )
  {
    gg += gc__div__2;
    // matrix off diagonal element
    hh = -gc__div__2;
  }

  for ( auto child_it = children.begin(); child_it != children.end(); ++child_it )
  {
    gg += ( *child_it ).gc__div__2;
  }

  // right hand side
  ff = ( ca__div__dt - gl__div__2 ) * v_comp + gl__times__el;

  if ( parent )
  {
    ff -= gc__div__2 * ( v_comp - parent->v_comp );
  }

  for ( auto child_it = children.begin(); child_it != children.end(); ++child_it )
  {
    ff -= ( *child_it ).gc__div__2 * ( v_comp - ( *child_it ).v_comp );
  }

  // add all currents to compartment
  std::pair< double, double > gi = compartment_currents.f_numstep( v_comp, lag );
  gg += gi.first;
  ff += gi.second;

  // add input current
  ff += currents.get_value( lag );
}


nest::CompTree::CompTree()
  : root_( -1, -1 )
  , size_( 0 )
{
  compartments_.resize( 0 );
  leafs_.resize( 0 );
}

/**
 * Add a compartment to the tree structure via the python interface
 * root should have -1 as parent index. Add root compartment first.
 * Assumes parent of compartment is already added
 */
void
nest::CompTree::add_compartment( const long parent_index )
{
  Compartment* compartment = new Compartment( size_, parent_index );
  add_compartment( compartment, parent_index );
}

void
nest::CompTree::add_compartment( const long parent_index, const DictionaryDatum& compartment_params )
{
  Compartment* compartment = new Compartment( size_, parent_index, compartment_params );
  add_compartment( compartment, parent_index );
}

void
nest::CompTree::add_compartment( Compartment* compartment, const long parent_index )
{
  size_++;

  if ( parent_index >= 0 )
  {
    /**
     * we do not raise an UnknownCompartment exception from within
     * get_compartment(), because we want to print a more informative
     * exception message
     */
    Compartment* parent = get_compartment( parent_index, get_root(), 0 );
    if ( not parent )
    {
      std::string msg = "does not exist in tree, but was specified as a parent compartment";
      throw UnknownCompartment( parent_index, msg );
    }

    parent->children.push_back( *compartment );
  }
  else
  {
    // we raise an error if the root already exists
    if ( root_.comp_index >= 0 )
    {
      std::string msg = ", the root, has already been instantiated";
      throw UnknownCompartment( root_.comp_index, msg );
    }
    root_ = *compartment;
  }

  compartment_indices_.push_back( compartment->comp_index );

  set_compartments();
}

/**
 * Get the compartment corresponding to the provided index in the tree.
 *
 * This function gets the compartments by a recursive search through the tree.
 *
 * The overloaded functions looks only in the subtree of the provided compartment,
 * and also has the option to throw an error if no compartment corresponding to
 * `compartment_index` is found in the tree
 */
nest::Compartment*
nest::CompTree::get_compartment( const long compartment_index ) const
{
  return get_compartment( compartment_index, get_root(), 1 );
}

nest::Compartment*
nest::CompTree::get_compartment( const long compartment_index, Compartment* compartment, const long raise_flag ) const
{
  Compartment* r_compartment = nullptr;

  if ( compartment->comp_index == compartment_index )
  {
    r_compartment = compartment;
  }
  else
  {
    auto child_it = compartment->children.begin();
    while ( not r_compartment and child_it != compartment->children.end() )
    {
      r_compartment = get_compartment( compartment_index, &( *child_it ), 0 );
      ++child_it;
    }
  }

  if ( not r_compartment and raise_flag )
  {
    std::string msg = "does not exist in tree";
    throw UnknownCompartment( compartment_index, msg );
  }

  return r_compartment;
}

/**
 * Get the compartment corresponding to the provided index in the tree. Optimized
 * trough the use of a pointer vector containing all compartments. Calling this
 * function before CompTree::init_pointers() is called will result in a segmentation
 * fault
 */
nest::Compartment*
nest::CompTree::get_compartment_opt( const long compartment_idx ) const
{
  return compartments_[ compartment_idx ];
}

/**
 * Initialize all tree structure pointers
 */
void
nest::CompTree::init_pointers()
{
  set_parents();
  set_compartments();
  set_leafs();
}

/**
 * For each compartments, sets its pointer towards its parent compartment
 */
void
nest::CompTree::set_parents()
{
  for ( auto compartment_idx_it = compartment_indices_.begin(); compartment_idx_it != compartment_indices_.end();
        ++compartment_idx_it )
  {
    Compartment* comp_ptr = get_compartment( *compartment_idx_it );
    // will be nullptr if root
    Compartment* parent_ptr = get_compartment( comp_ptr->p_index, &root_, 0 );
    comp_ptr->parent = parent_ptr;
  }
}

/**
 * Creates a vector of compartment pointers, organized in the order in which they were
 * added by `add_compartment()`
 */
void
nest::CompTree::set_compartments()
{
  compartments_.clear();

  for ( auto compartment_idx_it = compartment_indices_.begin(); compartment_idx_it != compartment_indices_.end();
        ++compartment_idx_it )
  {
    compartments_.push_back( get_compartment( *compartment_idx_it ) );
  }
}

/**
 * Creates a vector of compartment pointers of compartments that are also leafs of the tree.
 */
void
nest::CompTree::set_leafs()
{
  leafs_.clear();
  for ( auto compartment_it = compartments_.begin(); compartment_it != compartments_.end(); ++compartment_it )
  {
    if ( int( ( *compartment_it )->children.size() ) == 0 )
    {
      leafs_.push_back( *compartment_it );
    }
  }
}

/**
 * Initializes pointers for the spike buffers for all synapse receptors
 */
void
nest::CompTree::set_syn_buffers( std::vector< RingBuffer >& syn_buffers )
{
  for ( auto compartment_it = compartments_.begin(); compartment_it != compartments_.end(); ++compartment_it )
  {
    ( *compartment_it )->compartment_currents.set_syn_buffers( syn_buffers );
  }
}

/**
 * Returns a map of variable names and pointers to the recordables
 */
std::map< Name, double* >
nest::CompTree::get_recordables()
{
  std::map< Name, double* > recordables;

  /**
   * add recordables for all compartments, suffixed by compartment_idx,
   * to "recordables"
   */
  for ( auto compartment_it = compartments_.begin(); compartment_it != compartments_.end(); ++compartment_it )
  {
    std::map< Name, double* > recordables_comp = ( *compartment_it )->get_recordables();
    recordables.insert( recordables_comp.begin(), recordables_comp.end() );
  }
  return recordables;
}

/**
 * Initialize state variables
 */
void
nest::CompTree::pre_run_hook()
{
  if ( root_.comp_index < 0 )
  {
    std::string msg = "does not exist in tree, meaning that no compartments have been added";
    throw UnknownCompartment( 0, msg );
  }

  // initialize the compartments
  for ( auto compartment_it = compartments_.begin(); compartment_it != compartments_.end(); ++compartment_it )
  {
    ( *compartment_it )->pre_run_hook();
  }
}

/**
 * Returns vector of voltage values, indices correspond to compartments in `compartments_`
 */
std::vector< double >
nest::CompTree::get_voltage() const
{
  std::vector< double > v_comps;
  for ( auto compartment_it = compartments_.cbegin(); compartment_it != compartments_.cend(); ++compartment_it )
  {
    v_comps.push_back( ( *compartment_it )->v_comp );
  }
  return v_comps;
}

/**
 * Return voltage of single compartment voltage, indicated by the compartment_index
 */
double
nest::CompTree::get_compartment_voltage( const long compartment_index )
{
  return compartments_[ compartment_index ]->v_comp;
}

/**
 * Construct the matrix equation to be solved to advance the model one timestep
 */
void
nest::CompTree::construct_matrix( const long lag )
{
  for ( auto compartment_it = compartments_.begin(); compartment_it != compartments_.end(); ++compartment_it )
  {
    ( *compartment_it )->construct_matrix_element( lag );
  }
}

/**
 * Solve matrix with O(n) algorithm
 */
void
nest::CompTree::solve_matrix()
{
  std::vector< Compartment* >::iterator leaf_it = leafs_.begin();

  // start the down sweep (puts to zero the sub diagonal matrix elements)
  solve_matrix_downsweep( leafs_[ 0 ], leaf_it );

  // do up sweep to set voltages
  solve_matrix_upsweep( &root_, 0.0 );
}

void
nest::CompTree::solve_matrix_downsweep( Compartment* compartment, std::vector< Compartment* >::iterator leaf_it )
{
  // compute the input output transformation at compartment
  std::pair< double, double > output = compartment->io();

  // move on to the parent layer
  if ( compartment->parent )
  {
    Compartment* parent = compartment->parent;
    // gather input from child layers
    parent->gather_input( output );
    // move on to next compartments
    ++parent->n_passed;
    if ( parent->n_passed == int( parent->children.size() ) )
    {
      parent->n_passed = 0;
      // move on to next compartment
      solve_matrix_downsweep( parent, leaf_it );
    }
    else
    {
      // start at next leaf
      ++leaf_it;
      if ( leaf_it != leafs_.end() )
      {
        solve_matrix_downsweep( *leaf_it, leaf_it );
      }
    }
  }
}

void
nest::CompTree::solve_matrix_upsweep( Compartment* compartment, double vv )
{
  // compute compartment voltage
  vv = compartment->calc_v( vv );
  // move on to child compartments
  for ( auto child_it = compartment->children.begin(); child_it != compartment->children.end(); ++child_it )
  {
    solve_matrix_upsweep( &( *child_it ), vv );
  }
}

/**
 * Print the tree graph
 */
void
nest::CompTree::print_tree() const
{
  // loop over all compartments
  std::printf( ">>> CM tree with %d compartments <<<\n", int( compartments_.size() ) );
  for ( int ii = 0; ii < int( compartments_.size() ); ++ii )
  {
    Compartment* compartment = compartments_[ ii ];
    std::cout << "    Compartment " << compartment->comp_index << ": ";
    std::cout << "C_m = " << compartment->ca << " nF, ";
    std::cout << "g_L = " << compartment->gl << " uS, ";
    std::cout << "e_L = " << compartment->el << " mV, ";
    if ( compartment->parent )
    {
      std::cout << "Parent " << compartment->parent->comp_index << " --> ";
      std::cout << "g_c = " << compartment->gc << " uS, ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
