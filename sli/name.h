#ifndef NAME_H
#define NAME_H
/*
 *  name.h
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

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <iostream>

/**
 * Represent strings by ints to facilitate fast comparison.
 *
 * Each Name object represents a string by a unique integer number.
 * Comparing Name objects instead of comparing strings directly,
 * reduces the complexity of string comparison to that of int comparison.
 *
 * Each Name object contains a Handle to the string it represents. Strings are 
 * mapped to Handles via an associative array. Handles are stored in a table,
 * and each Handle contains its own index into this table as unique ID, as
 * well as the string represented. Fast comparison of Name objects is achieved
 * by comparing the indices stored in the handles. Reference counting 
 * permits deletion of unused Handles.
 *
 * @note Any string read by the interpreter should be converted to a Name
 * at once.
 * @note class Name maintains two static lookup tables and is thus not
 * thread-safe.
 *
 */

class Name
{
private:

  /**
   * Dataytype for Handles.
   * Initially, the table is empty.  When
   * creating a Handle for a new string, a TableEntry_ object is
   * inserted at the end of the table, provided there are no holes
   * in the table. When a Handle is created for a string that is
   * already present in the table, the reference count for that
   * TableEntry_ is increased. When the reference count of a
   * TableEntry_ drops to zero, a hole is created in the table. When
   * there are holes in the table, new TableEntry_ objects are
   * placed at the first hole in the table.
   */
  class Handle 
  {
  public:

    /**
     * Create a copy.
     * Registers the additional reference in the handleTable_.
     */
    Handle(const Handle&);

    /**
     * Create from string.
     * @note This does NOT check for duplicates in the table.
     *       Duplicate checking must be done by the caller.
     */
    Handle(const std::string&);

    /**
     * Delete Handle object.
     * The reference count in the handleTable_ is reduced. If it 
     * drops to zero, the table entry is deleted.
     */
    ~Handle();

    /**
     * Assign.
     * Reduces the reference count for the original handled entry and 
     * adds a reference to the value.
     */
    const Handle& operator=(const Handle&);

    //! integer ID of handle
    std::size_t get_id() const;

    //! number of references to handle
    std::size_t num_references() const;

    //! string represented by handle
    const std::string& lookup() const;

    bool operator==(const Handle&) const;
    bool operator!=(const Handle&) const;
    bool operator< (const Handle&) const;

    /**
     * Capacity of the underlying table (STL-sense capacity).
     */
    static std::size_t capacity(); 

    /**
     * Number of Handles in table.
     * This is obtained by scanning the entire
     * table for elements with non-zero reference count.
     */
    static std::size_t num_handles();

    /**
     * Total number of references to Handles in the table.
     * NullHandle is ignored. This is obtained by scanning the entire
     * table.
     */
    static std::size_t total_num_references();

    /**
     * Dump content of Handle table.
     */
    static void list(std::ostream&);

    /**
     * Provide information about Handle table.
     */
    static void info(std::ostream&);

    /**
     * Print individual handle.
     */
    void print(std::ostream&) const;

  private:

    /**
     * Remove handle from table.
     */
    void erase();

    //! ID of the Handle object
    std::size_t handle_;

    //! Type for entries in Handle table
    struct TableEntry_ {
      TableEntry_() : ref_count_(0) {}  //!< construct NullHandle
      TableEntry_(const std::string& s) : string_(s), ref_count_(0) {}
      std::string string_;     //!< string represented
      std::size_t ref_count_;  //!< reference count
    };

    /**
     * Datatype for handleTable_.
     */
    typedef std::vector<TableEntry_> HandleTable_;


    //! Block size for growing table
    static const std::size_t tableBlockSize_;

    /** Index of next available handle in table. 
     *  @note next_handle_ must index the next available table element
     *        at all times.
     */
    static std::size_t next_handle_; 

  public:
    /** 
     * Function returning a reference to the single table instance.
     * Implementation akin to Meyers Singleton, see Alexandrescu, ch 6.4.
     * @note Public so that it can be called by Name::handleMapInstance_()
     */
    static HandleTable_& handleTableInstance_();

  };

public:

  /**
   * Create Name without value.
   */
  Name()                     : name_(insert("-- ANONYMOUS --")) {}

  Name(const char s[])       : name_(insert(std::string(s))) {} 
  Name(const std::string &s) : name_(insert(s)) {}

  Name(const Name &n)        : name_(n.name_) {}

  ~Name();

  /**
   * Return string represented by Name.
   */
  const std::string& toString(void) const;

  /**
   * Return table index for Name object.
   */
  std::size_t toIndex(void)  const 
  {
    return name_.get_id();
  }
  
  bool operator== (const Name &n) const
  {
    return name_ == n.name_;
  }
  
  bool operator!= (const Name &n) const
  {
    return !(name_ == n.name_);
  }
  
  /**
   * Non-alphabetic ordering of names.
   * Entering Name's into dictionaries requires ordering. Ordering based
   * on string comparison would be very slow. We thus compare based on
   * table indices.
   */
  bool operator< (const Name &n) const
  {
    return name_ < n.name_;
  }

  static bool lookup(const std::string &s)
  {
    return (handleMapInstance_().find(s) != handleMapInstance_().end());
  }
  
  static void list(std::ostream &);
  static void info(std::ostream &);

private:

  /**
   * Datatype for map from strings to handles. 
   */
  typedef std::map<std::string, Name::Handle, std::less<std::string> > String2HandleMap_;

  /** 
   * Function returning a reference to the single map instance.
   * Implementation akin to Meyers Singleton, see Alexandrescu, ch 6.4.
   */
  static String2HandleMap_& handleMapInstance_();

  /**
   * Handle for the name represented by the Name object.
   */
  Handle name_;

  const Handle& insert(const std::string&);  
};


std::ostream& operator<<(std::ostream&, const Name&);


inline
std::size_t Name::Handle::get_id() const
{
  return handle_;
}

inline
std::size_t Name::Handle::num_references() const
{
  assert(handle_ < handleTableInstance_().size());
  assert(handleTableInstance_()[handle_].ref_count_ > 0);

  return handleTableInstance_()[handle_].ref_count_;
}

inline
const std::string& Name::Handle::lookup() const
{
  assert(handle_ < handleTableInstance_().size());
  assert(handleTableInstance_()[handle_].ref_count_ > 0);

  return handleTableInstance_()[handle_].string_;
}

inline
bool Name::Handle::operator==(const Name::Handle& rhs) const
{
  return handle_ == rhs.handle_;
}

inline
bool Name::Handle::operator!=(const Name::Handle& rhs) const
{
  return ! ( *this == rhs );
}

inline
bool Name::Handle::operator<(const Name::Handle& rhs) const
{
  return handle_ < rhs.handle_;
}

inline
Name::Handle::HandleTable_& Name::Handle::handleTableInstance_()
{
  // Meyers singleton, created first time function is invoked.

  /* handleTable_ is initalized with size >= 1, so that there is
   * space in the table for at least one element and next_handle_
   * is a valid element when initialized with 0.
   */
  static HandleTable_ handleTable(tableBlockSize_);

  return handleTable;
}

inline
Name::String2HandleMap_& Name::handleMapInstance_()
{
  // force creation of the handleTable before the handleMap is created, since
  // the map contains Handles, so the map must be deleted before the table
  // so that the Handles can properly be removed from the table.
  Handle::handleTableInstance_();

  // Meyers singleton, created first time function is invoked.
  static String2HandleMap_ handleMap;

  return handleMap;
}

#endif

