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

#ifndef SP_MANAGER_H
#define SP_MANAGER_H

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "growth_curve_factory.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node_collection.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictdatum.h"

namespace nest
{
class Node;

class SPBuilder;

/**
 * The SPManager class is in charge of managing the dynamic creation and
 * deletion of synapses in the simulation when structural plasticity is enabled.
 *
 * Otherwise it behaves as the normal ConnectionManager.
 * @param
 */
class SPManager : public ManagerInterface
{

public:
  SPManager();
  ~SPManager() override;

  void initialize( const bool ) override;
  void finalize( const bool ) override;

  void get_status( DictionaryDatum& ) override;
  /**
   * Set status of synaptic plasticity variables: synaptic update interval,
   * synapses and synaptic elements.
   *
   * @param d Dictionary containing the values to be set
   */
  void set_status( const DictionaryDatum& ) override;

  /**
   * Create a new Growth Curve object using the GrowthCurve Factory
   *
   * @param name which defines the type of NC to be created
   * @return a new Growth Curve object of the type indicated by name
   */
  GrowthCurve* new_growth_curve( Name name );

  /**
   * Add a growth curve for MSP
   */
  template < typename GrowthCurve >
  void register_growth_curve( const std::string& name );

  /**
   * Disconnect two collections of nodes.
   *
   * The connection is established on the thread/process that owns the target node.
   *
   * Obtains the right connection builder and performs a synapse deletion
   * according to the specified connection specs.
   *
   * \param sources Node collection of the source Nodes.
   * \param targets Node collection of the target Nodes.
   * \param connectivity Params connectivity Dictionary
   * \param synapse Params synapse parameters Dictionary
   *  conn_spec disconnection specs. For now only all to all and one to one
   * rules are implemented.
   */
  void disconnect( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    DictionaryDatum& conn_spec,
    DictionaryDatum& syn_spec );

  /**
   * Disconnect two nodes.
   *
   * The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * \param snode_id node ID of the sending Node.
   * \param target Pointer to target Node.
   * \param target_thread Thread that hosts the target node.
   * \param syn_id The synapse model to use.
   */
  void disconnect( const size_t snode_id, Node* target, size_t target_thread, const size_t syn_id );

  void update_structural_plasticity();
  void update_structural_plasticity( SPBuilder* );

  /**
   * Enable structural plasticity
   */
  void enable_structural_plasticity();

  /**
   * Disable structural plasticity
   */
  void disable_structural_plasticity();

  bool is_structural_plasticity_enabled() const;

  double get_structural_plasticity_update_interval() const;

  /**
   * Returns the minimum delay of all SP builders.
   *
   * This influences the min_delay of the kernel, as the connections
   * are build during the simulation. Hence, the
   * ConnectionManager::min_delay() methods have to respect this delay
   * as well.
   */
  long builder_min_delay() const;

  /**
   * Returns the maximum delay of all SP builders.
   *
   * This influences the max_delay of the kernel, as the connections
   * are build during the simulation. Hence, the
   * ConnectionManager::max_delay() methods have to respect this delay
   * as well.
   */
  long builder_max_delay() const;

  // Creation of synapses
  bool create_synapses( std::vector< size_t >& pre_vacant_id,
    std::vector< int >& pre_vacant_n,
    std::vector< size_t >& post_vacant_id,
    std::vector< int >& post_vacant_n,
    SPBuilder* sp_conn_builder );
  // Deletion of synapses on the pre synaptic side
  void delete_synapses_from_pre( const std::vector< size_t >& pre_deleted_id,
    std::vector< int >& pre_deleted_n,
    const size_t synapse_model,
    const std::string& se_pre_name,
    const std::string& se_post_name );
  // Deletion of synapses on the postsynaptic side
  void delete_synapses_from_post( std::vector< size_t >& post_deleted_id,
    std::vector< int >& post_deleted_n,
    size_t synapse_model,
    std::string se_pre_name,
    std::string se_post_name );
  // Deletion of synapses
  void delete_synapse( size_t source, size_t target, long syn_id, std::string se_pre_name, std::string se_post_name );

  void get_synaptic_elements( std::string se_name,
    std::vector< size_t >& se_vacant_id,
    std::vector< int >& se_vacant_n,
    std::vector< size_t >& se_deleted_id,
    std::vector< int >& se_deleted_n );

  void serialize_id( std::vector< size_t >& id, std::vector< int >& n, std::vector< size_t >& res );
  void global_shuffle( std::vector< size_t >& v );
  void global_shuffle( std::vector< size_t >& v, size_t n );

private:
  /**
   * Time interval for structural plasticity update (creation/deletion of
   * synapses).
   */
  double structural_plasticity_update_interval_;

  /**
   * Indicates whether the Structrual Plasticity functionality is On (True) of
   * Off (False).
   */
  bool structural_plasticity_enabled_;
  std::vector< SPBuilder* > sp_conn_builders_;

  /**
   * GrowthCurve factories, indexed by growthcurvedict_ elements.
   */
  std::vector< GenericGrowthCurveFactory* > growthcurve_factories_;

  DictionaryDatum growthcurvedict_; //!< Dictionary for growth rules.
};


template < typename GrowthCurve >
void
SPManager::register_growth_curve( const std::string& name )
{
  assert( not growthcurvedict_->known( name ) );
  GenericGrowthCurveFactory* nc = new GrowthCurveFactory< GrowthCurve >();
  assert( nc );
  const int id = growthcurve_factories_.size();
  growthcurve_factories_.push_back( nc );
  growthcurvedict_->insert( name, id );
}

} // namespace nest

#endif /* #ifndef SP_MANAGER_H */
