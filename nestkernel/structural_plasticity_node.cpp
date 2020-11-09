/*
 *  structural_plasticity_node.cpp
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

#include "structural_plasticity_node.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

nest::StructuralPlasticityNode::StructuralPlasticityNode()
  : Ca_t_( 0.0 )
  , Ca_minus_( 0.0 )
  , tau_Ca_( 10000.0 )
  , beta_Ca_( 0.001 )
  , synaptic_elements_map_()
{
}

nest::StructuralPlasticityNode::StructuralPlasticityNode( const StructuralPlasticityNode& n )
  : Node( n )
  , Ca_t_( n.Ca_t_ )
  , Ca_minus_( n.Ca_minus_ )
  , tau_Ca_( n.tau_Ca_ )
  , beta_Ca_( n.beta_Ca_ )
  , synaptic_elements_map_( n.synaptic_elements_map_ )
{
}

void
nest::StructuralPlasticityNode::get_status( DictionaryDatum& d ) const
{
  DictionaryDatum synaptic_elements_d;
  DictionaryDatum synaptic_element_d;

  def< double >( d, names::Ca, Ca_minus_ );
  def< double >( d, names::tau_Ca, tau_Ca_ );
  def< double >( d, names::beta_Ca, beta_Ca_ );

  synaptic_elements_d = DictionaryDatum( new Dictionary );
  def< DictionaryDatum >( d, names::synaptic_elements, synaptic_elements_d );
  for ( std::map< Name, SynapticElement >::const_iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    synaptic_element_d = DictionaryDatum( new Dictionary );
    def< DictionaryDatum >( synaptic_elements_d, it->first, synaptic_element_d );
    it->second.get( synaptic_element_d );
  }
}

void
nest::StructuralPlasticityNode::set_status( const DictionaryDatum& d )
{
  // We need to preserve values in case invalid values are set
  double new_tau_Ca = tau_Ca_;
  double new_beta_Ca = beta_Ca_;
  updateValue< double >( d, names::tau_Ca, new_tau_Ca );
  updateValue< double >( d, names::beta_Ca, new_beta_Ca );

  if ( new_tau_Ca <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  tau_Ca_ = new_tau_Ca;

  if ( new_beta_Ca <= 0.0 )
  {
    throw BadProperty(
      "For Ca to function as an integrator of the electrical activity, beta_ca "
      "needs to be greater than 0." );
  }
  beta_Ca_ = new_beta_Ca;

  // check, if to clear spike history and K_minus
  bool clear = false;
  updateValue< bool >( d, names::clear, clear );
  if ( clear )
  {
    clear_history();
  }

  if ( d->known( names::synaptic_elements_param ) )
  {
    const DictionaryDatum synaptic_elements_dict = getValue< DictionaryDatum >( d, names::synaptic_elements_param );

    for ( std::map< Name, SynapticElement >::iterator it = synaptic_elements_map_.begin();
          it != synaptic_elements_map_.end();
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

  synaptic_elements_map_ = std::map< Name, SynapticElement >();
  synaptic_elements_d = getValue< DictionaryDatum >( d, names::synaptic_elements );

  for ( Dictionary::const_iterator i = synaptic_elements_d->begin(); i != synaptic_elements_d->end(); ++i )
  {
    insert_result = synaptic_elements_map_.insert( std::pair< Name, SynapticElement >( i->first, SynapticElement() ) );
    ( insert_result.first->second ).set( getValue< DictionaryDatum >( synaptic_elements_d, i->first ) );
  }
}

void
nest::StructuralPlasticityNode::clear_history()
{
  Ca_minus_ = 0.0;
  Ca_t_ = 0.0;
}

double
nest::StructuralPlasticityNode::get_synaptic_elements( Name n ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.find( n );
  double z_value;

  if ( se_it != synaptic_elements_map_.end() )
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
nest::StructuralPlasticityNode::get_synaptic_elements_vacant( Name n ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.find( n );

  if ( se_it != synaptic_elements_map_.end() )
  {
    return se_it->second.get_z_vacant();
  }
  else
  {
    return 0;
  }
}

int
nest::StructuralPlasticityNode::get_synaptic_elements_connected( Name n ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.find( n );

  if ( se_it != synaptic_elements_map_.end() )
  {
    return se_it->second.get_z_connected();
  }
  else
  {
    return 0;
  }
}

std::map< Name, double >
nest::StructuralPlasticityNode::get_synaptic_elements() const
{
  std::map< Name, double > n_map;

  for ( std::map< Name, SynapticElement >::const_iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    n_map.insert( std::pair< Name, double >( it->first, get_synaptic_elements( it->first ) ) );
  }
  return n_map;
}

void
nest::StructuralPlasticityNode::update_synaptic_elements( double t )
{
  assert( t >= Ca_t_ );

  for ( std::map< Name, SynapticElement >::iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    it->second.update( t, Ca_t_, Ca_minus_, tau_Ca_ );
  }
  // Update calcium concentration
  Ca_minus_ = Ca_minus_ * std::exp( ( Ca_t_ - t ) / tau_Ca_ );
  Ca_t_ = t;
}

void
nest::StructuralPlasticityNode::decay_synaptic_elements_vacant()
{
  for ( std::map< Name, SynapticElement >::iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    it->second.decay_z_vacant();
  }
}

void
nest::StructuralPlasticityNode::connect_synaptic_element( Name name, int n )
{
  std::map< Name, SynapticElement >::iterator se_it;
  se_it = synaptic_elements_map_.find( name );

  if ( se_it != synaptic_elements_map_.end() )
  {
    se_it->second.connect( n );
  }
}

void
nest::StructuralPlasticityNode::set_spiketime( Time const& t_sp, double offset )
{
  const double t_sp_ms = t_sp.get_ms() - offset;
  update_synaptic_elements( t_sp_ms );
  Ca_minus_ += beta_Ca_;
}

} // of namespace nest
