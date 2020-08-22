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

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "node_collection.h"
#include "growth_curve_factory.h"
#include "nest_time.h"
#include "nest_types.h"

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
 * Otherwise it behaves as the normal ConnectionManager.
 * @param
 */
class SPManager : public ManagerInterface
{

public:
  SPManager();
  virtual ~SPManager();

  virtual void initialize();
  virtual void finalize();

  virtual void get_status( DictionaryDatum& );
  virtual void set_status( const DictionaryDatum& );

  DictionaryDatum& get_growthcurvedict();

  /**
   * Create a new Growth Curve object using the GrowthCurve Factory
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
   * Disconnect two collections of nodes.  The connection is
   * established on the thread/process that owns the target node.
   *
   * \param sources Node collection of the source Nodes.
   * \param targets Node collection of the target Nodes.
   * \param connectivityParams connectivity Dictionary
   * \param synapseParams synapse parameters Dictionary
   */
  void disconnect( NodeCollectionPTR, NodeCollectionPTR, DictionaryDatum&, DictionaryDatum& );

  /**
   * Disconnect two nodes.
   * The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * \param snode_id node ID of the sending Node.
   * \param target Pointer to target Node.
   * \param target_thread Thread that hosts the target node.
   * \param syn_id The synapse model to use.
   */
  void disconnect( const index snode_id, Node* target, thread target_thread, const index syn_id );

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
   * This influences the min_delay of the kernel, as the connections
   * are build during the simulation. Hence, the
   * ConnectionManager::min_delay() methods have to respect this delay
   * as well.
   */
  delay builder_min_delay() const;

  /**
   * Returns the maximum delay of all SP builders.
   * This influences the max_delay of the kernel, as the connections
   * are build during the simulation. Hence, the
   * ConnectionManager::max_delay() methods have to respect this delay
   * as well.
   */
  delay builder_max_delay() const;

  // Creation of synapses
  void create_synapses( std::vector< index >& pre_vacant_id,
    std::vector< int >& pre_vacant_n,
    std::vector< index >& post_vacant_id,
    std::vector< int >& post_vacant_n,
    SPBuilder* sp_conn_builder );
  // Deletion of synapses on the pre synaptic side
  void delete_synapses_from_pre( const std::vector< index >& pre_deleted_id,
    std::vector< int >& pre_deleted_n,
    const index synapse_model,
    const std::string& se_pre_name,
    const std::string& se_post_name );
  // Deletion of synapses on the post synaptic side
  void delete_synapses_from_post( std::vector< index >& post_deleted_id,
    std::vector< int >& post_deleted_n,
    index synapse_model,
    std::string se_pre_name,
    std::string se_post_name );
  // Deletion of synapses
  void delete_synapse( index source, index target, long syn_id, std::string se_pre_name, std::string se_post_name );

  void get_synaptic_elements( std::string se_name,
    std::vector< index >& se_vacant_id,
    std::vector< int >& se_vacant_n,
    std::vector< index >& se_deleted_id,
    std::vector< int >& se_deleted_n );

  void serialize_id( std::vector< index >& id, std::vector< int >& n, std::vector< index >& res );
  void global_shuffle( std::vector< index >& v );
  void global_shuffle( std::vector< index >& v, size_t n );

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

  /** @BeginDocumentation

   Name: growthcurvedict - growth curves for Model of Structural Plasticity

   Description:
   This dictionary provides indexes for the growth curve factory
   */
  DictionaryDatum growthcurvedict_; //!< Dictionary for growth rules.

  /**
   * GrowthCurve factories, indexed by growthcurvedict_ elements.
   */
  std::vector< GenericGrowthCurveFactory* > growthcurve_factories_;
};

inline DictionaryDatum&
SPManager::get_growthcurvedict()
{
  return growthcurvedict_;
}

inline GrowthCurve*
SPManager::new_growth_curve( Name name )
{
  const long nc_id = ( *growthcurvedict_ )[ name ];
  return growthcurve_factories_.at( nc_id )->create();
}

inline bool
SPManager::is_structural_plasticity_enabled() const
{
  return structural_plasticity_enabled_;
}

inline double
SPManager::get_structural_plasticity_update_interval() const
{
  return structural_plasticity_update_interval_;
}
}
#endif /* SP_MANAGER_H */
