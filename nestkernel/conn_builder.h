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

// Includes from nestkernel:
#include "conn_parameter.h"
#include "nest_time.h"
#include "node_collection.h"
#include "parameter.h"

// Includes from sli:
#include "dictdatum.h"
#include "sliexceptions.h"

namespace nest
{
class Node;
class ConnParameter;
class SparseNodeArray;

/**
 * Abstract base class for ConnBuilders.
 *
 * The base class extracts and holds parameters and provides
 * the connect interface. Derived classes implement the connect
 * method.
 *
 * @note Naming classes *Builder to avoid name confusion with Connector classes.
 */

class ConnBuilder
{
public:
  //! Connect with or without structural plasticity
  virtual void connect();

  //! Delete synapses with or without structural plasticity
  virtual void disconnect();

  ConnBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );
  virtual ~ConnBuilder();

  /**
   * Mark ConnBuilder subclasses as building tripartite rules or not.
   *
   * @note This flag is required for template specialisation of ConnBuilderFactory's.
   */
  static constexpr bool is_tripartite = false;

  size_t
  get_synapse_model() const
  {
    if ( synapse_model_id_.size() > 1 )
    {
      throw KernelException( "Can only retrieve synapse model when one synapse per connection is used." );
    }
    return synapse_model_id_[ 0 ];
  }

  bool
  get_default_delay() const
  {
    if ( synapse_model_id_.size() > 1 )
    {
      throw KernelException( "Can only retrieve default delay when one synapse per connection is used." );
    }
    return default_delay_[ 0 ];
  }

  void set_synaptic_element_names( const std::string& pre_name, const std::string& post_name );

  bool all_parameters_scalar_() const;

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

  virtual bool
  supports_symmetric() const
  {
    return false;
  }

  virtual bool
  is_symmetric() const
  {
    return false;
  }

  bool
  allows_autapses() const
  {
    return allow_autapses_;
  }

  bool
  allows_multapses() const
  {
    return allow_multapses_;
  }

  //! Return true if rule is applicable only to nodes with proxies
  virtual bool
  requires_proxies() const
  {
    return true;
  }

protected:
  //! Implements the actual connection algorithm
  virtual void connect_() = 0;

  virtual void
  sp_connect_()
  {
    throw NotImplemented( "This connection rule is not implemented for structural plasticity." );
  }

  virtual void
  disconnect_()
  {
    throw NotImplemented( "This disconnection rule is not implemented." );
  }

  virtual void
  sp_disconnect_()
  {
    throw NotImplemented( "This connection rule is not implemented for structural plasticity." );
  }

  void update_param_dict_( size_t snode_id, Node& target, size_t target_thread, RngPtr rng, size_t indx );

  //! Create connection between given nodes, fill parameter values
  void single_connect_( size_t, Node&, size_t, RngPtr );
  void single_disconnect_( size_t, Node&, size_t );

  /**
   * Moves pointer in parameter array.
   *
   * Calls value-function of all parameters being instantiations of
   * ArrayDoubleParameter or ArrayIntegerParameter, thus moving the pointer
   * to the next parameter value. The function is called when the target
   * node is not located on the current thread or MPI-process and read of an
   * array.
   */
  void skip_conn_parameter_( size_t, size_t n_skip = 1 );

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

  NodeCollectionPTR sources_;
  NodeCollectionPTR targets_;

  bool allow_autapses_;
  bool allow_multapses_;
  bool make_symmetric_;
  bool creates_symmetric_connections_;

  //! buffer for exceptions raised in threads
  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_;

  // Name of the pre synaptic and postsynaptic elements for this connection builder
  std::string pre_synaptic_element_name_;
  std::string post_synaptic_element_name_;

  bool use_structural_plasticity_;

  //! pointers to connection parameters specified as arrays
  std::vector< ConnParameter* > parameters_requiring_skipping_;

  std::vector< size_t > synapse_model_id_;

  //! dictionaries to pass to connect function, one per thread for every syn_spec
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

class OneToOneBuilder : public ConnBuilder
{
public:
  OneToOneBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  bool
  supports_symmetric() const override
  {
    return true;
  }

  bool
  requires_proxies() const override
  {
    return false;
  }

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

class AllToAllBuilder : public ConnBuilder
{
public:
  AllToAllBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs )
    : ConnBuilder( sources, targets, conn_spec, syn_specs )
  {
  }

  bool
  is_symmetric() const override
  {
    return sources_ == targets_ and all_parameters_scalar_();
  }

  bool
  requires_proxies() const override
  {
    return false;
  }

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


class FixedInDegreeBuilder : public ConnBuilder
{
public:
  FixedInDegreeBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  void inner_connect_( const int, RngPtr, Node*, size_t, bool, long );
  ParameterDatum indegree_;
};

class FixedOutDegreeBuilder : public ConnBuilder
{
public:
  FixedOutDegreeBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  ParameterDatum outdegree_;
};

class FixedTotalNumberBuilder : public ConnBuilder
{
public:
  FixedTotalNumberBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  long N_;
};

class BernoulliBuilder : public ConnBuilder
{
public:
  BernoulliBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_() override;

private:
  void inner_connect_( const int, RngPtr, Node*, size_t );
  ParameterDatum p_; //!< connection probability
};

/**
 * Helper class to support parameter handling for tripartite builders.
 *
 * In tripartite builders, the actual builder class decides which connections to create and
 * handles parameterization of the primary connection. For each third-party connection,
 * it maintains an AuxiliaryBuilder which handles the parameterization of the corresponding
 * third-party connection.
 */
class AuxiliaryBuilder : public ConnBuilder
{
public:
  AuxiliaryBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

  //! forwards to single_connect_() in underlying ConnBuilder
  void single_connect( size_t, Node&, size_t, RngPtr );

protected:
  void
  connect_() override
  {
    // The auxiliary builder does not create connections, it only parameterizes them.
    assert( false );
  }
};

/**
 * Class representing tripartite Bernoulli connector
 *
 * For each source-target pair, a Bernoulli trial is performed. If a primary connection is created, a third-factor
 * connection is created conditionally on a second Bernoulli trial. The third-party neuron to be connected is
 * chosen from a pool, which can either be set up in blocks or randomized. The third-party neuron receives
 * input from the source neuron and provides output to the target neuron of the primary connection.
 */
class TripartiteBernoulliWithPoolBuilder : public ConnBuilder
{
public:
  /**
   * Constructor
   *
   * @param sources Source population for primary connection
   * @param targets Target population for primary connection
   * @param third Third-party population
   * @param conn_spec Connection specification dictionary for tripartite bernoulli rule
   * @param syn_specs Dictionary of synapse specifications for the three connections that may be created. Allowed keys
   * are `"primary"`, `"third_in"`, `"third_out"`
   */
  TripartiteBernoulliWithPoolBuilder( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    NodeCollectionPTR third,
    const DictionaryDatum& conn_spec,
    const std::map< Name, std::vector< DictionaryDatum > >& syn_specs );

  static constexpr bool is_tripartite = true;

protected:
  void connect_() override;

private:
  //! Provide index of first third-party node to be assigned to pool for given target node
  size_t get_first_pool_index_( const size_t target_index ) const;

  NodeCollectionPTR third_;

  AuxiliaryBuilder third_in_builder_;
  AuxiliaryBuilder third_out_builder_;

  double p_primary_;          //!< connection probability for pre-post connections
  double p_third_if_primary_; //!< probability of third-factor connection if primary connection created
  bool random_pool_;          //!< if true, select astrocyte pool at random
  size_t pool_size_;          //!< size of third-factor pool
  size_t targets_per_third_;  //!< target nodes per third-factor node
};

class SymmetricBernoulliBuilder : public ConnBuilder
{
public:
  SymmetricBernoulliBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
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

class SPBuilder : public ConnBuilder
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
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_spec );

  const std::string&
  get_pre_synaptic_element_name() const
  {
    return pre_synaptic_element_name_;
  }

  const std::string&
  get_post_synaptic_element_name() const
  {
    return post_synaptic_element_name_;
  }

  void
  set_name( const std::string& name )
  {
    name_ = name;
  }

  std::string
  get_name() const
  {
    return name_;
  }

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

  using ConnBuilder::connect_;
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

inline void
ConnBuilder::register_parameters_requiring_skipping_( ConnParameter& param )
{
  if ( param.is_array() )
  {
    parameters_requiring_skipping_.push_back( &param );
  }
}

inline void
ConnBuilder::skip_conn_parameter_( size_t target_thread, size_t n_skip )
{
  for ( std::vector< ConnParameter* >::iterator it = parameters_requiring_skipping_.begin();
        it != parameters_requiring_skipping_.end();
        ++it )
  {
    ( *it )->skip( target_thread, n_skip );
  }
}

} // namespace nest

#endif
