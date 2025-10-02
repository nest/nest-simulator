/*
 *  conn_builder.h
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

#ifndef CONN_BUILDER_H
#define CONN_BUILDER_H

/**
 * Class managing flexible connection creation.
 *
 * Created based on the connection_creator used for spatial networks.
 *
 */

// C++ includes:
#include <map>
#include <set>
#include <vector>

// Includes from libnestutil
#include "block_vector.h"

// Includes from nestkernel:
// #include "conn_parameter.h"
// #include "kernel_manager.h"
#include "node_collection.h"
#include "parameter.h"
// #include "sp_manager.h"

// Includes from sli:
#include "dictdatum.h"
#include "nest_datums.h"
#include "sliexceptions.h"

namespace nest
{
class Node;
class ConnParameter;
class SparseNodeArray;
class BipartiteConnBuilder;
class ThirdInBuilder;
class ThirdOutBuilder;


/**
 * Abstract base class for Bipartite ConnBuilders which form the components of ConnBuilder.
 *
 * The base class extracts and holds parameters and provides
 * the connect interface. Derived classes implement the connect
 * method.
 *
 * @note This class is also the base class for all components of a TripartiteConnBuilder.
 */
class BipartiteConnBuilder
{
public:
  //! Connect with or without structural plasticity
  virtual void connect();

  //! Delete synapses with or without structural plasticity
  virtual void disconnect();

  /**
   * Create new bipartite builder.
   *
   * @param sources Source population to connect from
   * @param targets Target population to connect to
   * @param third_out `nullptr` if pure bipartite connection, pointer to \class ThirdOutBuilder object if this builder
   * creates the primary connection of a tripartite connectivity
   * @param conn_spec Connection specification (if part of tripartite, spec for the specific part)
   * @param syn_specs Collection of synapse specifications (usually single element, several for collocated synapses)
   */
  BipartiteConnBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdOutBuilder* third_out,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );
  virtual ~BipartiteConnBuilder();

  size_t get_synapse_model() const;

  bool get_default_delay() const;

  void set_synaptic_element_names( const std::string& pre_name, const std::string& post_name );

  /**
   * Updates the number of connected synaptic elements in the target and the source.
   *
   * @param snode_id The node ID of the source
   * @param tnode_id The node ID of the target
   * @param tid the thread of the target
   * @param update Amount of connected synaptic elements to update
   * @return A bool indicating if the target node is on the local thread/process or not
   */
  bool change_connected_synaptic_elements( size_t snode_id, size_t tnode_id, const size_t tid, int update );

  //! Return true if rule allows creation of symmetric connectivity
  virtual bool supports_symmetric() const;

  //! Return true if rule automatically creates symmetric connectivity
  virtual bool is_symmetric() const;

  bool allows_autapses() const;

  bool allows_multapses() const;

  //! Return true if rule is applicable only to nodes with proxies
  virtual bool requires_proxies() const;

protected:
  //! Implements the actual connection algorithm
  virtual void connect_() = 0;

  bool all_parameters_scalar_() const;

  virtual void sp_connect_();

  virtual void disconnect_();

  virtual void sp_disconnect_();

  void update_param_dict_( size_t snode_id, Node& target, size_t target_thread, RngPtr rng, size_t indx );

  //! Create connection between given nodes, fill parameter values
  void single_connect_( size_t, Node&, size_t, RngPtr );
  void single_disconnect_( size_t snode_id, Node& target, size_t target_thread );

  /**
   * Moves pointer in parameter array.
   *
   * Calls value-function of all parameters being instantiations of
   * ArrayDoubleParameter or ArrayIntegerParameter, thus moving the pointer
   * to the next parameter value. The function is called when the target
   * node is not located on the current thread or MPI-process and read of an
   * array.
   */
  void skip_conn_parameter_( size_t target_thread, size_t n_skip = 1 );
  /**
   * Returns true if conventional looping over targets is indicated.
   *
   * Conventional looping over targets must be used if
   * - any connection parameter requires skipping
   * - targets are not given as a simple range (lookup otherwise too slow)
   *
   * Conventional looping should be used if
   * - the number of targets is smaller than the number of local nodes
   *
   * For background, see Ippen et al (2017).
   *
   * @return true if conventional looping is to be used
   */
  bool loop_over_targets_() const;

  NodeCollectionPTR sources_; //!< Population to connect from
  NodeCollectionPTR targets_; //!< Population to connect to

  ThirdOutBuilder* third_out_; //!< To be triggered when primary connection is created

  bool allow_autapses_;
  bool allow_multapses_;
  bool make_symmetric_;
  bool creates_symmetric_connections_;

  //! Buffer for exceptions raised in threads
  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_;

  // Name of the pre synaptic and postsynaptic elements for this connection builder
  std::string pre_synaptic_element_name_;
  std::string post_synaptic_element_name_;

  bool use_structural_plasticity_;

  //! Pointers to connection parameters specified as arrays
  std::vector< ConnParameter* > parameters_requiring_skipping_;

  std::vector< size_t > synapse_model_id_;

  /**
   * Dictionaries to pass to connect function, one per thread for every syn_spec
   *
   * Outer dim: syn_spec, inner dim: thread
   *
   * @note Each thread can independently modify its dictionary to pass parameters on
   */
  std::vector< std::vector< DictionaryDatum > > param_dicts_;

private:
  typedef std::map< Name, ConnParameter* > ConnParameterMap;

  //! indicate that weight and delay should not be set per synapse
  std::vector< bool > default_weight_and_delay_;

  //! indicate that weight should not be set per synapse
  std::vector< bool > default_weight_;

  //! indicate that delay should not be set per synapse
  std::vector< bool > default_delay_;

  // null-pointer indicates that default be used
  std::vector< ConnParameter* > weights_;
  std::vector< ConnParameter* > delays_;

  //! all other parameters, mapping name to value representation
  std::vector< ConnParameterMap > synapse_params_;

  //! synapse-specific parameters that should be skipped when we set default synapse parameters
  std::set< Name > skip_syn_params_;

  /**
   * Collects all array parameters in a vector.
   *
   * If the inserted parameter is an array it will be added to a vector of
   * ConnParameters. This vector will be exploited in some connection
   * routines to ensuring thread-safety.
   */
  void register_parameters_requiring_skipping_( ConnParameter& param );

  /**
   * Set synapse specific parameters.
   */
  void set_synapse_model_( DictionaryDatum syn_params, size_t indx );
  void set_default_weight_or_delay_( DictionaryDatum syn_params, size_t indx );
  void set_synapse_params( DictionaryDatum syn_defaults, DictionaryDatum syn_params, size_t indx );

  /**
   * Set structural plasticity parameters (if provided)
   *
   * This function first checks if any of the given syn_specs contains
   * one of the structural plasticity parameters pre_synaptic_element
   * or post_synaptic_element. If that is the case and only a single
   * syn_spec is given, the parameters are copied to the variables
   * pre_synaptic_element_name_ and post_synaptic_element_name_, and
   * the flag use_structural_plasticity_ is set to true.
   *
   * An exception is thrown if either
   * * only one of the structural plasticity parameter is given
   * * multiple syn_specs are given and structural plasticity parameters
   *   are present
   */
  void set_structural_plasticity_parameters( std::vector< DictionaryDatum > syn_specs );

  /**
   * Reset weight and delay pointers
   */
  void reset_weights_();
  void reset_delays_();
};


/**
 * Builder creating "thírd in" connections based on data from "third out" builder.
 *
 * This builder creates the actual connections from primary sources to third-factor nodes
 * based on the source-third lists generated by the third-out builder.
 *
 * The `ThirdOutBuilder::third_connect()` method calls `register_connection()`
 * for each source node to third-factor node connection that needs to be created to store this
 * information in the `ThirdIn` builder.
 *
 * The `connect_()` method of this class needs to be called after all primary connections
 * and third-factor to target connections have been created. It then exchanges information
 * about required source-third connections with the other MPI ranks and creates required
 * connections locally.
 *
 * The class is final because there is no freedom of choice of connection rule at this stage.
 */
class ThirdInBuilder final : public BipartiteConnBuilder
{
public:
  /**
   * Create ThirdInBuilder
   *
   * @param sources Source population of primary connection
   * @param third Third-factor population
   * @param third_conn_spec is ignored by this builder but required to make base class happy
   * @param syn_specs Collection of synapse specification for connection from primary source to third factor
   *
   * @todo Once DictionaryDatums are gone, see if we can remove `third_conn_spec` and just pass empty conn spec
   * container to base-class constructor, since \class ThirdInBuilder has no connection rule properties to set.
   */
  ThirdInBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR third,
    const DictionaryDatum& third_conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );
  ~ThirdInBuilder();

  /**
   * Register required source node to third-factor node connection.
   *
   * @param primary_source_id GID of source node to connect from
   * @param third_node_id GID of target node to connect to
   */
  void register_connection( size_t primary_source_id, size_t third_node_id );

private:
  //! Exchange required connection info via MPI and create needed connections locally
  void connect_() override;

  /**
   * Provide information on connections from primary source to third-factor population.
   *
   * Data is written by ThirdOutBuilder and communicated and processed by ThirdInBuilder.
   */
  struct SourceThirdInfo_
  {
    SourceThirdInfo_()
      : source_gid( 0 )
      , third_gid( 0 )
      , third_rank( 0 )
    {
    }
    SourceThirdInfo_( size_t src, size_t trd, size_t rank )
      : source_gid( src )
      , third_gid( trd )
      , third_rank( rank )
    {
    }

    size_t source_gid; //!< GID of source node to connect from
    size_t third_gid;  //!< GID of third-factor node to connect to
    size_t third_rank; //!< Rank of third-factor node (stored locally as it is needed multiple times)
  };

  //! Container for register source-third connections, one per thread via pointer for collision free operation in
  //! thread-local storage
  std::vector< BlockVector< SourceThirdInfo_ >* > source_third_gids_;

  //! Number of source-third pairs to send. Outer dimension is writing thread, inner dimension MPI rank to send to
  std::vector< std::vector< size_t >* > source_third_counts_;
};


/**
 * Builder for connections from third-factor nodes to primary target populations.
 *
 * This builder creates connections from third-factor nodes to primary targets based on the
 * third-factor connection rule. It also registers source-third-factor pairs with the
 * corresponding third-in ConnBuilder for later instantiation.
 *
 * This class needs to be subclassed for each third-factor connection rule.
 */
class ThirdOutBuilder : public BipartiteConnBuilder
{
public:
  /**
   * Create ThirdOutBuilder.
   *
   * @param third Third-factor population
   * @param targets Target population of primary connection
   * @param third_in ThirdInBuilder which will create source-third connections later
   * @param third_conn_spec Specification for third-factor connectivity
   * @param syn_specs Collection of synapse specifications for third-target connections
   */
  ThirdOutBuilder( const NodeCollectionPTR third,
    const NodeCollectionPTR targets,
    ThirdInBuilder* third_in,
    const DictionaryDatum& third_conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  //! Only call third_connect() on ThirdOutBuilder
  void connect() override final;

  /**
   * Create third-factor connection for given primary connection.
   *
   * @param source_gid GID of source of primary connection
   * @param target Target node of primary connection
   */
  virtual void third_connect( size_t source_gid, Node& target ) = 0;

protected:
  ThirdInBuilder* third_in_;
};


/**
 * Class representing a connection builder which may be bi- or tripartite.
 *
 * A ConnBuilder always has a primary BipartiteConnBuilder. It additionally can have a pair of third_in and third_out
 * Bipartite builders, where the third_in builder must perform one-to-one connections on given source-third pairs.
 */
class ConnBuilder
{
public:
  /**
   * Constructor for bipartite connection
   *
   * @param primary_rule Name of conn rule for primary connection
   * @param sources Source population for primary connection
   * @param targets Target population for primary connection
   * @param conn_spec Connection specification dictionary for tripartite bernoulli rule
   * @param syn_spec Collection of dictionaries with synapse specifications
   */
  ConnBuilder( const std::string& primary_rule,
    NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  /**
   * Constructor for tripartite connection
   *
   * @param primary_rule Name of conn rule for primary connection
   * @param third_rule Name of conn rule for third-factor connection
   * @param sources Source population for primary connection
   * @param targets Target population for primary connection
   * @param third Third-party population
   * @param conn_spec Connection specification dictionary for tripartite bernoulli rule
   * @param syn_specs Dictionary of synapse specifications for the three connections that may be created. Allowed keys
   * are `"primary"`, `"third_in"`, `"third_out"`, and for each of these the value must be a collection of dictionaries
   * with synapse specifications as for bipartite connectivity.
   */
  ConnBuilder( const std::string& primary_rule,
    const std::string& third_rule,
    NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    NodeCollectionPTR third,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& third_conn_spec,
    const std::map< Name, std::vector< DictionaryDatum > >& syn_specs );

  ~ConnBuilder();

  //! Connect with or without structural plasticity
  void connect();

  //! Delete synapses with or without structural plasticity
  void disconnect();

private:
  // Order of declarations based on dependencies, do not change.
  ThirdInBuilder* third_in_builder_;
  ThirdOutBuilder* third_out_builder_;
  BipartiteConnBuilder* primary_builder_;
};

/**
 * Build third-factor connectivity based on Bernoulli trials, selecting third factor nodes from a fixed pool per target
 * node.
 */
class ThirdBernoulliWithPoolBuilder : public ThirdOutBuilder
{
public:
  ThirdBernoulliWithPoolBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdInBuilder*,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );
  ~ThirdBernoulliWithPoolBuilder();

  void third_connect( size_t source_gid, Node& target ) override;

private:
  void connect_() override; //!< only call third_connect()

  /**
   * For block pool, return index of first pool element for given target node.
   *
   * @param targe_index
   */
  size_t get_first_pool_index_( const size_t target_index ) const;

  double p_;                 //!< probability of creating a third-factor connection
  bool random_pool_;         //!< random or block pool?
  size_t pool_size_;         //!< number of nodes per pool
  size_t targets_per_third_; //!< number of target nodes per third-factor node

  /**
   * Type for single pool of third-factor nodes
   *
   * @todo Could probably be BlockVector, but currently some problem with back_inserter when sampling pool.
   */
  typedef std::vector< NodeIDTriple > PoolType_;

  //! Type mapping target GID to pool for this target
  typedef std::map< size_t, PoolType_ > TgtPoolMap_;

  /**
   * Thread-specific pools of third-factor nodes.
   *
   * Each thread maintains a map from target node IDs to the third-factor node pool for that target node.
   * Since each target lives on exactly one thread, there will be no overlap. For each node, the pool is
   * created when a third-factor connection needs to be made to that node for the first time.
   * The pools are deleted when the ConnBuilder is destroyed at the end of the connect call.
   * We store a pointer instead of the map itself to ensure that the map is in thread-local memory.
   */
  std::vector< TgtPoolMap_* > pools_; // outer: threads
};


class OneToOneBuilder : public BipartiteConnBuilder
{
public:
  OneToOneBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdOutBuilder* third_out,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  bool supports_symmetric() const override;

  bool requires_proxies() const override;

protected:
  void connect_() override;

  /**
   * Connect two nodes in a OneToOne fashion with structural plasticity.
   *
   * This method is used by the SPManager based on the homostatic rules defined
   * for the synaptic elements on each node.
   */
  void sp_connect_() override;

  /**
   * Disconnecti two nodes connected in a OneToOne fashion without structural plasticity.
   *
   * This method can be manually called by the user to delete existing synapses.
   */
  void disconnect_() override;

  /**
   * Disconnect two nodes connected in a OneToOne fashion with structural plasticity.
   *
   * This method is used by the SPManager based on the homostatic rules defined
   * for the synaptic elements on each node.
   */
  void sp_disconnect_() override;
};

class AllToAllBuilder : public BipartiteConnBuilder
{
public:
  AllToAllBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdOutBuilder* third_out,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs )
    : BipartiteConnBuilder( sources, targets, third_out, conn_spec, syn_specs )
  {
  }

  bool is_symmetric() const override;

  bool requires_proxies() const override;

protected:
  void connect_() override;

  /**
   * Connect two nodes in a AllToAll fashion with structural plasticity.
   *
   * This method is used by the SPManager based on the homostatic rules defined
   * for the synaptic elements on each node.
   */
  void sp_connect_() override;

  /**
   * Disconnecti two nodes connected in a AllToAll fashion without structural plasticity.
   *
   * This method can be manually called by the user to delete existing synapses.
   */
  void disconnect_() override;

  /**
   * Disconnect two nodes connected in a AllToAll fashion with structural plasticity.
   *
   * This method is used by the SPManager based on the homostatic rules defined
   * for the synaptic elements on each node.
   */
  void sp_disconnect_() override;

private:
  void inner_connect_( const int, RngPtr, Node*, size_t, bool );
};


class FixedInDegreeBuilder : public BipartiteConnBuilder
{
public:
  FixedInDegreeBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder* third_out,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  void inner_connect_( const int, RngPtr, Node*, size_t, bool, long );
  ParameterDatum indegree_;
};

class FixedOutDegreeBuilder : public BipartiteConnBuilder
{
public:
  FixedOutDegreeBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder* third_out,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  ParameterDatum outdegree_;
};

class FixedTotalNumberBuilder : public BipartiteConnBuilder
{
public:
  FixedTotalNumberBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder* third_out,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  long N_;
};

class BernoulliBuilder : public BipartiteConnBuilder
{
public:
  BernoulliBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder* third_out,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  void inner_connect_( const int, RngPtr, Node*, size_t );
  ParameterDatum p_; //!< connection probability
};

class PoissonBuilder : public BipartiteConnBuilder
{
public:
  PoissonBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder* third_out,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  void inner_connect_( const int, RngPtr, Node*, size_t );
  ParameterDatum pairwise_avg_num_conns_; //!< Mean number of connections
};

class SymmetricBernoulliBuilder : public BipartiteConnBuilder
{
public:
  SymmetricBernoulliBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder* third_out,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

  bool
  supports_symmetric() const override
  {
    return true;
  }

protected:
  void connect_() override;

private:
  double p_; //!< connection probability
};

class SPBuilder : public BipartiteConnBuilder
{
public:
  /**
   * The SPBuilder is in charge of the creation of synapses during the simulation
   * under the control of the structural plasticity manager
   *
   * @param sources the source nodes on which synapses can be created/deleted
   * @param targets the target nodes on which synapses can be created/deleted
   * @param conn_spec connectivity specification
   * @param syn_spec synapse specifications
   */
  SPBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdOutBuilder* third_out,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_spec );

  const std::string& get_pre_synaptic_element_name() const;

  const std::string& get_post_synaptic_element_name() const;

  void set_name( const std::string& name );

  std::string get_name() const;

  /**
   * Writes the default delay of the connection model, if the SPBuilder only uses the default delay.
   *
   * If not, the min/max_delay has to be specified explicitly with the kernel status.
   */
  void update_delay( long& d ) const;

  /**
   *  @note Only for internal use by SPManager.
   */
  void sp_connect( const std::vector< size_t >& sources, const std::vector< size_t >& targets );

protected:
  //! The name of the SPBuilder; used to identify its properties in the structural_plasticity_synapses kernel attributes
  std::string name_;

  using BipartiteConnBuilder::connect_;
  void connect_() override;
  void connect_( NodeCollectionPTR sources, NodeCollectionPTR targets );

  /**
   * In charge of dynamically creating the new synapses
   *
   * @param sources nodes from which synapses can be created
   * @param targets target nodes for the newly created synapses
   */
  void connect_( const std::vector< size_t >& sources, const std::vector< size_t >& targets );
};
} // namespace nest

#endif
