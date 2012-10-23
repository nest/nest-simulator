/*
 *  name.cc
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

#include "name.h"

#include <iostream>
#include <iomanip>


// ---------------------------------------------------------------

/* Static data for Name::Handle:
 * These are POD variables/constants with compile-time constants
 * as initializers. They are therefore initialized upon loading,
 * before any code is executed. They are thus initalized before
 * Name::Handle::handleTableInstance_() is executed for the very
 * first time, creating the actual table.
 */
const std::size_t Name::Handle::tableBlockSize_ = 100;
std::size_t       Name::Handle::next_handle_= 0;

// ---------------------------------------------------------------

Name::Handle::Handle(const Name::Handle& h) :
  handle_(h.handle_)
{
  assert(handle_ < handleTableInstance_().size());
  // Handles with ref count zero should not exist
  assert(handleTableInstance_()[handle_].ref_count_ > 0);

  ++(handleTableInstance_()[handle_].ref_count_);
}

Name::Handle::Handle(const std::string& s)  :
  handle_(next_handle_)
{
  // next_handle_ ALWAYS points to next available table slot, so 
  // we can insert right away; handle_ is set in the initalizer list
  assert(handle_ < handleTableInstance_().size());
  handleTableInstance_()[handle_] = TableEntry_(s);
  handleTableInstance_()[handle_].ref_count_ = 1;

  // advance to next available slot
  do {
    ++next_handle_;
  }  while ( next_handle_ < handleTableInstance_().size() 
	     && handleTableInstance_()[next_handle_].ref_count_ != 0 );
  
  // expand table if necessary
  if ( next_handle_ == handleTableInstance_().size() )
    handleTableInstance_().resize(handleTableInstance_().size() + tableBlockSize_);

  // verify that next_handle_ points to a valid handle
  assert(next_handle_ < handleTableInstance_().size());
  assert(handleTableInstance_()[next_handle_].ref_count_ == 0);
}


Name::Handle::~Handle()
{
  assert(handle_ < handleTableInstance_().size());
  assert(handleTableInstance_()[handle_].ref_count_ > 0);
  erase();
}
  
const Name::Handle& Name::Handle::operator=(const Name::Handle& rhs)
{
  if ( handle_ != rhs.handle_ )
  {
    erase();
    handle_ = rhs.handle_;
    assert(handle_ < handleTableInstance_().size());
    ++(handleTableInstance_()[handle_].ref_count_);
  }
  return *this;
}

void Name::Handle::erase()
{
  assert(handle_ < handleTableInstance_().size());
  assert(handleTableInstance_()[handle_].ref_count_ > 0);

  --(handleTableInstance_()[handle_].ref_count_);

  if ( handleTableInstance_()[handle_].ref_count_  == 0 )
    if ( next_handle_ > handle_ )
      next_handle_ = handle_;     // have created a hole

  // just double checking that next_handle_ is valid
  assert(next_handle_ < handleTableInstance_().size());
  assert(handleTableInstance_()[next_handle_].ref_count_ == 0);
}

std::size_t Name::Handle::capacity()
{
  return handleTableInstance_().size();
}

std::size_t Name::Handle::num_handles()
{
  std::size_t n = 0;

  for ( HandleTable_::const_iterator it = handleTableInstance_().begin() ;
	it != handleTableInstance_().end() ; ++it )
    if ( it->ref_count_ > 0 )
      ++n;

  return n;
}

std::size_t Name::Handle::total_num_references()
{
  std::size_t n = 0;

  for ( HandleTable_::const_iterator it = handleTableInstance_().begin() ;
	it != handleTableInstance_().end() ; ++it )
    n += it->ref_count_;

  return n;
}

void Name::Handle::list(std::ostream& os)
{
  std::size_t n = 0;

  for ( HandleTable_::const_iterator it = handleTableInstance_().begin() ; 
	it != handleTableInstance_().end() ; ++it )
  {
    os << std::setw(6) << n << ": "
       << std::setw(6) << it->ref_count_ << ": "
       << it->string_ << std::endl;
    ++n;
  }
}

void Name::Handle::info(std::ostream& os)
{
  os << "Table size    : " << std::setw(8) << Handle::capacity() << std::endl
     << "Num handles   : " << std::setw(8) << Handle::num_handles() << std::endl
     << "Num references: " << std::setw(8) << Handle::total_num_references() << std::endl;
}

void Name::Handle::print(std::ostream &o) const
{
    o << "[ "<< num_references() << " -> \"" << lookup() <<"\" ]";
}

// ---------------------------------------------------------------

Name::~Name()
{
  // There are at least two references to a handleTable_ entry for which
  // a Name object exists: the reference from the Name object itself
  // (i.e., this), and from the entry in the String2HandleMap.
  // Theres is one important exception to this rule: The very first Name
  // object created is created before the handleMap and thus destroyed after
  // it. Thus, the handleMap is already destroyed when that first Name object
  // is destroyed, so that that Name object has a Handle with only a single
  // reference, namely the Name object. Thus, no attempt is made to erase that
  // Name object from the no longer existing handleMap.
  // A difficulty with this explanation is that if my understanding of the
  // order of creation and deletion is correct, then the handleTable_ should
  // be destroyed in between the handleMap and the very last Name, whence it
  // should be impossible to check the number of references.
  if ( name_.num_references() == 2 )
    handleMapInstance_().erase(handleMapInstance_().find(name_.lookup())); // remove from map
}

const std::string& Name::toString(void) const
{
  return name_.lookup();
}

const Name::Handle& Name::insert(const std::string &s)
{
  Name::String2HandleMap_::const_iterator where;

  where = handleMapInstance_().find(s);
  if( where == handleMapInstance_().end() )
  {
    // The following is more comlex code than a simple
    // handleMap_[s] = Handle(s), but it avoids the creation
    // of spurious Handle objects. HEP 2007-05-24
    const std::pair<String2HandleMap_::iterator, bool> 
      res = handleMapInstance_().insert(std::make_pair(s, Handle(s)));
    assert(res.second);
    return res.first->second;
  }
  else
    return ((*where).second);
}

void Name::list(std::ostream &out)
{
  out << "\nString2Handle Map content:" << std::endl;
  for ( Name::String2HandleMap_::const_iterator where = handleMapInstance_().begin(); 
	where != handleMapInstance_().end(); ++where )
  {
    out << (*where).first << " -> ";
    (*where).second.print(out);
    out << std::endl;
  }

  out << "\nHandle::handleTable_ content" << std::endl;
  Name::Handle::list(out);
}

void Name::info(std::ostream &out)
{
  out << "Total number of names : "<< handleMapInstance_().size() << std::endl;
  out << "State of Handle Table: \n";
  Handle::info(out);
}



std::ostream & operator<< (std::ostream & o, const Name &n)
{
    o << n.toString().c_str() ;
    return o;
}


