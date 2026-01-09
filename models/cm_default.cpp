/*
 *  cm_default.cpp
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

#include "cm_default.h"

namespace nest
{
void
register_cm_default( const std::string& name )
{
  register_node_model< cm_default >( name );
}


/*
 * For some reason this code block is needed. However, I have found no
 * difference in calling init_recordable_pointers() from the pre_run_hook function,
 * except that an unused-variable warning is generated in the code-checks
 */
template <>
void
DynamicRecordablesMap< cm_default >::create( cm_default& host )
{
  host.init_recordables_pointers_();
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::cm_default::cm_default()
  : ArchivingNode()
  , c_tree_()
  , syn_buffers_( 0 )
  , logger_( *this )
  , V_th_( -55.0 )
{
  recordablesMap_.create( *this );
  recordables_values.resize( 0 );
}

nest::cm_default::cm_default( const cm_default& n )
  : ArchivingNode( n )
  , c_tree_( n.c_tree_ )
  , syn_buffers_( n.syn_buffers_ )
  , logger_( *this )
  , V_th_( n.V_th_ )
{
  recordables_values.resize( 0 );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ----------------------------------------------------------------
 */
void
cm_default::get_status( DictionaryDatum& statusdict ) const
{
  def< double >( statusdict, names::V_th, V_th_ );
  ArchivingNode::get_status( statusdict );

  // add all recordables to the status dictionary
  ( *statusdict )[ names::recordables ] = recordablesMap_.get_list();

  // We add a list of dicts with compartment information and
  // a list of dicts with receptor information to the status dictionary
  ArrayDatum compartment_ad;
  ArrayDatum receptor_ad;
  for ( long comp_idx_ = 0; comp_idx_ != c_tree_.get_size(); comp_idx_++ )
  {
    DictionaryDatum dd = DictionaryDatum( new Dictionary );
    Compartment* compartment = c_tree_.get_compartment( comp_idx_ );

    // add compartment info
    def< long >( dd, names::comp_idx, comp_idx_ );
    def< long >( dd, names::parent_idx, compartment->p_index );
    compartment_ad.push_back( dd );

    // add receptor info
    compartment->compartment_currents.add_receptor_info( receptor_ad, compartment->comp_index );
  }
  // add compartment info and receptor info to the status dictionary
  def< ArrayDatum >( statusdict, names::compartments, compartment_ad );
  def< ArrayDatum >( statusdict, names::receptors, receptor_ad );
}

void
nest::cm_default::set_status( const DictionaryDatum& statusdict )
{
  updateValue< double >( statusdict, names::V_th, V_th_ );
  ArchivingNode::set_status( statusdict );

  /**
   * Add a compartment (or compartments) to the tree, so that the new compartment
   * has the compartment specified by "parent_idx" as parent. The parent
   * has to be in the tree, otherwise an error will be raised.  We add either a
   * single compartment or multiple compartments, depending on whether the
   * entry was a list of dicts or a single dict
   */
  const auto add_compartments_list_or_dict = [ this, statusdict ]( const Name name )
  {
    Datum* dat = ( *statusdict )[ name ].datum();
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( dat );
    DictionaryDatum* dd = dynamic_cast< DictionaryDatum* >( dat );

    if ( ad )
    {
      // A list of compartments is provided, we add them all to the tree
      for ( Token* tt = ( *ad ).begin(); tt != ( *ad ).end(); ++tt )
      {
        // cast the Datum pointer stored within token dynamically to a
        // DictionaryDatum pointer
        add_compartment_( *dynamic_cast< DictionaryDatum* >( tt->datum() ) );
      }
    }
    else if ( dd )
    {
      // A single compartment is provided, we add add it to the tree
      add_compartment_( *dd );
    }
    else
    {
      throw BadProperty(
        "\'compartments\' entry could not be identified, provide "
        "list of parameter dicts for multiple compartments" );
    }
  };

  /**
   * Add a receptor (or receptors) to the tree, so that the new receptor
   * targets the compartment specified by "comp_idx". The compartment
   * has to be in the tree, otherwise an error will be raised.  We add either a
   * single receptor or multiple receptors, depending on whether the
   * entry was a list of dicts or a single dict
   */
  const auto add_receptors_list_or_dict = [ this, statusdict ]( const Name name )
  {
    Datum* dat = ( *statusdict )[ name ].datum();
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( dat );
    DictionaryDatum* dd = dynamic_cast< DictionaryDatum* >( dat );

    if ( ad )
    {
      for ( Token* tt = ( *ad ).begin(); tt != ( *ad ).end(); ++tt )
      {
        // cast the Datum pointer stored within token dynamically to a
        // DictionaryDatum pointer
        add_receptor_( *dynamic_cast< DictionaryDatum* >( tt->datum() ) );
      }
    }
    else if ( dd )
    {
      add_receptor_( *dd );
    }
    else
    {
      throw BadProperty(
        "\'receptors\' entry could not be identified, provide "
        "list of parameter dicts for multiple receptors" );
    }
  };

  if ( statusdict->known( names::compartments ) )
  {
    // Compartments can only be set on a newly created compartment model.
    // To add additional compartments, add_compartments should be used.
    if ( c_tree_.get_size() > 0 )
    {
      throw BadProperty( "\'compartments\' is already defined for this model" );
    }
    add_compartments_list_or_dict( names::compartments );
  }

  if ( statusdict->known( names::add_compartments ) )
  {
    add_compartments_list_or_dict( names::add_compartments );
  }

  if ( statusdict->known( names::receptors ) )
  {
    // Receptors can only be set on a newly created compartment model.
    // To add additional receptors, add_receptors should be used.
    if ( syn_buffers_.size() > 0 )
    {
      throw BadProperty( "\'receptors\' is already defined for this model" );
    }
    add_receptors_list_or_dict( names::receptors );
  }
  if ( statusdict->known( names::add_receptors ) )
  {
    add_receptors_list_or_dict( names::add_receptors );
  }

  /**
   * we need to initialize the recordables pointers to guarantee that the
   * recordables of the new compartments and/or receptors will be in the
   * recordables map
   */
  init_recordables_pointers_();
}
void
nest::cm_default::add_compartment_( DictionaryDatum& dd )
{
  dd->clear_access_flags();

  if ( dd->known( names::params ) )
  {
    c_tree_.add_compartment(
      getValue< long >( dd, names::parent_idx ), getValue< DictionaryDatum >( dd, names::params ) );
  }
  else
  {
    c_tree_.add_compartment( getValue< long >( dd, names::parent_idx ) );
  }

  ALL_ENTRIES_ACCESSED( *dd, "cm_default::add_compartment_", "Unread dictionary entries: " );
}
void
nest::cm_default::add_receptor_( DictionaryDatum& dd )
{
  dd->clear_access_flags();

  const long compartment_idx = getValue< long >( dd, names::comp_idx );
  const std::string receptor_type = getValue< std::string >( dd, names::receptor_type );

  // create a ringbuffer to collect spikes for the receptor
  RingBuffer buffer;

  // add the ringbuffer to the global receptor vector
  const size_t syn_idx = syn_buffers_.size();
  syn_buffers_.push_back( buffer );

  // add the receptor to the compartment
  Compartment* compartment = c_tree_.get_compartment( compartment_idx );
  if ( dd->known( names::params ) )
  {
    compartment->compartment_currents.add_synapse(
      receptor_type, syn_idx, getValue< DictionaryDatum >( dd, names::params ) );
  }
  else
  {
    compartment->compartment_currents.add_synapse( receptor_type, syn_idx );
  }

  ALL_ENTRIES_ACCESSED( *dd, "cm_default::add_receptor_", "Unread dictionary entries: " );
}

void
nest::cm_default::init_recordables_pointers_()
{
  /**
   * Get the map of all recordables (i.e. all state variables of the model):
   * --> keys are state variable names suffixed by the compartment index for
   *     voltage (e.g. "v_comp1") or by the synapse index for receptor currents
   * --> values are pointers to the specific state variables
   */
  std::map< Name, double* > recordables = c_tree_.get_recordables();

  for ( auto rec_it = recordables.begin(); rec_it != recordables.end(); rec_it++ )
  {
    // check if name is already in recordables map
    auto recname_it = find( recordables_names.begin(), recordables_names.end(), rec_it->first );
    if ( recname_it == recordables_names.end() )
    {
      // recordable name is not yet in map, we need to add it
      recordables_names.push_back( rec_it->first );
      recordables_values.push_back( rec_it->second );
      const long rec_idx = recordables_values.size() - 1;
      // add the recordable to the recordable_name -> recordable_index map
      recordablesMap_.insert( rec_it->first, DataAccessFunctor< cm_default >( *this, rec_idx ) );
    }
    else
    {
      // recordable name is in map, we update the pointer to the recordable
      long index = recname_it - recordables_names.begin();
      recordables_values[ index ] = rec_it->second;
    }
  }
}

void
nest::cm_default::pre_run_hook()
{
  logger_.init();

  // initialize the pointers within the compartment tree
  c_tree_.init_pointers();
  // initialize the pointers to the synapse buffers for the receptor currents
  c_tree_.set_syn_buffers( syn_buffers_ );
  // initialize the recordables pointers
  init_recordables_pointers_();

  c_tree_.pre_run_hook();
}

/**
 * Update and spike handling functions
 */
void
nest::cm_default::update( Time const& origin, const long from, const long to )
{
  for ( long lag = from; lag < to; ++lag )
  {
    const double v_0_prev = c_tree_.get_root()->v_comp;

    c_tree_.construct_matrix( lag );
    c_tree_.solve_matrix();

    // threshold crossing
    if ( c_tree_.get_root()->v_comp >= V_th_ and v_0_prev < V_th_ )
    {
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel::manager< EventDeliveryManager >.send( *this, se, lag );
    }

    logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::cm_default::handle( SpikeEvent& e )
{
  if ( e.get_weight() < 0 )
  {
    throw BadProperty( "Synaptic weights must be positive." );
  }

  assert( e.get_delay_steps() > 0 );
  assert( e.get_rport() < syn_buffers_.size() );

  syn_buffers_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::cm_default::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  Compartment* compartment = c_tree_.get_compartment_opt( e.get_rport() );
  compartment->currents.add_value(
    e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ), w * c );
}

void
nest::cm_default::handle( DataLoggingRequest& e )
{
  logger_.handle( e );
}

} // namespace
