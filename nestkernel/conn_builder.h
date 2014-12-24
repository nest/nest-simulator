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

#include <map>
#include <vector>

#include "dictdatum.h"
#include "gid_collection.h"
#include "lockptr.h"
#include "sliexceptions.h"

#include "gslrandomgen.h"

namespace nest {

  class Network;
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

    //! parameters: sources, targets, specifications
    ConnBuilder(Network&,
		const GIDCollection&, const GIDCollection&, 
		const DictionaryDatum&, const DictionaryDatum&);
    virtual ~ConnBuilder();

  protected:

    //! Implements the actual connection algorithm
    virtual void connect_() =0;

    //! Create connection between given nodes, fill parameter values
    void single_connect_(index, Node&, thread, librandom::RngPtr&);

    Network& net_;

    const GIDCollection& sources_;
    const GIDCollection& targets_;

    bool autapses_;
    bool multapses_;

    //! buffer for exceptions raised in threads
    std::vector<lockPTR<WrappedThreadException> > exceptions_raised_;

  private:
    typedef std::map<Name, ConnParameter*> ConnParameterMap;

    index synapse_model_;

    //! indicate that weight and delay should not be set per synapse
    bool default_weight_and_delay_;
    
    //! indicate that weight should not be set per synapse
    bool default_weight_;
    
    // null-pointer indicates that default be used
    ConnParameter* weight_;
    ConnParameter* delay_;

    //! all other parameters, mapping name to value representation
    ConnParameterMap synapse_params_;

    //! dictionaries to pass to connect function, one per thread
    std::vector<DictionaryDatum> param_dicts_;

    // check for synapse specific errors or warnings
    // This is a temporary function which should be removed once all parameter types work with Connect. 
    // The remaining error and warnings should then be handled within the synapse model.
    void check_synapse_params_(std::string, const DictionaryDatum& );
  };

  class OneToOneBuilder : public ConnBuilder
  {
  public:
  OneToOneBuilder(Network& net,
		  const GIDCollection& sources, 
		  const GIDCollection& targets, 
		  const DictionaryDatum& conn_spec,
		  const DictionaryDatum& syn_spec)
    : ConnBuilder(net, sources, targets, conn_spec, syn_spec)
    {}

  protected:
    void connect_();
  };

  class AllToAllBuilder : public ConnBuilder
  {
  public:
  AllToAllBuilder(Network& net,
		  const GIDCollection& sources, 
		  const GIDCollection& targets, 
		  const DictionaryDatum& conn_spec,
		  const DictionaryDatum& syn_spec)
    : ConnBuilder(net, sources, targets, conn_spec, syn_spec)
    {}

  protected:
    void connect_();
  };


  class FixedInDegreeBuilder : public ConnBuilder
  {
  public:
    FixedInDegreeBuilder(Network&,
			 const GIDCollection&, 
			 const GIDCollection&, 
			 const DictionaryDatum&,
			 const DictionaryDatum&);

  protected:
    void connect_();

  private:
    long indegree_;   
  };

  class FixedOutDegreeBuilder : public ConnBuilder
  {
  public:
    FixedOutDegreeBuilder(Network&,
			  const GIDCollection&, 
			  const GIDCollection&, 
			  const DictionaryDatum&,
			  const DictionaryDatum&);

  protected:
    void connect_();

  private:
    long outdegree_;   
  };

  class FixedTotalNumberBuilder : public ConnBuilder
  {
  public:
    FixedTotalNumberBuilder(Network&,
			    const GIDCollection&, 
			    const GIDCollection&, 
			    const DictionaryDatum&,
			    const DictionaryDatum&);

  protected:
    void connect_();

  private:
    long N_;   
  };

  class BernoulliBuilder : public ConnBuilder
  {
  public:
    BernoulliBuilder(Network&,
		     const GIDCollection&, 
		     const GIDCollection&, 
		     const DictionaryDatum&,
		     const DictionaryDatum&);

  protected:
    void connect_();

  private:
    double p_;   //!< connection probability
  };

  
}  // namespace nest

#endif
