/*
 *  connection_id.h
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

#ifndef CONNECTION_ID_H
#define CONNECTION_ID_H

#include "dictutils.h"
#include "nest_names.h"

namespace nest
{
  
  class ConnectionID
  {
    public:
      ConnectionID() {}  
      ConnectionID(long source_gid, long target_thread, long synapse_typeid, long port);
      
      DictionaryDatum get_dict();
      bool operator==(const ConnectionID& c);
      std::ostream & print_me(std::ostream& out) const;
    
    private:
      long source_gid_;
      long target_thread_;
      long synapse_typeid_;
      long port_;
  };

  std::ostream & operator<<(std::ostream& , const ConnectionID&);
  
} // namespace

#endif /* #ifndef CONNECTION_ID_H */
