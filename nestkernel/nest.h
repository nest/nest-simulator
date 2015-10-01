/*
 *  nest.h
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

#ifndef NEST_H
#define NEST_H

#include <ostream>

#include "nest_types.h"
#include "nest_time.h"
#include "nest_datums.h"
#include "randomgen.h"
#include "dictdatum.h"
#include "arraydatum.h"
#include "logging.h"

namespace nest
{

void Init( int argc, char* argv[] );
void FailExit( int exitcode );

void Install( const std::string& module_name );

void ResetKernel();
void ResetNetwork();

void EnableDryrunMode( const index n_procs );

void RegisterLoggerClient( const deliver_logging_event_ptr client_callback );
void PrintNetwork( index gid, index depth, std::ostream& out = std::cout );

librandom::RngPtr GetVpRNG( index target );
librandom::RngPtr GetGlobalRNG();

void SetKernelStatus( const DictionaryDatum& dict );
DictionaryDatum GetKernelStatus();

void SetNodeStatus( const index node_id, const DictionaryDatum& dict );
DictionaryDatum GetNodeStatus( const index node_id );

void SetConnectionStatus( const ConnectionDatum& conn, const DictionaryDatum& dict );
DictionaryDatum GetConnectionStatus( const ConnectionDatum& conn );

index Create( const Name& model_name, const index n );

void Connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& connectivity,
  const DictionaryDatum& synapse_params );
ArrayDatum GetConnections( const DictionaryDatum& dict );

void Simulate( const double_t& t );
void ResumeSimulation();

void CopyModel( const Name& oldmodname, const Name& newmodname, const DictionaryDatum& dict );

void SetModelDefaults( const Name& model_name, const DictionaryDatum& );
DictionaryDatum GetModelDefaults( const Name& model_name );

void SetNumRecProcesses( const index n_rec_procs );

void ChangeSubnet( const index node_gid );
index CurrentSubnet();

ArrayDatum GetNodes( const index subnet_id,
  const DictionaryDatum& params,
  const bool include_remotes,
  const bool return_gids_only );
ArrayDatum
GetLeaves( const index subnet_id, const DictionaryDatum& params, const bool include_remotes );
ArrayDatum
GetChildren( const index subnet_id, const DictionaryDatum& params, const bool include_remotes );

void RestoreNodes( const ArrayDatum& node_list );
}


#endif /* NEST_H */
