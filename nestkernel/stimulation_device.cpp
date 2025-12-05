/*
 *  stimulation_device.cpp
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


// Includes from nestkernel:
#include "stimulation_device.h"
#include "kernel_manager.h"


nest::StimulationDevice::StimulationDevice()
  : DeviceNode()
  , Device()
  , first_syn_id_( invalid_synindex )
  , backend_params_()
{
}

nest::StimulationDevice::StimulationDevice( StimulationDevice const& sd )
  : DeviceNode( sd )
  , Device( sd )
  , P_( sd.P_ )
  , first_syn_id_( invalid_synindex ) // a new instance can't have any connections
  , backend_params_( sd.backend_params_ )
{
}

bool
nest::StimulationDevice::is_active( const Time& T ) const
{
  long step = T.get_steps();
  if ( get_type() == StimulationDevice::Type::CURRENT_GENERATOR
    or get_type() == StimulationDevice::Type::DELAYED_RATE_CONNECTION_GENERATOR
    or get_type() == StimulationDevice::Type::DOUBLE_DATA_GENERATOR )
  {
    step = step + 2;
  }
  return get_t_min_() < step and step <= get_t_max_();
}

void
nest::StimulationDevice::enforce_single_syn_type( synindex syn_id )
{
  if ( first_syn_id_ == invalid_synindex )
  {
    first_syn_id_ = syn_id;
  }
  if ( syn_id != first_syn_id_ )
  {
    throw IllegalConnection( "All outgoing connections from a device must use the same synapse type." );
  }
}

void
nest::StimulationDevice::pre_run_hook()
{
  Device::pre_run_hook();
}

void
nest::StimulationDevice::set_initialized_()
{
  kernel().io_manager.enroll_stimulator( P_.stimulus_source_, *this, backend_params_ );
}

const std::string&
nest::StimulationDevice::get_label() const
{
  return P_.label_;
}


nest::StimulationDevice::Parameters_::Parameters_()
  : label_()
  , stimulus_source_()
{
}

void
nest::StimulationDevice::Parameters_::get( Dictionary& d ) const
{
  d[ names::label ] = label_;
  d[ names::stimulus_source ] = stimulus_source_;
}

void
nest::StimulationDevice::Parameters_::set( const Dictionary& d )
{
  d.update_value( names::label, label_ );

  std::string stimulus_source;
  if ( d.update_value( names::stimulus_source, stimulus_source ) )
  {

    if ( not kernel().io_manager.is_valid_stimulation_backend( stimulus_source ) )
    {
      std::string msg = std::format( "Unknown input backend '{}'", stimulus_source );
      throw BadProperty( msg );
    }
    stimulus_source_ = stimulus_source;
  }
}

void
nest::StimulationDevice::set_status( const Dictionary& d )
{

  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  Device::set_status( d );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    Dictionary backend_params;

    // copy all properties not previously accessed from d to backend_params
    for ( auto& [ key, entry ] : d )
    {
      if ( not d.has_been_accessed( key ) )
      {
        backend_params[ key ] = entry.item;
      }
    }

    // cache all properties accessed by the backend in private member
    backend_params_.clear();
    for ( auto& [ key, entry ] : backend_params )
    {
      if ( backend_params.has_been_accessed( key ) )
      {
        backend_params_[ key ] = entry.item;
        d.mark_as_accessed( key );
      }
    }
  }
  else
  {
    kernel().io_manager.enroll_stimulator( ptmp.stimulus_source_, *this, d );
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}


void
nest::StimulationDevice::get_status( Dictionary& d ) const
{
  P_.get( d );

  Device::get_status( d );

  d[ names::element_type ] = names::stimulator;

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    // overwrite with cached parameters
    for ( const auto& [ param_name, param_value ] : backend_params_ )
    {
      d[ param_name ] = param_value;
    }
  }
}
