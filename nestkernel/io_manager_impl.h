/*
 *  io_manager_impl.h
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

#ifndef IO_MANAGER_IMPL_H
#define IO_MANAGER_IMPL_H

#include "io_manager.h"

namespace nest
{

template < class RecordingBackendT >
void
IOManager::register_recording_backend( const Name name )
{
  if ( recording_backends_.find( name ) != recording_backends_.end() )
  {
    throw BackendAlreadyRegistered( name.toString() );
  }

  RecordingBackendT* recording_backend = new RecordingBackendT();
  recording_backend->pre_run_hook();
  recording_backends_.insert( std::make_pair( name, recording_backend ) );
}

template < class StimulationBackendT >
void
IOManager::register_stimulation_backend( const Name name )
{
  if ( stimulation_backends_.find( name ) != stimulation_backends_.end() )
  {
    throw BackendAlreadyRegistered( name.toString() );
  }

  StimulationBackendT* stimulation_backend = new StimulationBackendT();
  stimulation_backend->pre_run_hook();
  stimulation_backends_.insert( std::make_pair( name, stimulation_backend ) );
}

} // namespace nest

#endif /* #ifndef IO_MANAGER_IMPL_H */
