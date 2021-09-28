/*
 *  conn_builder_sonata.h
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

#ifndef CONN_BUILDER_SONATA_H
#define CONN_BUILDER_SONATA_H

#include "config.h"

// C++ includes:
#include <map>
#include <vector>

// Includes from nestkernel:
#include "conn_builder.h"
#include "nest_datums.h"

namespace nest
{

class SonataBuilder : public ConnBuilder
{

public:
  SonataBuilder( NodeCollectionPTR sources,
      NodeCollectionPTR targets,
      const DictionaryDatum& conn_spec,
      const std::vector< DictionaryDatum >& syn_specs );

protected:
  void connect_();

};

} // namespace nest


#endif /* ifdef CONN_BUILDER_SONATA_H */
