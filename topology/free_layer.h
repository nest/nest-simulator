#ifndef FREE_LAYER_H
#define FREE_LAYER_H

/*
 *  free_layer.h
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

#include <limits>
#include <sstream>
#include <algorithm>
#include "layer.h"
#include "topology_names.h"
#include "dictutils.h"
#include "ntree_impl.h"

namespace nest
{

  /**
   * Layer with free positioning of neurons, positions specified by user.
   */
  template <int D>
  class FreeLayer: public Layer<D>
  {
  public:
    Position<D> get_position(index lid) const;
    void set_status(const DictionaryDatum&);
    void get_status(DictionaryDatum&) const;

  protected:

    template <class Ins>
    void communicate_positions_(Ins iter, const Selector& filter);

    void insert_global_positions_ntree_(Ntree<D,index> & tree, const Selector& filter);
    void insert_global_positions_vector_(std::vector<std::pair<Position<D>,index> > & vec, const Selector& filter);
    void insert_local_positions_ntree_(Ntree<D,index> & tree, const Selector& filter);

    /// Vector of positions. Should match node vector in Subnet.
    std::vector<Position<D> > positions_;
  };

  template <int D>
  void FreeLayer<D>::set_status(const DictionaryDatum &d)
  {
    Layer<D>::set_status(d);

    // Read positions from dictionary
    if(d->known(names::positions)) {
      TokenArray pos = getValue<TokenArray>(d, names::positions);
      if(this->global_size()/this->depth_ != pos.size()) {
        std::stringstream expected;
        std::stringstream got;
        expected << "position array with length " << this->global_size()/this->depth_;
        got << "position array with length" << pos.size();
        throw TypeMismatch(expected.str(), got.str());
      }

      positions_.clear();
      positions_.reserve(this->local_size());

      const index nodes_per_depth = this->global_size()/this->depth_;
      const index first_lid = this->nodes_[0]->get_lid();

      for(vector<Node*>::iterator i = this->local_begin(); i != this->local_end(); ++i) {

        if ( ((*i)->get_lid() != first_lid) &&
             ((*i)->get_lid() % nodes_per_depth == first_lid ) ) {
          break;
        }

        Position<D> point =
          getValue<std::vector<double_t> >(pos[(*i)->get_lid() % nodes_per_depth]);

        if (not ((point >= this->lower_left_) and (point < this->lower_left_+this->extent_)))
          throw BadProperty("Node position outside of layer");

        positions_.push_back(point);

      }

    }
  }

  template <int D>
  void FreeLayer<D>::get_status(DictionaryDatum &d) const
  {
    Layer<D>::get_status(d);

    DictionaryDatum topology_dict = getValue<DictionaryDatum>((*d)[names::topology]);

    TokenArray points;
    for(typename std::vector<Position<D> >::const_iterator it = positions_.begin();
        it != positions_.end(); ++it) {
      points.push_back(it->getToken());
    }
    def2<TokenArray, ArrayDatum>(topology_dict, names::positions, points);

  }

  template <int D>
  Position<D> FreeLayer<D>::get_position(index lid) const
  {
    return positions_[lid % positions_.size()];
  }

  template <int D>
  template <class Ins>
  void FreeLayer<D>::communicate_positions_(Ins iter, const Selector& filter)
  {
    assert(this->nodes_.size() >= positions_.size());
    
    std::vector<double_t> local_gid_pos;
    std::vector<Node*>::const_iterator nodes_begin;
    std::vector<Node*>::const_iterator nodes_end;

    if (filter.select_depth()) {
      local_gid_pos.reserve((D+1)*(this->nodes_.size()/this->depth_ + 1));
      nodes_begin = this->local_begin(filter.depth);
      nodes_end = this->local_end(filter.depth);
    } else {
      local_gid_pos.reserve((D+1)*this->nodes_.size());
      nodes_begin = this->local_begin();
      nodes_end = this->local_end();
    }

    for(std::vector<Node*>::const_iterator node_it = nodes_begin; node_it != nodes_end; ++node_it) {

      if (filter.select_model() && ((*node_it)->get_model_id() != filter.model))
        continue;

      local_gid_pos.push_back((*node_it)->get_gid());
      for(int j=0;j<D;++j)
        local_gid_pos.push_back(positions_[(*node_it)->get_subnet_index() % positions_.size()][j]);
    }

    std::vector<double_t> global_gid_pos;
    std::vector<int> displacements;
    Communicator::communicate(local_gid_pos,global_gid_pos,displacements);

    for(index i = 0; i<global_gid_pos.size(); i+=D+1) {
      *iter++ = std::pair<Position<D>,index>(&global_gid_pos[i+1], global_gid_pos[i]);
    }

  }

  template <int D>
  void FreeLayer<D>::insert_global_positions_ntree_(Ntree<D,index> & tree, const Selector& filter)
  {

    communicate_positions_(std::inserter(tree, tree.end()), filter);

  }

  template <int D>
  void FreeLayer<D>::insert_local_positions_ntree_(Ntree<D,index> & tree, const Selector& filter)
  {
    assert(this->nodes_.size() >= positions_.size());
    
    std::vector<Node*>::const_iterator nodes_begin;
    std::vector<Node*>::const_iterator nodes_end;

    if (filter.select_depth()) {
      nodes_begin = this->local_begin(filter.depth);
      nodes_end = this->local_end(filter.depth);
    } else {
      nodes_begin = this->local_begin();
      nodes_end = this->local_end();
    }

    for(std::vector<Node*>::const_iterator node_it = nodes_begin; node_it != nodes_end; ++node_it) {

      if (filter.select_model() && ((*node_it)->get_model_id() != filter.model))
        continue;

      tree.insert(std::pair<Position<D>,index>(positions_[(*node_it)->get_subnet_index() % positions_.size()],(*node_it)->get_gid()));
    }

  }

  template<int D>
  static bool gid_less(const std::pair<Position<D>,index>& a, const std::pair<Position<D>,index>& b)
  {
    return a.second < b.second;
  }

  template <int D>
  void FreeLayer<D>::insert_global_positions_vector_(std::vector<std::pair<Position<D>,index> > & vec, const Selector& filter)
  {

    communicate_positions_(std::back_inserter(vec), filter);

    // Sort vector to ensure consistent results
    std::sort(vec.begin(),vec.end(),gid_less<D>);

  }

} // namespace nest

#endif
