#ifndef CONNECTION_CREATOR_IMPL_H
#define CONNECTION_CREATOR_IMPL_H

/*
 *  connection_creator_impl.h
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

#include <vector>
#include "connection_creator.h"
#include "binomial_randomdev.h"

namespace nest
{
  template<int D>
  void ConnectionCreator::connect(Layer<D>& source, Layer<D>& target)
  {
    switch (type_) {
    case Target_driven:

      target_driven_connect_(source, target);
      break;

    case Convergent:

      convergent_connect_(source, target);
      break;

    case Divergent:

      divergent_connect_(source, target);
      break;

    case Source_driven:

      source_driven_connect_(source, target);
      break;

    default:
      throw BadProperty("Unknown connection type.");
    }

  }

  template<int D>
  void ConnectionCreator::get_parameters_(const Position<D> & pos, librandom::RngPtr rng, DictionaryDatum d)
  {
    for(ParameterMap::iterator iter=parameters_.begin(); iter != parameters_.end(); ++iter) {
      def<double_t>(d, iter->first, iter->second->value(pos, rng));
    }
  }

  template<int D>
  void ConnectionCreator::target_driven_connect_(Layer<D>& source, Layer<D>& target)
  {
    // Target driven connect
    // For each local target node:
    //  1. Apply Mask to source layer
    //  2. For each source node: Compute probability, draw random number, make
    //     connection conditionally

    DictionaryDatum d = new Dictionary();

    // Nodes in the subnet are grouped by depth, so to select by depth, we
    // just adjust the begin and end pointers:
    std::vector<Node*>::const_iterator target_begin;
    std::vector<Node*>::const_iterator target_end;
    if (target_filter_.select_depth()) {
      target_begin = target.local_begin(target_filter_.depth);
      target_end = target.local_end(target_filter_.depth);
    } else {
      target_begin = target.local_begin();
      target_end = target.local_end();
    }

    if (mask_.valid()) {

      // Retrieve global positions:
      MaskedLayer<D> masked_layer(source,source_filter_,mask_,true,allow_oversized_);

      for (std::vector<Node*>::const_iterator tgt_it = target_begin;tgt_it != target_end;++tgt_it) {

        if (target_filter_.select_model() && ((*tgt_it)->get_model_id() != target_filter_.model))
          continue;

        index target_id = (*tgt_it)->get_gid();
        librandom::RngPtr rng = net_.get_rng((*tgt_it)->get_thread());
        Position<D> target_pos = target.get_position((*tgt_it)->get_subnet_index());

        // If there is a kernel, we create connections conditionally,
        // otherwise all sources within the mask are created. Test moved
        // outside the loop for efficiency.
        if (kernel_.valid()) {

          for(typename Ntree<D,index>::masked_iterator iter=masked_layer.begin(target_pos); iter!=masked_layer.end(); ++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            if (rng->drand() < kernel_->value(source.compute_displacement(target_pos,iter->first), rng)) {
              get_parameters_(source.compute_displacement(target_pos,iter->first), rng, d);
              net_.connect(iter->second,target_id,d,synapse_model_);
            }

          }

        } else {

          // no kernel

          for(typename Ntree<D,index>::masked_iterator iter=masked_layer.begin(target_pos); iter!=masked_layer.end(); ++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            get_parameters_(source.compute_displacement(target_pos,iter->first), rng, d);
            net_.connect(iter->second,target_id,d,synapse_model_);
          }

        }

      }

    } else {
      // no mask

      std::vector<std::pair<Position<D>,index> >* positions = source.get_global_positions_vector(source_filter_);
      for (std::vector<Node*>::const_iterator tgt_it = target_begin;tgt_it != target_end;++tgt_it) {

        if (target_filter_.select_model() && ((*tgt_it)->get_model_id() != target_filter_.model))
          continue;

        index target_id = (*tgt_it)->get_gid();
        librandom::RngPtr rng = net_.get_rng((*tgt_it)->get_thread());
        Position<D> target_pos = target.get_position((*tgt_it)->get_subnet_index());

        // If there is a kernel, we create connections conditionally,
        // otherwise all sources within the mask are created. Test moved
        // outside the loop for efficiency.
        if (kernel_.valid()) {

          for(typename std::vector<std::pair<Position<D>,index> >::iterator iter=positions->begin();iter!=positions->end();++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            if (rng->drand() < kernel_->value(source.compute_displacement(target_pos,iter->first), rng)) {
              get_parameters_(source.compute_displacement(target_pos,iter->first), rng, d);
              net_.connect(iter->second,target_id,d,synapse_model_);
            }
          }

        } else {

          for(typename std::vector<std::pair<Position<D>,index> >::iterator iter=positions->begin();iter!=positions->end();++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            get_parameters_(source.compute_displacement(target_pos,iter->first), rng, d);
            net_.connect(iter->second,target_id,d,synapse_model_);
          }

        }
      }
    }

  }

  template<int D>
  void ConnectionCreator::source_driven_connect_(Layer<D>& source, Layer<D>& target)
  {
    // Source driven connect is actually implemented as target driven,
    // but with displacements computed in the target layer. The Mask has been
    // reversed so that it can be applied to the source instead of the target.
    // For each local target node:
    //  1. Apply (Converse)Mask to source layer
    //  2. For each source node: Compute probability, draw random number, make
    //     connection conditionally

    DictionaryDatum d = new Dictionary();

    // Nodes in the subnet are grouped by depth, so to select by depth, we
    // just adjust the begin and end pointers:
    std::vector<Node*>::const_iterator target_begin;
    std::vector<Node*>::const_iterator target_end;
    if (target_filter_.select_depth()) {
      target_begin = target.local_begin(target_filter_.depth);
      target_end = target.local_end(target_filter_.depth);
    } else {
      target_begin = target.local_begin();
      target_end = target.local_end();
    }

    if (mask_.valid()) {

      // By supplying the target layer to the MaskedLayer constructor, the
      // mask is mirrored so it may be applied to the source layer instead
      MaskedLayer<D> masked_layer(source,source_filter_,mask_,true,allow_oversized_,target);

      for (std::vector<Node*>::const_iterator tgt_it = target_begin;tgt_it != target_end;++tgt_it) {

        if (target_filter_.select_model() && ((*tgt_it)->get_model_id() != target_filter_.model))
          continue;

        index target_id = (*tgt_it)->get_gid();
        librandom::RngPtr rng = net_.get_rng((*tgt_it)->get_thread());
        Position<D> target_pos = target.get_position((*tgt_it)->get_subnet_index());

        // If there is a kernel, we create connections conditionally,
        // otherwise all sources within the mask are created. Test moved
        // outside the loop for efficiency.
        if (kernel_.valid()) {

          for(typename Ntree<D,index>::masked_iterator iter=masked_layer.begin(target_pos); iter!=masked_layer.end(); ++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            if (rng->drand() < kernel_->value(target.compute_displacement(iter->first, target_pos), rng)) {
              get_parameters_(target.compute_displacement(iter->first, target_pos), rng, d);
              net_.connect(iter->second,target_id,d,synapse_model_);
            }

          }

        } else {

          // no kernel

          for(typename Ntree<D,index>::masked_iterator iter=masked_layer.begin(target_pos); iter!=masked_layer.end(); ++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            get_parameters_(target.compute_displacement(iter->first,target_pos), rng, d);
            net_.connect(iter->second,target_id,d,synapse_model_);
          }

        }

      }

    } else {
      // no mask

      std::vector<std::pair<Position<D>,index> >* positions = source.get_global_positions_vector(source_filter_);
      for (std::vector<Node*>::const_iterator tgt_it = target_begin;tgt_it != target_end;++tgt_it) {

        if (target_filter_.select_model() && ((*tgt_it)->get_model_id() != target_filter_.model))
          continue;

        index target_id = (*tgt_it)->get_gid();
        librandom::RngPtr rng = net_.get_rng((*tgt_it)->get_thread());
        Position<D> target_pos = target.get_position((*tgt_it)->get_subnet_index());

        // If there is a kernel, we create connections conditionally,
        // otherwise all sources within the mask are created. Test moved
        // outside the loop for efficiency.
        if (kernel_.valid()) {

          for(typename std::vector<std::pair<Position<D>,index> >::iterator iter=positions->begin();iter!=positions->end();++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            if (rng->drand() < kernel_->value(target.compute_displacement(iter->first,target_pos), rng)) {
              get_parameters_(target.compute_displacement(iter->first,target_pos), rng, d);
              net_.connect(iter->second,target_id,d,synapse_model_);
            }
          }

        } else {

          for(typename std::vector<std::pair<Position<D>,index> >::iterator iter=positions->begin();iter!=positions->end();++iter) {

            if ((not allow_autapses_) and (iter->second == target_id))
              continue;

            get_parameters_(target.compute_displacement(iter->first,target_pos), rng, d);
            net_.connect(iter->second,target_id,d,synapse_model_);
          }

        }
      }
    }

  }

  template<int D>
  void ConnectionCreator::convergent_connect_(Layer<D>& source, Layer<D>& target)
  {
    // Convergent connections (fixed fan in)
    //
    // For each local target node:
    // 1. Apply Mask to source layer
    // 2. Compute connection probability for each source position
    // 3. Draw source nodes and make connections

    DictionaryDatum d = new Dictionary();

    // Nodes in the subnet are grouped by depth, so to select by depth, we
    // just adjust the begin and end pointers:
    std::vector<Node*>::const_iterator target_begin;
    std::vector<Node*>::const_iterator target_end;
    if (target_filter_.select_depth()) {
      target_begin = target.local_begin(target_filter_.depth);
      target_end = target.local_end(target_filter_.depth);
    } else {
      target_begin = target.local_begin();
      target_end = target.local_end();
    }

    if (mask_.valid()) {

      for (std::vector<Node*>::const_iterator tgt_it = target_begin;tgt_it != target_end;++tgt_it) {

        if (target_filter_.select_model() && ((*tgt_it)->get_model_id() != target_filter_.model))
          continue;

        index target_id = (*tgt_it)->get_gid();
        librandom::RngPtr rng = net_.get_rng((*tgt_it)->get_thread());
        Position<D> target_pos = target.get_position((*tgt_it)->get_subnet_index());

        // Get (position,GID) pairs for sources inside mask
        std::vector<std::pair<Position<D>,index> > positions =
            source.get_global_positions_vector(source_filter_, mask_,
                                               target.get_position((*tgt_it)->get_subnet_index()),
                                               allow_oversized_);

        // We will select `number_of_connections_` sources within the mask.
        // If there is no kernel, we can just draw uniform random numbers,
        // but with a kernel we have to set up a probability distribution
        // function using the Vose class.
        if (kernel_.valid()) {

          std::vector<double_t> probabilities;

          // Collect probabilities for the sources
          for(typename std::vector<std::pair<Position<D>,index> >::iterator iter=positions.begin();iter!=positions.end();++iter) {

              probabilities.push_back(kernel_->value(source.compute_displacement(target_pos,iter->first), rng));

          }

          if ((positions.size()==0) or
              ((not allow_autapses_) and (positions.size()==1) and (positions[0].second==target_id)) or
              ((not allow_multapses_) and (positions.size()<number_of_connections_)) ) {
            std::string msg = String::compose("Global target ID %1: Not enough sources found inside mask", target_id);
            throw KernelException(msg.c_str());
          }

          // A Vose object draws random integers with a non-uniform
          // distribution.
          Vose lottery(probabilities);

          // If multapses are not allowed, we must keep track of which
          // sources have been selected already.
          std::vector<bool> is_selected(positions.size());

          // Draw `number_of_connections_` sources
          for(int i=0;i<(int)number_of_connections_;++i) {
            index random_id = lottery.get_random_id(rng);
            if ((not allow_multapses_) and (is_selected[random_id])) {
              --i;
              continue;
            }

            index source_id = positions[random_id].second;
            if ((not allow_autapses_) and (source_id == target_id)) {
              --i;
              continue;
            }

            get_parameters_(source.compute_displacement(target_pos,positions[random_id].first), rng, d);
            net_.connect(source_id, target_id, d, synapse_model_);
            is_selected[random_id] = true;
          }

        } else {

          // no kernel

          if ((positions.size()==0) or
              ((not allow_autapses_) and (positions.size()==1) and (positions[0].second==target_id)) or
              ((not allow_multapses_) and (positions.size()<number_of_connections_)) ) {
            std::string msg = String::compose("Global target ID %1: Not enough sources found inside mask", target_id);
            throw KernelException(msg.c_str());
          }

          // If multapses are not allowed, we must keep track of which
          // sources have been selected already.
          std::vector<bool> is_selected(positions.size());

          // Draw `number_of_connections_` sources
          for(int i=0;i<(int)number_of_connections_;++i) {
            index random_id = rng->ulrand(positions.size());
            if ((not allow_multapses_) and (is_selected[random_id])) {
              --i;
              continue;
            }
            index source_id = positions[random_id].second;
            get_parameters_(source.compute_displacement(target_pos,positions[random_id].first), rng, d);
            net_.connect(source_id, target_id, d, synapse_model_);
            is_selected[random_id] = true;
          }

        }

      }

    } else {
      // no mask

      // Get (position,GID) pairs for all nodes in source layer
      std::vector<std::pair<Position<D>,index> >* positions = source.get_global_positions_vector(source_filter_);

      for (std::vector<Node*>::const_iterator tgt_it = target_begin;tgt_it != target_end;++tgt_it) {

        if (target_filter_.select_model() && ((*tgt_it)->get_model_id() != target_filter_.model))
          continue;

        index target_id = (*tgt_it)->get_gid();
        librandom::RngPtr rng = net_.get_rng((*tgt_it)->get_thread());
        Position<D> target_pos = target.get_position((*tgt_it)->get_subnet_index());

        if ( (positions->size()==0) or
             ((not allow_autapses_) and (positions->size()==1) and ((*positions)[0].second==target_id)) or
             ((not allow_multapses_) and (positions->size()<number_of_connections_)) ) {
          std::string msg = String::compose("Global target ID %1: Not enough sources found", target_id);
          throw KernelException(msg.c_str());
        }

        // We will select `number_of_connections_` sources within the mask.
        // If there is no kernel, we can just draw uniform random numbers,
        // but with a kernel we have to set up a probability distribution
        // function using the Vose class.
        if (kernel_.valid()) {

          std::vector<double_t> probabilities;

          // Collect probabilities for the sources
          for(typename std::vector<std::pair<Position<D>,index> >::iterator iter=positions->begin();iter!=positions->end();++iter) {
            probabilities.push_back(kernel_->value(source.compute_displacement(target_pos,iter->first), rng));
          }

          // A Vose object draws random integers with a non-uniform
          // distribution.
          Vose lottery(probabilities);

          // If multapses are not allowed, we must keep track of which
          // sources have been selected already.
          std::vector<bool> is_selected(positions->size());

          // Draw `number_of_connections_` sources
          for(int i=0;i<(int)number_of_connections_;++i) {
            index random_id = lottery.get_random_id(rng);
            if ((not allow_multapses_) and (is_selected[random_id])) {
              --i;
              continue;
            }

            index source_id = (*positions)[random_id].second;
            if ((not allow_autapses_) and (source_id == target_id)) {
              --i;
              continue;
            }

            Position<D> source_pos = (*positions)[random_id].first;
            get_parameters_(source.compute_displacement(target_pos,source_pos), rng, d);
            net_.connect(source_id, target_id, d, synapse_model_);
            is_selected[random_id] = true;
          }

        } else {

          // no kernel

          // If multapses are not allowed, we must keep track of which
          // sources have been selected already.
          std::vector<bool> is_selected(positions->size());

          // Draw `number_of_connections_` sources
          for(int i=0;i<(int)number_of_connections_;++i) {
            index random_id = rng->ulrand(positions->size());
            if ((not allow_multapses_) and (is_selected[random_id])) {
              --i;
              continue;
            }

            index source_id = (*positions)[random_id].second;
            if ((not allow_autapses_) and (source_id == target_id)) {
              --i;
              continue;
            }

            Position<D> source_pos = (*positions)[random_id].first;
            get_parameters_(source.compute_displacement(target_pos,source_pos), rng, d);
            net_.connect(source_id, target_id, d, synapse_model_);
            is_selected[random_id] = true;
          }

        }

      }
    }
  }


  template<int D>
  void ConnectionCreator::divergent_connect_(Layer<D>& source, Layer<D>& target)
  {
    // Divergent connections (fixed fan out)
    //
    // For each (global) source: (All connections made on all mpi procs)
    // 1. Apply mask to global targets
    // 2. If using kernel: Compute connection probability for each global target
    // 3. Draw connections to make using global rng

    MaskedLayer<D> masked_target(target,target_filter_,mask_,true,allow_oversized_);

    std::vector<std::pair<Position<D>,index> >* sources = source.get_global_positions_vector(source_filter_);
    DictionaryDatum d = new Dictionary();

    for (typename std::vector<std::pair<Position<D>,index> >::iterator src_it = sources->begin(); src_it != sources->end(); ++src_it) {

      Position<D> source_pos = src_it->first;
      index source_id = src_it->second;
      std::vector<index> targets;
      std::vector<Position<D> > displacements;
      std::vector<double_t> probabilities;

      // Find potential targets and probabilities

      for(typename Ntree<D,index>::masked_iterator tgt_it=masked_target.begin(source_pos); tgt_it!=masked_target.end(); ++tgt_it) {

        if ((not allow_autapses_) and (source_id == tgt_it->second))
          continue;

        Position<D> target_displ = target.compute_displacement(source_pos, tgt_it->first);
        librandom::RngPtr rng = net_.get_grng();

        targets.push_back(tgt_it->second);
        displacements.push_back(target_displ);

        if (kernel_.valid())
          probabilities.push_back(kernel_->value(target_displ, rng));
        else
          probabilities.push_back(1.0);
      }

      if ((targets.size()==0) or
          ((not allow_multapses_) and (targets.size()<number_of_connections_)) ) {
        std::string msg = String::compose("Global source ID %1: Not enough targets found", source_id);
        throw KernelException(msg.c_str());
      }

      // Draw targets.  A Vose object draws random integers with a
      // non-uniform distribution.
      Vose lottery(probabilities);

      // If multapses are not allowed, we must keep track of which
      // targets have been selected already.
      std::vector<bool> is_selected(targets.size());

      // Draw `number_of_connections_` targets
      for(long_t i=0;i<(long_t)number_of_connections_;++i) {
        index random_id = lottery.get_random_id(net_.get_grng());
        if ((not allow_multapses_) and (is_selected[random_id])) {
          --i;
          continue;
        }
        Position<D> target_displ = displacements[random_id];
        index target_id = targets[random_id];
        get_parameters_(target_displ, net_.get_grng(), d);
        net_.connect(source_id, target_id, d, synapse_model_);
        is_selected[random_id] = true;
      }

    }

  }

} // namespace nest

#endif
