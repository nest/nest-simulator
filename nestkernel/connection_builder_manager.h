/*
 *  connection_builder_manager.h
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

#ifndef CONNECTIONBUILDER_MANAGER_H 
#define CONNECTIONBUILDER_MANAGER_H 

#include <omp.h>

#include "nest.h"
#include "manager_interface.h"
#include "kernel_manager.h"

namespace nest
{

class GenericConnBuilderFactory;

class ConnectionBuilderManager : ManagerInterface
{
public:
  ConnectionBuilderManager();
  ~ConnectionBuilderManager()
  {
  }

  virtual void init();
  virtual void reset();

  virtual void set_status( const Dictionary& );
  virtual void get_status( Dictionary& );

  /** 
   * BeginDocumentation
   * Name: connruledict - dictionary containing all connectivity rules
   * Description:
   * This dictionary provides the connection rules that can be used
   * in Connect.
   * 'connruledict info' shows the contents of the dictionary.
   * SeeAlso: Connect
   */
  Dictionary* connruledict_; //!< Dictionary for connection rules.

  /**
   * Add a connectivity rule, i.e. the respective ConnBuilderFactory.
   */
  template < typename ConnBuilder >
  void register_conn_builder( const std::string& name );

  std::vector< GenericConnBuilderFactory* >
  connbuilder_factories_; //! ConnBuilder factories, indexed by connruledict_ elements.

  /**
   * Create connections.
   */
  void connect( const GIDCollection&,
		const GIDCollection&,
		const DictionaryDatum&,
		const DictionaryDatum& );

  /**
   * Connect from an array of dictionaries.
   */
  void connect( ArrayDatum& connectome );

};


inline void
ConnectionBuilderManager::connect( ArrayDatum& connectome )
{
  kernel().connection_manager.connect( connectome );
}

} // namespace nest

#endif /* CONNECTIONBUILDER_MANAGER_H */
