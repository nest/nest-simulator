/*
 *  sp_manager.h
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

/*
 * File:   sp_updater.h
 * Author: naveau
 *
 * Created on November 26, 2013, 2:28 PM
 */

#ifndef SP_MANAGER_H
#define SP_MANAGER_H

#include "connection_manager.h"

namespace nest
{
class SPBuilder;

/**
 * The SPManager class is in charge of managing the dynamic creation and deletion
 * of synapses in the simulation when structural plasticity is enabled. Otherwise
 * it behaves as the normal ConnectionManager.
 * @param
 */
class SPManager : public ConnectionManager
{

public:
  SPManager( Network& );
  virtual ~SPManager();

  void reset();

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  void get_structural_plasticity_status( DictionaryDatum& ) const;
  void set_structural_plasticity_status( const DictionaryDatum& );

  const Time get_min_delay() const;
  const Time get_max_delay() const;

  void update_structural_plasticity();
  void update_structural_plasticity( SPBuilder* );

  // Creation of synapses
  void create_synapses( std::vector< index >& pre_vacant_id,
    std::vector< int_t >& pre_vacant_n,
    std::vector< index >& post_vacant_id,
    std::vector< int_t >& post_vacant_n,
    SPBuilder* sp_conn_builder );
  // Deletion of synapses on the pre synaptic side
  void delete_synapses_from_pre( std::vector< index >& pre_deleted_id,
    std::vector< int_t >& pre_deleted_n,
    index synapse_model,
    std::string se_pre_name,
    std::string se_post_name );
  // Deletion of synapses on the post synaptic side
  void delete_synapses_from_post( std::vector< index >& post_deleted_id,
    std::vector< int_t >& post_deleted_n,
    index synapse_model,
    std::string se_pre_name,
    std::string se_post_name );
  // Deletion of synapses
  void delete_synapse( index source,
    index target,
    long syn_id,
    std::string se_pre_name,
    std::string se_post_name );

  void get_synaptic_elements( std::string se_name,
    std::vector< index >& se_vacant_id,
    std::vector< int_t >& se_vacant_n,
    std::vector< index >& se_deleted_id,
    std::vector< int_t >& se_deleted_n );

  void get_sources( std::vector< index > targets,
    std::vector< std::vector< index > >& sources,
    index synapse_model );
  void get_targets( std::vector< index > sources,
    std::vector< std::vector< index > >& targets,
    index synapse_model );
  void serialize_id( std::vector< index >& id, std::vector< int_t >& n, std::vector< index >& res );
  void global_shuffle( std::vector< index >& v );
  void global_shuffle( std::vector< index >& v, size_t n );

  // Time interval for structural plasticity update (creation/deletion of synapses)
  static long_t structural_plasticity_update_interval;
  std::vector< SPBuilder* > sp_conn_builders;

private:
};
}
#endif /* SP_MANAGER_H */
