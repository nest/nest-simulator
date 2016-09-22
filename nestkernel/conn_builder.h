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
 * This is a very first draft, a very much stripped-down version of the
 * Topology connection_creator.
 *
 */

// C++ includes:
#include <map>
#include <vector>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from librandom:
#include "gslrandomgen.h"

// Includes from nestkernel:
#include "conn_parameter.h"
#include "gid_collection.h"
#include "nest_time.h"

// Includes from sli:
#include "dictdatum.h"
#include "sliexceptions.h"

namespace nest
{
class Node;
class ConnParameter;

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
  /**
   * Connect sources to targets according to specifications in dictionary.
   *
   * To create a connection, call
   *
   *   cb.connect();
   *
   * where conn_spec_dict speficies connection type and its parameters.
   */
  virtual void connect();
  virtual void disconnect();

  //! parameters: sources, targets, specifications
  ConnBuilder( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );
  virtual ~ConnBuilder();

  index
  get_synapse_model() const
  {
    return synapse_model_;
  }

  bool
  get_default_delay() const
  {
    return default_delay_;
  }

  void set_pre_synaptic_element_name( std::string name );
  void set_post_synaptic_element_name( std::string name );

  int change_connected_synaptic_elements( index, index, const int, int );

  virtual bool
  supports_symmetric() const
  {
    return false;
  }

protected:
  //! Implements the actual connection algorithm
  virtual void connect_() = 0;
  virtual void
  sp_connect_()
  {
    throw NotImplemented(
      "This connection rule is not implemented for structural plasticity" );
  }
  virtual void
  disconnect_()
  {
    throw NotImplemented( "This disconnection rule is not implemented" );
  }
  virtual void
  sp_disconnect_()
  {
    throw NotImplemented(
      "This connection rule is not implemented for structural plasticity" );
  }

  //! Create connection between given nodes, fill parameter values
  void single_connect_( index, Node&, thread, librandom::RngPtr& );
  void single_disconnect_( index, Node&, thread );

  /**
   * Moves pointer in parameter array.
   *
   * Calls value-function of all parameters being instantiations of
   * ArrayDoubleParameter or ArrayIntegerParameter, thus moving the pointer
   * to the next parameter value. The function is called when the target
   * node is not located on the current thread or MPI-process and read of an
   * array.
   */
  void skip_conn_parameter_( thread );

  GIDCollection const* sources_;
  GIDCollection const* targets_;

  bool autapses_;
  bool multapses_;
  bool symmetric_;

  //! buffer for exceptions raised in threads
  std::vector< lockPTR< WrappedThreadException > > exceptions_raised_;

  // Name of the pre synaptic and post synaptic elements for this connection
  // builder
  std::string pre_synaptic_element_name;
  std::string post_synaptic_element_name;

private:
  typedef std::map< Name, ConnParameter* > ConnParameterMap;

  index synapse_model_;

  //! indicate that weight and delay should not be set per synapse
  bool default_weight_and_delay_;

  //! indicate that weight should not be set per synapse
  bool default_weight_;

  //! indicate that delay should not be set per synapse
  bool default_delay_;

  // null-pointer indicates that default be used
  ConnParameter* weight_;
  ConnParameter* delay_;

  //! all other parameters, mapping name to value representation
  ConnParameterMap synapse_params_;

  //! dictionaries to pass to connect function, one per thread
  std::vector< DictionaryDatum > param_dicts_;

  //! pointers to connection parameters specified as arrays
  std::vector< ConnParameter* > parameters_requiring_skipping_;

  /**
   * Collects all array paramters in a vector.
   *
   * If the inserted parameter is an array it will be added to a vector of
   * ConnParameters. This vector will be exploited in some connection
   * routines to ensuring thread-safety.
   */
  void register_parameters_requiring_skipping_( ConnParameter& param );

  // check for synapse specific errors or warnings
  // This is a temporary function which should be removed once all parameter
  // types work with Connect.
  // The remaining error and warnings should then be handled within the synapse
  // model.
  void check_synapse_params_( std::string, const DictionaryDatum& );
};

class OneToOneBuilder : public ConnBuilder
{
public:
  OneToOneBuilder( const GIDCollection& sources,
    const GIDCollection& targets,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_spec )
    : ConnBuilder( sources, targets, conn_spec, syn_spec )
  {
  }

  bool
  supports_symmetric() const
  {
    return true;
  }

protected:
  void connect_();
  void sp_connect_();
  void disconnect_();
  void sp_disconnect_();
};

class AllToAllBuilder : public ConnBuilder
{
public:
  AllToAllBuilder( const GIDCollection& sources,
    const GIDCollection& targets,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_spec )
    : ConnBuilder( sources, targets, conn_spec, syn_spec )
  {
  }

protected:
  void connect_();
  void sp_connect_();
  void disconnect_();
  void sp_disconnect_();
};


class FixedInDegreeBuilder : public ConnBuilder
{
public:
  FixedInDegreeBuilder( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );

protected:
  void connect_();

private:
  long indegree_;
};

class FixedOutDegreeBuilder : public ConnBuilder
{
public:
  FixedOutDegreeBuilder( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );

protected:
  void connect_();

private:
  long outdegree_;
};

class FixedTotalNumberBuilder : public ConnBuilder
{
public:
  FixedTotalNumberBuilder( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );

protected:
  void connect_();

private:
  long N_;
};

class BernoulliBuilder : public ConnBuilder
{
public:
  BernoulliBuilder( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );

protected:
  void connect_();

private:
  double p_; //!< connection probability
};

class SPBuilder : public ConnBuilder
{
public:
  SPBuilder( const GIDCollection& sources,
    const GIDCollection& targets,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_spec );

  std::string
  get_pre_synaptic_element_name() const
  {
    return pre_synaptic_element_name;
  }
  std::string
  get_post_synaptic_element_name() const
  {
    return post_synaptic_element_name;
  }

  /**
   * Writes the default delay of the connection model, if the
   * SPBuilder only uses the default delay. If not, the min/max_delay
   * has to be specified explicitly with the kernel status.
   */
  void update_delay( delay& d ) const;

  void sp_connect( GIDCollection sources, GIDCollection targets );

protected:
  void connect_();
  void connect_( GIDCollection sources, GIDCollection targets );
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
ConnBuilder::skip_conn_parameter_( thread target_thread )
{
  for ( std::vector< ConnParameter* >::iterator it =
          parameters_requiring_skipping_.begin();
        it != parameters_requiring_skipping_.end();
        ++it )
    ( *it )->skip( target_thread );
}

} // namespace nest

#endif
