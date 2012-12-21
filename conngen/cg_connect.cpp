/*
 *  cg_connect.cpp
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

#include "cg_connect.h"

#include "network.h"
#include "communicator.h"

namespace nest 
{
  void cg_connect(ConnectionGeneratorDatum& cg, RangeSet& sources, index source_offset, RangeSet& targets, index target_offset, DictionaryDatum params_map, index syn)
  {
    cg_set_masks(cg, sources, targets);
    cg->start();

    int source, target, num_parameters = cg->arity();
    if (num_parameters == 0)
    {
      while (cg->next(source, target, NULL))
        ConnectionGeneratorModule::get_network().connect(source + source_offset, target + target_offset, syn);
    }
    else if (num_parameters == 2)
    {
      if (!params_map->known(names::weight) || !params_map->known(names::delay))
        throw BadProperty("The parameter map has to contain the indices of weight and delay.");  

      long w_idx = (*params_map)[names::weight];
      long d_idx = (*params_map)[names::delay];
      std::vector<double> params(2);

      while (cg->next(source, target, &params[0]))
        ConnectionGeneratorModule::get_network().connect(source + source_offset, target + target_offset, params[w_idx], params[d_idx], syn);
    }
    else
    {
      ConnectionGeneratorModule::get_network().message(SLIInterpreter::M_ERROR, "Connect", "Either two or no parameters in the Connection Set expected.");
      throw DimensionMismatch();  
    }
  }
  
  void cg_connect(ConnectionGeneratorDatum& cg, RangeSet& sources, std::vector<long>& source_gids, RangeSet& targets, std::vector<long>& target_gids, DictionaryDatum params_map, index syn)
  {
    cg_set_masks(cg, sources, targets);
    cg->start();

    int source, target, num_parameters = cg->arity();
    if (num_parameters == 0)
    {
      while (cg->next(source, target, NULL))
        ConnectionGeneratorModule::get_network().connect(source_gids.at(source), target_gids.at(target), syn);
    }
    else if (num_parameters == 2)
    {
      if (!params_map->known(names::weight) || !params_map->known(names::delay))
        throw BadProperty("The parameter map has to contain the indices of weight and delay.");  

      long w_idx = (*params_map)[names::weight];
      long d_idx = (*params_map)[names::delay];
      std::vector<double> params(2);

      while (cg->next(source, target, &params[0]))
        ConnectionGeneratorModule::get_network().connect(source_gids.at(source), target_gids.at(target), params[w_idx], params[d_idx], syn);
    }
    else
    {
      ConnectionGeneratorModule::get_network().message(SLIInterpreter::M_ERROR, "Connect", "Either two or no parameters in the Connection Set expected.");
      throw DimensionMismatch();  
    }
  }

  void cg_set_masks(ConnectionGeneratorDatum& cg, RangeSet& sources, RangeSet& targets)
  {
    std::vector<ConnectionGenerator::Mask> masks(Communicator::get_num_processes());
    cg_create_masks(&masks, sources, targets);

    cg->setMask(masks, Communicator::get_rank());
  }

  void cg_create_masks(std::vector<ConnectionGenerator::Mask>* masks, RangeSet& sources, RangeSet& targets)
  {
    // We need to do some index translation here as the CG expects
    // indices from 0..n for both source and target populations.

    size_t length = 0;
    for (RangeSet::iterator source = sources.begin(); source != sources.end(); ++source)
    {
      for (size_t proc = 0; proc < static_cast<size_t>(Communicator::get_num_processes()); ++proc)
      {
        size_t last = source->last - source->first;
        if (proc <= last)
        {
          size_t left = proc + length;
          size_t right = last + length;
          (*masks)[(proc + source->first) % Communicator::get_num_processes()].sources.insert(left, right);
        }
      }
      length += source->last - source->first + 1;
    }

    length = 0;
    for (RangeSet::iterator target = targets.begin(); target != targets.end(); ++target)
    {
      for (size_t proc = 0; proc < static_cast<size_t>(Communicator::get_num_processes()); ++proc)
      {
        size_t last = target->last - target->first;
        if (proc <= last)
        {
          size_t left = proc + length;
          size_t right = last + length;
          (*masks)[(proc + target->first) % Communicator::get_num_processes()].targets.insert(left, right);
        }
      }
      length += target->last - target->first + 1;
    }
  }

  index cg_get_right_border(index left, size_t step, std::vector<long>& gids)
  {
    if (left == gids.size() -1)
      return left;
    
    long leftmost_r = -1;
    long i = gids.size() - 1, last_i = gids.size() -1;

    long right = -1;
    while(true)
    {
      if ((i == static_cast<long>(gids.size()) - 1 && gids[i] - gids[left] == i - static_cast<long>(left)) || i == leftmost_r) 
        return last_i;

      last_i = i;
      if (gids[i] - gids[left] == i - static_cast<long>(left))
        i += step;
      else
      {
        leftmost_r = i;
        i -= step;
      }

      if (step != 1)
          step /= 2;
    }
    return right;
  }

  void cg_get_ranges(RangeSet& ranges, std::vector<long>& gids)
  {
    index right = 0, left = 0;
    while(true)
    {
      right = cg_get_right_border(left, (gids.size() - left) / 2, gids);
      ranges.push_back(Range(gids[left], gids[right]));
      if (right == gids.size() - 1)
        break;
      else
        left = right + 1;
    }
  }
  

} // namespace nest
