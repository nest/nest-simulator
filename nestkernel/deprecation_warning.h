/*
 *  deprecation_warning.h
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

#ifndef DEPRECATION_WARNING_H
#define DEPRECATION_WARNING_H

//#include "node.h"
#include <map>
#include <iostream>
#include <assert.h>

namespace nest
{
class DeprecationWarning
{
public:
  friend class Node;

  DeprecationWarning();
  DeprecationWarning( const DeprecationWarning& dn );
  virtual ~DeprecationWarning()
  {
  }

  void set_deprecation_warnings();
  bool get_deprecation_warning( const std::string );

  // private:
  std::map< std::string, bool > deprecation_warning_;
};

DeprecationWarning::DeprecationWarning()
  : deprecation_warning_()
{
  // std::cerr << "kommer vi kanskje aldri inn hit?\n";
  // deprecation_warning_["allow_offgrid_spikes"] = true;
}

DeprecationWarning::DeprecationWarning( const DeprecationWarning& dn )
  : deprecation_warning_( dn.deprecation_warning_ )
{
  // std::cerr << "er det her vi kommer inn?\n";
}

void
DeprecationWarning::set_deprecation_warnings()
{
  // std::cerr << "naa haaper jeg vi kommer inn hit!\n";

  // deprecation_warning_["allow_offgrid_spikes"] = true;
  deprecation_warning_.insert(
    std::pair< std::string, bool >( "allow_offgrid_spikes", true ) );

  assert( deprecation_warning_.find( "allow_offgrid_spikes" )->second == true );

  // std::cerr << "og nå er den vel ikke tom?? " << deprecation_warning_.empty()
  // << " " << deprecation_warning_.size() << std::endl;
}

bool
DeprecationWarning::get_deprecation_warning( const std::string param )
{
  // std::cerr << "kommer vi inn hit da?\n";
  bool deprecated = false;

  std::map< std::string, bool >::const_iterator it;

  // std::cerr << "er det når vi skal finne deprecation warningen at vi
  // feiler??\n";

  // std::cerr << "er den tom??\n";
  // std::cerr << "tom? " << deprecation_warning_.empty() << " " <<
  // deprecation_warning_.size() << std::endl;
  // std::cerr << "deprecation warningen er (begynnelse): \n";
  // std::cerr << deprecation_warning_.find( "allow_offgrid_spikes" )->second <<
  // std::endl;

  it = deprecation_warning_.find( param );

  // std::cerr << "er det find?\n";

  // std::cerr << "deprecation warningen er (begynnelse): " <<
  // deprecation_warning_[param] << std::endl;

  if ( it != deprecation_warning_.end() )
  {
    if ( deprecation_warning_[ param ] )
    {
      // std::cerr << "vi skal jo ikke inn her hver gang..\n";
      deprecated = true;
      deprecation_warning_[ param ] = false;
    }
  }

  // std::cerr << "deprecation warningen er: " << deprecation_warning_[param] <<
  // std::endl;
  // std::cerr << "gi meg deprecated: " << deprecated << std::endl;

  return deprecated;
}
}

#endif /* DEPRECATION_WARNING_H */
