/*
 *  structural_plasticity_vector.cpp
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

#include "structural_plasticity_vector.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "synaptic_element.h"

// Includes from sli:
#include "dictutils.h"


namespace nest
{
nest::StructuralPlasticityVector::StructuralPlasticityVector()
  : Ca_t_( 0 )
  , Ca_minus_( 0 )
  , tau_Ca_( 0 )
  , beta_Ca_( 0 )
  , synaptic_elements_map_( 0 )
{
}


void
StructuralPlasticityVector::resize( index extended_space, index thread_id )
{
  index total_space = size();

  Ca_minus_.resize( total_space, 0 );
  Ca_t_.resize( total_space, 0 );
  tau_Ca_.resize( total_space, 10000.0 );
  beta_Ca_.resize( total_space, 0.001 );
  synaptic_elements_map_.resize( total_space, std::map< Name, SynapticElement >() );

  VectorizedNode::resize( extended_space, thread_id );
}
void
nest::StructuralPlasticityVector::get_status( DictionaryDatum& d, index local_id ) const
{
  DictionaryDatum synaptic_elements_d;
  DictionaryDatum synaptic_element_d;

  def< double >( d, names::Ca, Ca_minus_.at( local_id ) );
  def< double >( d, names::tau_Ca, tau_Ca_.at( local_id ) );
  def< double >( d, names::beta_Ca, beta_Ca_.at( local_id ) );

  synaptic_elements_d = DictionaryDatum( new Dictionary );
  def< DictionaryDatum >( d, names::synaptic_elements, synaptic_elements_d );
  for ( std::map< Name, SynapticElement >::const_iterator it = synaptic_elements_map_.at( local_id ).begin();
        it != synaptic_elements_map_.at( local_id ).end();
        ++it )
  {
    synaptic_element_d = DictionaryDatum( new Dictionary );
    def< DictionaryDatum >( synaptic_elements_d, it->first, synaptic_element_d );
    it->second.get( synaptic_element_d );
  }
}
void
nest::StructuralPlasticityVector::set_status( const DictionaryDatum& d, index local_id )
{
  // We need to preserve values in case invalid values are set
  double new_tau_Ca = tau_Ca_.at( local_id );
  double new_beta_Ca = beta_Ca_.at( local_id );
  updateValue< double >( d, names::tau_Ca, new_tau_Ca );
  updateValue< double >( d, names::beta_Ca, new_beta_Ca );

  if ( new_tau_Ca <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  tau_Ca_.at( local_id ) = new_tau_Ca;

  if ( new_beta_Ca <= 0.0 )
  {
    throw BadProperty(
      "For Ca to function as an integrator of the electrical activity, beta_ca "
      "needs to be greater than 0." );
  }
  beta_Ca_.at( local_id ) = new_beta_Ca;

  // check, if to clear spike history and K_minus
  bool clear = false;
  updateValue< bool >( d, names::clear, clear );
  if ( clear )
  {
    clear_history( local_id );
  }

  if ( d->known( names::synaptic_elements_param ) )
  {
    const DictionaryDatum synaptic_elements_dict = getValue< DictionaryDatum >( d, names::synaptic_elements_param );

    for ( std::map< Name, SynapticElement >::iterator it = synaptic_elements_map_.at( local_id ).begin();
          it != synaptic_elements_map_.at( local_id ).end();
          ++it )
    {
      if ( synaptic_elements_dict->known( it->first ) )
      {
        const DictionaryDatum synaptic_elements_a = getValue< DictionaryDatum >( synaptic_elements_dict, it->first );
        it->second.set( synaptic_elements_a );
      }
    }
  }
  if ( not d->known( names::synaptic_elements ) )
  {
    return;
  }
  // we replace the existing synaptic_elements_map_ by the new one
  DictionaryDatum synaptic_elements_d;
  std::pair< std::map< Name, SynapticElement >::iterator, bool > insert_result;

  synaptic_elements_map_.at( local_id ) = std::map< Name, SynapticElement >();
  synaptic_elements_d = getValue< DictionaryDatum >( d, names::synaptic_elements );

  for ( Dictionary::const_iterator i = synaptic_elements_d->begin(); i != synaptic_elements_d->end(); ++i )
  {
    insert_result =
      synaptic_elements_map_.at( local_id ).insert( std::pair< Name, SynapticElement >( i->first, SynapticElement() ) );
    ( insert_result.first->second ).set( getValue< DictionaryDatum >( synaptic_elements_d, i->first ) );
  }
}
void
nest::StructuralPlasticityVector::clear_history( index local_id )
{
  Ca_minus_.at( local_id ) = 0.0;
  Ca_t_.at( local_id ) = 0.0;
}

double
nest::StructuralPlasticityVector::get_synaptic_elements( Name n, index local_id ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.at( local_id ).find( n );
  double z_value;

  if ( se_it != synaptic_elements_map_.at( local_id ).end() )
  {
    z_value = ( se_it->second ).get_z();
    if ( ( se_it->second ).continuous() )
    {
      return z_value;
    }
    else
    {
      return std::floor( z_value );
    }
  }
  else
  {
    return 0.0;
  }
}
int
nest::StructuralPlasticityVector::get_synaptic_elements_vacant( Name n, index local_id ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.at( local_id ).find( n );

  if ( se_it != synaptic_elements_map_.at( local_id ).end() )
  {
    return se_it->second.get_z_vacant();
  }
  else
  {
    return 0;
  }
}
int
nest::StructuralPlasticityVector::get_synaptic_elements_connected( Name n, index local_id ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.at( local_id ).find( n );

  if ( se_it != synaptic_elements_map_.at( local_id ).end() )
  {
    return se_it->second.get_z_connected();
  }
  else
  {
    return 0;
  }
}
std::map< Name, double >
nest::StructuralPlasticityVector::get_synaptic_elements( index local_id ) const
{
  std::map< Name, double > n_map;

  for ( std::map< Name, SynapticElement >::const_iterator it = synaptic_elements_map_.at( local_id ).begin();
        it != synaptic_elements_map_.at( local_id ).end();
        ++it )
  {
    n_map.insert( std::pair< Name, double >( it->first, get_synaptic_elements( it->first, local_id ) ) );
  }
  return n_map;
}
void
nest::StructuralPlasticityVector::update_synaptic_elements( double t, index local_id )
{
  assert( t >= Ca_t_.at( local_id ) );

  for ( std::map< Name, SynapticElement >::iterator it = synaptic_elements_map_.at( local_id ).begin();
        it != synaptic_elements_map_.at( local_id ).end();
        ++it )
  {
    it->second.update( t, Ca_t_.at( local_id ), Ca_minus_.at( local_id ), tau_Ca_.at( local_id ) );
  }
  // Update calcium concentration
  Ca_minus_.at( local_id ) =
    Ca_minus_.at( local_id ) * std::exp( ( Ca_t_.at( local_id ) - t ) / tau_Ca_.at( local_id ) );
  Ca_t_.at( local_id ) = t;
}
void
nest::StructuralPlasticityVector::decay_synaptic_elements_vacant( index local_id )
{
  for ( std::map< Name, SynapticElement >::iterator it = synaptic_elements_map_.at( local_id ).begin();
        it != synaptic_elements_map_.at( local_id ).end();
        ++it )
  {
    it->second.decay_z_vacant();
  }
}
void
nest::StructuralPlasticityVector::connect_synaptic_element( Name name, int n, index local_id )
{
  std::map< Name, SynapticElement >::iterator se_it;
  se_it = synaptic_elements_map_.at( local_id ).find( name );

  if ( se_it != synaptic_elements_map_.at( local_id ).end() )
  {
    se_it->second.connect( n );
  }
}
void
nest::StructuralPlasticityVector::set_spiketime( const Time& t_sp, index local_id, double offset )
{
  const double t_sp_ms = t_sp.get_ms() - offset;
  update_synaptic_elements( t_sp_ms, local_id );
  Ca_minus_.at( local_id ) += beta_Ca_.at( local_id );
}
}