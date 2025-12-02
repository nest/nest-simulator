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
nest::StructuralPlasticityNode::get_status( Dictionary& d ) const
{

  d[ names::Ca ] = Ca_minus_;
  d[ names::tau_Ca ] = tau_Ca_;
  d[ names::beta_Ca ] = beta_Ca_;

  Dictionary synaptic_elements_d;
  for ( std::map< std::string, SynapticElement >::const_iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    Dictionary synaptic_element_d;
    it->second.get( synaptic_element_d );
    synaptic_elements_d[ it->first ] = synaptic_element_d;
  }
  d[ names::synaptic_elements ] = synaptic_elements_d;
}

void
nest::StructuralPlasticityNode::set_status( const Dictionary& d )
{
  // We need to preserve values in case invalid values are set
  double new_Ca_ = Ca_minus_;
  double new_tau_Ca = tau_Ca_;
  double new_beta_Ca = beta_Ca_;
  d.update_value( names::Ca, new_Ca_ );
  d.update_value( names::tau_Ca, new_tau_Ca );
  d.update_value( names::beta_Ca, new_beta_Ca );

  if ( new_Ca_ < 0.0 )
  {
    throw BadProperty( "Calcium concentration cannot be negative." );
  }
  Ca_minus_ = new_Ca_;

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
  d.update_value( names::clear, clear );
  if ( clear )
  {
    clear_history();
  }

  if ( d.known( names::synaptic_elements_param ) )
  {
    const Dictionary synaptic_elements_dict = d.get< Dictionary >( names::synaptic_elements_param );

    for ( std::map< std::string, SynapticElement >::iterator it = synaptic_elements_map_.begin();
          it != synaptic_elements_map_.end();
          ++it )
    {
      if ( synaptic_elements_dict.known( it->first ) )
      {
        const Dictionary synaptic_elements_a = synaptic_elements_dict.get< Dictionary >( it->first );
        it->second.set( synaptic_elements_a );
      }
    }
  }
  if ( not d.known( names::synaptic_elements ) )
  {
    return;
  }
  // we replace the existing synaptic_elements_map_ by the new one
  Dictionary synaptic_elements_d;
  std::pair< std::map< std::string, SynapticElement >::iterator, bool > insert_result;

  synaptic_elements_map_ = std::map< std::string, SynapticElement >();
  synaptic_elements_d = d.get< Dictionary >( names::synaptic_elements );

  for ( auto& syn_element : synaptic_elements_d )
  {
    SynapticElement se;
    se.set( synaptic_elements_d.get< Dictionary >( syn_element.first ) );
    synaptic_elements_map_.insert( std::pair< std::string, SynapticElement >( syn_element.first, se ) );
  }
}

void
nest::StructuralPlasticityNode::clear_history()
{
  Ca_minus_ = 0.0;
  Ca_t_ = 0.0;
}

double
nest::StructuralPlasticityNode::get_synaptic_elements( std::string n ) const
{
  std::map< std::string, SynapticElement >::const_iterator se_it;
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
nest::StructuralPlasticityNode::get_synaptic_elements_vacant( std::string n ) const
{
  std::map< std::string, SynapticElement >::const_iterator se_it;
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
nest::StructuralPlasticityNode::get_synaptic_elements_connected( std::string n ) const
{
  std::map< std::string, SynapticElement >::const_iterator se_it;
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

std::map< std::string, double >
nest::StructuralPlasticityNode::get_synaptic_elements() const
{
  std::map< std::string, double > n_map;

  for ( std::map< std::string, SynapticElement >::const_iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    n_map.insert( std::pair< std::string, double >( it->first, get_synaptic_elements( it->first ) ) );
  }
  return n_map;
}

void
nest::StructuralPlasticityNode::update_synaptic_elements( double t )
{
  assert( t >= Ca_t_ );

  for ( std::map< std::string, SynapticElement >::iterator it = synaptic_elements_map_.begin();
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
  for ( std::map< std::string, SynapticElement >::iterator it = synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    it->second.decay_z_vacant();
  }
}

void
nest::StructuralPlasticityNode::connect_synaptic_element( std::string name, int n )
{
  std::map< std::string, SynapticElement >::iterator se_it;
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
