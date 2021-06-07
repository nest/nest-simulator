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
#include "enum_bitfield.h"
#include "logging.h"

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

enum class RegisterConnectionModelFlags : unsigned
{
  REGISTER_HPC = 1 << 0,
  REGISTER_LBL = 1 << 1,
  IS_PRIMARY = 1 << 2,
  HAS_DELAY = 1 << 3,
  SUPPORTS_WFR = 1 << 4,
  REQUIRES_SYMMETRIC = 1 << 5,
  REQUIRES_CLOPATH_ARCHIVING = 1 << 6,
  REQUIRES_URBANCZIK_ARCHIVING = 1 << 7
};

template <>
struct EnableBitMaskOperators< RegisterConnectionModelFlags >
{
  static const bool enable = true;
};

const RegisterConnectionModelFlags default_connection_model_flags = RegisterConnectionModelFlags::REGISTER_HPC
  | RegisterConnectionModelFlags::REGISTER_LBL | RegisterConnectionModelFlags::IS_PRIMARY
  | RegisterConnectionModelFlags::HAS_DELAY;

const RegisterConnectionModelFlags default_secondary_connection_model_flags =
  RegisterConnectionModelFlags::SUPPORTS_WFR | RegisterConnectionModelFlags::HAS_DELAY;

/**
 * Register connection model (i.e. an instance of a class inheriting from `Connection`).
 */
template < template < typename > class ConnectorModelT >
void register_connection_model( const std::string& name,
  const RegisterConnectionModelFlags flags = default_connection_model_flags );

/**
 * Register secondary connection models (e.g. gap junctions, rate-based models).
 */
template < template < typename > class ConnectorModelT >
void register_secondary_connection_model( const std::string& name,
  const RegisterConnectionModelFlags flags = default_secondary_connection_model_flags );

void print_nodes_to_stream( std::ostream& out = std::cout );

RngPtr get_rank_synced_rng();
RngPtr get_vp_synced_rng( thread tid );
RngPtr get_vp_specific_rng( thread tid );

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
  const std::vector< DictionaryDatum >& synapse_params );

/**
 * @brief Connect arrays of node IDs one-to-one
 *
 * Connects an array of sources to an array of targets, with weights and
 * delays from specified arrays, using the one-to-one
 * rule. Additional synapse parameters can be specified with p_keys and p_values.
 * Sources, targets, weights, delays, and receptor types are given
 * as pointers to the first element. All arrays must have the same length,
 * n. Weights, delays, and receptor types can be unspecified by passing a
 * nullptr.
 *
 * The p_keys vector contains keys of additional synapse parameters, with
 * associated values in the flat array p_values. If there are n sources and targets,
 * and M additional synapse parameters, p_keys has a size of M, and the p_values array
 * has length of M*n.
 */
void connect_arrays( long* sources,
  long* targets,
  double* weights,
  double* delays,
  std::vector< std::string >& p_keys,
  double* p_values,
  size_t n,
  std::string syn_model );

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

Datum* node_collection_array_index( const Datum* datum, const long* array, unsigned long n );
Datum* node_collection_array_index( const Datum* datum, const bool* array, unsigned long n );
}


#endif /* NEST_H */
