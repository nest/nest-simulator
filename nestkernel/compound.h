/*
 *  compound.h
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

#ifndef COMPOUND_H
#define COMPOUND_H
#include <vector>
#include <string>
#include "node.h"
#include "dictdatum.h"

/* BeginDocumentation

Name: subnet - Root node for compound networks.

Description:
A network node of type subnet serves as a root node for subnetworks
(also called compound networks or compounds).

Parameters:
Parameters that can be accessed via the GetStatus and SetStatus functions:

children_on_same_vp (booltype) -
  Whether all children are allocated on the same virtual process
customdict (dictionarytype) -
  A user-defined dictionary, which may be used to store additional data.
label (stringtype) -
  A user-defined string, which  may be used to give a symbolic name to the
  node. From the SLI level, the FindNodes command may be used to find a
  compound's addess from it's label.
number_of_children (integertype) -
  The numbber of direct children of the subnet

Author:
(unknown). label and customdict attribute added by kupper, 18-jul-2003.

Remarks:
  This model is called "subnet" in SLIs modeldict, while the C++ class
  implementing it is called "Compound".

  SeeAlso: modeldict, Node
*/


namespace nest{

  using std::vector;

  class Node;

  /**
   * Base class for all compound nodes.
   * This class can be used
   * - to group other Nodes into "sub-networks"
   * - to construct Node classes which are composed of multiple 
   *   subnodes.
   */
  class Compound: public Node
  {
  public:

    Compound();   

    Compound(const Compound &);

    virtual ~Compound(){}
   
    void set_status(const DictionaryDatum&);
    void get_status(DictionaryDatum&) const;

    bool has_proxies() const;
          
    Node * at(index) const;
    Node * operator[](index) const;

    size_t size() const;
    bool   empty() const;
    void   reserve(size_t);

    void push_back(Node*);
    
    /**
     * Add a node to the compound.
     * This function adds a node to the compound and returns its local id.
     * The node is appended to the compound child-list. 
     */ 
    index add_node(Node *);

    /**
     * Return iterator to the first child node.
     */
    vector<Node*>::iterator begin();

    /**
     * Return iterator to the end of the child-list.
     */
    vector<Node*>::iterator end();

    /**
     * Return const iterator to the first child node.
     */
    vector<Node*>::const_iterator begin() const;

    /**
     * Return const iterator to the end of the child-list.
     */
    vector<Node*>::const_iterator end() const;

    /**
     * Return the compounds's user label.
     * Each compound can be given a user-defined string as a label, which
     * may be used to give a symbolic name to the node. From the SLI
     * level, the FindNodes command may be used to find a compound's
     * addess from it's label.
     */
    std::string get_label() const;
    
    /**
     * Set the compound's user label.
     * Each compound can be given a user-defined string as a label, which
     * may be used to give a symbolic name to the node. From the SLI
     * level, the FindNodes command may be used to find a compound's
     * addess from it's label. This sets the label for all nodes on the
     * same level (i.e. for all threads) simulataneously
     */
    void set_label(std::string const);

    
    /**
     * Set the compound's user label.
     * Each compound can be given a user-defined string as a label, which
     * may be used to give a symbolic name to the node. From the SLI
     * level, the FindNodes command may be used to find a compound's
     * addess from it's label. This does not set the label for the nodes
     * on other threads.
     */
    void set_label_non_recursive(std::string const);

    /**
     * Set the compound's custom dictionary.
     * Each compound can be given a user-defined dictionary, which
     * may be used to store additional data. From the SLI
     * level, the SetStatus command may be used to set a compound's
     * custom dictionary.
     */
    DictionaryDatum get_customdict() const;
    
    /**
     * Return pointer to the compound's custom dictionary.
     * Each compound contains a user-definable dictionary, which
     * may be used to store additional data. From the SLI
     * level, the SetStatus command may be used to set a compound's
     * custom dictionary.
     */
    void set_customdict(DictionaryDatum const dict);
    
    std::string print_network(int , int, std::string = "");
    void get_dimensions(std::vector<int>&) const;

    bool get_children_on_same_vp() const;
    void set_children_on_same_vp(bool);

    thread get_children_vp() const;
    void set_children_vp(thread);
    
    virtual bool allow_entry() const;

  protected:
    void init_state_(const Node&) {}
    void init_buffers_() {}

    void calibrate() {}
    void update(Time const &, const long_t, const long_t) {}
    
    /**
     * Pointer to child nodes.
     * This vector contains the pointers to the child nodes.
     * Since deletion of Nodes is possible, entries in this
     * vector may be NULL. Note that all code must handle
     * this case gracefully.
     */
    vector<Node *> nodes_;       //!< Pointer to child nodes.

    /**
     * flag indicating if all children of this compound have to
     * be created on the same virtual process or not. Use with
     * care. This may lead to severe performance problems!
     */
    bool children_on_same_vp_;
    thread children_vp_;
    
  private:
    std::string     label_;      //!< user-defined label for this node.
    DictionaryDatum customdict_; //!< user-defined dictionary for this node.
    // note that DictionaryDatum is a pointer and must be initialized in the constructor.
    bool homogeneous_;           //!< flag which indicates if the compound contains different kinds of models.
  };

  /**
   * Add a node to the compound.
   */
  inline
  index Compound::add_node(Node *n)
  {
    const index lid=nodes_.size();
    if ((homogeneous_) && (lid > 0))
      if (n->get_model_id() != (nodes_.at(lid - 1))->get_model_id())
	homogeneous_ = false;
    n->set_lid_(lid);
    nodes_.push_back(n);
    n->set_parent_(this);
    return lid;
  }

  inline
  void Compound::push_back(Node *n)
  {
    nodes_.push_back(n);
  }
  
  
  /** Index child node (with range check).
   * \throws std::out_of_range (implemented via \c std::vector)
   */
  inline
    Node* Compound::at(index i) const
  {
    return nodes_.at(i); //with range check
  }

  /** Index child node (without range check).
   */
  inline
  Node* Compound::operator [](index i) const
  {
    return nodes_[i]; //without range check
  }

  inline
  vector<Node*>::iterator Compound::begin()
  {
    return nodes_.begin();
  }

  inline
  vector<Node*>::iterator Compound::end()
  {
    return nodes_.end();
  }

  inline
  vector<Node*>::const_iterator Compound::begin() const
  {
    return nodes_.begin();
  }

  inline
  vector<Node*>::const_iterator Compound::end() const
  {
    return nodes_.end();
  }

  inline
  bool Compound::empty() const
  {
    return nodes_.empty();
  }

  inline
  size_t Compound::size() const
  {
    return nodes_.size();
  }

  inline 
  void Compound::reserve(size_t n)
  {
    nodes_.reserve(n);
  }

  inline 
  std::string Compound::get_label() const
  {
    return label_;
  }

  inline 
  DictionaryDatum Compound::get_customdict() const
  {
    return customdict_;
  }
  
  inline 
  void Compound::set_customdict(DictionaryDatum const d)
  {
    customdict_=d;
  }
  
  inline
  bool Compound::has_proxies() const
  {
    return false;
  }

  inline
  bool Compound::get_children_on_same_vp() const
  {
    return children_on_same_vp_;
  }

  inline
  void Compound::set_children_on_same_vp(bool children_on_same_vp)
  {
    children_on_same_vp_ = children_on_same_vp;
  }

  inline
  thread Compound::get_children_vp() const
  {
    return children_vp_;
  }

  inline
  void Compound::set_children_vp(thread children_vp)
  {
    children_vp_ = children_vp;
  }
  
} // namespace

#endif
