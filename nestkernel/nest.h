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

// C++ includes:
#include <ostream>

// Includes from libnestutil:
#include "logging.h"

// Includes from librandom:
#include "randomgen.h"

// Includes from nestkernel:
#include "nest_datums.h"
#include "nest_time.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"

namespace nest
{

void init_nest( int* argc, char** argv[] );
void fail_exit( int exitcode );

void install_module( const std::string& module_name );

void reset_kernel();

void enable_dryrun_mode( const index n_procs );

void register_logger_client( const deliver_logging_event_ptr client_callback );
void print_nodes_to_stream( std::ostream& out = std::cout );

librandom::RngPtr get_vp_rng( thread tid );
librandom::RngPtr get_global_rng();

void set_kernel_status( const DictionaryDatum& dict );
DictionaryDatum get_kernel_status();

void set_node_status( const index node_id, const DictionaryDatum& dict );
DictionaryDatum get_node_status( const index node_id );

void set_connection_status( const ConnectionDatum& conn, const DictionaryDatum& dict );
DictionaryDatum get_connection_status( const ConnectionDatum& conn );

NodeCollectionPTR create( const Name& model_name, const index n );

NodeCollectionPTR get_nodes( const DictionaryDatum& dict, const bool local_only );

void connect( NodeCollectionPTR sources,
  NodeCollectionPTR targets,
  const DictionaryDatum& connectivity,
  const DictionaryDatum& synapse_params );

ArrayDatum get_connections( const DictionaryDatum& dict );

void simulate( const double& t );

/**
 * @fn run(const double& time)
 * @brief Run a partial simulation for `time` ms
 *
 * Runs a partial simulation for `time` ms
 * after a call to prepare() and before a cleanup().
 * Can be called multiple times between a prepare()/cleanup()
 * pair to divide a simulation into multiple pieces with access
 * to the API in between.
 *
 * Thus, simulate(t) = prepare(); run(t/2); run(t/2); cleanup()
 *
 * @see prepare()
 * @see cleanup()
 */
void run( const double& t );

/**
 * @fn prepare()
 * @brief do calibrations for network, open files, ... before run()
 *
 * Prepares a simulation before calling any number of run(t_n)
 * calls to actually run the simulation
 *
 * @see run()
 * @see cleanup()
 */
void prepare();

/**
 * @fn cleanup()
 * @brief do cleanup after a simulation, such as closing files
 *
 * Do cleanup to end a simulation using run() commands.
 * After calling cleanup(), further run() calls must only happen
 * after another call to prepare()
 *
 * @see run()
 * @see prepare()
 */
void cleanup();

void copy_model( const Name& oldmodname, const Name& newmodname, const DictionaryDatum& dict );

void set_model_defaults( const Name& model_name, const DictionaryDatum& );
DictionaryDatum get_model_defaults( const Name& model_name );

ParameterDatum multiply_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum divide_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum add_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum subtract_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum
compare_parameter( const ParameterDatum& param1, const ParameterDatum& param2, const DictionaryDatum& d );
ParameterDatum
conditional_parameter( const ParameterDatum& param1, const ParameterDatum& param2, const ParameterDatum& param3 );
ParameterDatum min_parameter( const ParameterDatum& param, const double other_value );
ParameterDatum max_parameter( const ParameterDatum& param, const double other_value );
ParameterDatum redraw_parameter( const ParameterDatum& param, const double min, const double max );
ParameterDatum exp_parameter( const ParameterDatum& param );
ParameterDatum sin_parameter( const ParameterDatum& param );
ParameterDatum cos_parameter( const ParameterDatum& param );
ParameterDatum pow_parameter( const ParameterDatum& param, const double exponent );
ParameterDatum dimension_parameter( const ParameterDatum& param_x, const ParameterDatum& param_y );
ParameterDatum
dimension_parameter( const ParameterDatum& param_x, const ParameterDatum& param_y, const ParameterDatum& param_z );
ParameterDatum create_parameter( const DictionaryDatum& param_dict );
double get_value( const ParameterDatum& param );
bool is_spatial( const ParameterDatum& param );
std::vector< double > apply( const ParameterDatum& param, const NodeCollectionDatum& nc );
std::vector< double > apply( const ParameterDatum& param, const DictionaryDatum& positions );
}


#endif /* NEST_H */
