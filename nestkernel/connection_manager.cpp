/*
 *  connection_manager.cpp
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

#include "connection_manager.h"
#include "connector_model.h"
#include "network.h"
#include "nest_time.h"
#include "connectiondatum.h"

namespace nest
{

ConnectionManager::ConnectionManager(Network& net)
        : net_(net)
{}

ConnectionManager::~ConnectionManager()
{
  delete_connections_();  
  clear_prototypes_();

  for (std::vector<ConnectorModel*>::iterator i = pristine_prototypes_.begin(); i != pristine_prototypes_.end(); ++i)
    if (*i != 0)
        delete *i;
}

void ConnectionManager::init(Dictionary* synapsedict)
{
  synapsedict_ = synapsedict;
  init_();
}

void ConnectionManager::init_()
{
  synapsedict_->clear();

  // (re-)append all synapse prototypes
  for (std::vector<ConnectorModel*>::iterator i = pristine_prototypes_.begin(); i != pristine_prototypes_.end(); ++i )
    if (*i != 0)
    {
      std::string name = (*i)->get_name();
      prototypes_.push_back((*i)->clone(name));
      synapsedict_->insert(name, prototypes_.size() - 1);
    }

  std::vector<std::vector<std::vector<Connector*> > > tmp(
      net_.get_num_threads(), std::vector<std::vector<Connector*> >(
          1, std::vector<Connector*>(prototypes_.size(), 0)));
  connections_.swap(tmp);
}

void ConnectionManager::delete_connections_()
{
  for (tVVVConnector::iterator it = connections_.begin(); it != connections_.end(); ++it)
    for (tVVConnector::iterator iit = it->begin(); iit != it->end(); ++iit)
      for ( tVConnector::iterator iiit = iit->begin(); iiit != iit->end(); ++iiit)
	delete *iiit;
}

void ConnectionManager::clear_prototypes_()
{
  for (std::vector<ConnectorModel*>::iterator pt = prototypes_.begin(); pt != prototypes_.end(); ++pt)
    if (*pt != 0)
      delete *pt;
  prototypes_.clear();
}
  
void ConnectionManager::reset()
{
  delete_connections_();
  clear_prototypes_();
  init_();
}

bool ConnectionManager::synapse_prototype_in_use(index syn_id)
{
  if (syn_id < prototypes_.size() && prototypes_[syn_id] != 0)
    return prototypes_[syn_id]->get_num_connectors() > 0;
  else
    throw UnknownSynapseType(syn_id);

  return false;
}

index ConnectionManager::register_synapse_prototype(ConnectorModel * cf)
{
  std::string name = cf->get_name();
  
  if ( synapsedict_->known(name) )
  {
    delete cf;
    throw NamingConflict("A synapse type called '" + name + "' already exists.\n"
                         "Please choose a different name!");
  }
  

  pristine_prototypes_.push_back(cf);
  prototypes_.push_back(cf->clone(name));
  const index id = prototypes_.size() - 1;

  synapsedict_->insert(name, id);

  return id;
}

void ConnectionManager::unregister_synapse_prototype(index syn_id)
{
  const std::string name = get_synapse_prototype(syn_id).get_name();

  if (synapse_prototype_in_use(syn_id))
    throw ModelInUse(name); 
  
  synapsedict_->erase(name);

  // unregister the synapse prototype from the pristine_prototypes_ list
  delete pristine_prototypes_[syn_id];
  pristine_prototypes_[syn_id] = 0;

  // unregister the synapse prototype from the prototypes_ list
  delete prototypes_[syn_id];
  prototypes_[syn_id] = 0;
}

void ConnectionManager::try_unregister_synapse_prototype(index syn_id)
{
  const std::string name = get_synapse_prototype(syn_id).get_name();

  if ( synapse_prototype_in_use(syn_id) )
    throw ModelInUse(name); 
}

void ConnectionManager::calibrate(const TimeConverter & tc)
{
  for (std::vector<ConnectorModel*>::iterator pt = prototypes_.begin(); pt != prototypes_.end(); ++pt)
    if (*pt != 0)
      (*pt)->calibrate(tc);
}

const Time ConnectionManager::get_min_delay() const
{
  Time min_delay = Time::pos_inf();

  std::vector<ConnectorModel*>::const_iterator it;
  for (it = prototypes_.begin(); it != prototypes_.end(); ++it)
    if (*it != 0 && (*it)->get_num_connections() > 0)
      min_delay = std::min( min_delay, (*it)->get_min_delay() );

  if (min_delay == Time::pos_inf())
    min_delay = Time::get_resolution();

  return min_delay;
} 

const Time ConnectionManager::get_max_delay() const
{
  Time max_delay = Time::get_resolution();

  std::vector<ConnectorModel*>::const_iterator it;
  for (it = prototypes_.begin(); it != prototypes_.end(); ++it)
    if (*it != 0 && (*it)->get_num_connections() > 0)
      max_delay = std::max( max_delay, (*it)->get_max_delay() );

  return max_delay;
}

void ConnectionManager::validate_connector(thread tid, index gid, index syn_id)
{
  assert_valid_syn_id(syn_id);

  if (connections_[tid].size() < net_.size())
    connections_[tid].resize(net_.size());
  
  if (connections_[tid][gid].size() < prototypes_.size())
    connections_[tid][gid].resize(prototypes_.size(), 0);

  if (connections_[tid][gid][syn_id] == 0)
  {
    assert (prototypes_[syn_id] != 0);
    connections_[tid][gid][syn_id] = prototypes_[syn_id]->get_connector();
  }
}

index ConnectionManager::copy_synapse_prototype(index old_id, std::string new_name)
{
  // we can assert here, as nestmodule checks this for us
  assert (! synapsedict_->known(new_name));

  ConnectorModel* new_prototype = get_synapse_prototype(old_id).clone(new_name);
  prototypes_.push_back(new_prototype);
  int new_id = prototypes_.size() - 1;
  synapsedict_->insert(new_name, new_id);
  return new_id;
}

void ConnectionManager::get_status(DictionaryDatum& d) const
{
  def<long>(d, "num_connections", get_num_connections());
}

void ConnectionManager::set_prototype_status(index syn_id, const DictionaryDatum& d)
{
  assert_valid_syn_id(syn_id);

  prototypes_[syn_id]->set_status(d);
}

DictionaryDatum ConnectionManager::get_prototype_status(index syn_id) const
{
  assert_valid_syn_id(syn_id);

  DictionaryDatum dict(new Dictionary);
  prototypes_[syn_id]->get_status(dict);
  return dict;
}

DictionaryDatum ConnectionManager::get_synapse_status(index gid, index syn_id, port p, thread tid)
{
  assert_valid_syn_id(syn_id);
  validate_connector(tid, gid, syn_id);

  DictionaryDatum dict(new Dictionary);
  connections_[tid][gid][syn_id]->get_synapse_status(dict, p);
  (*dict)[names::source] = gid;
  (*dict)[names::synapse_type] = LiteralDatum(get_synapse_prototype(syn_id).get_name());

  return dict;
}

void ConnectionManager::set_synapse_status(index gid, index syn_id, port p, thread tid, const DictionaryDatum& dict)
{
  assert_valid_syn_id(syn_id);
  validate_connector(tid, gid, syn_id);
  connections_[tid][gid][syn_id]->set_synapse_status(dict, p);
}

DictionaryDatum ConnectionManager::get_connector_status(const Node& node, index syn_id)
{
  assert_valid_syn_id(syn_id);

  DictionaryDatum dict(new Dictionary);
  index gid = node.get_gid();
  for (thread tid = 0; tid < net_.get_num_threads(); tid++)
  {
    validate_connector(tid, gid, syn_id);
    connections_[tid][gid][syn_id]->get_status(dict);
  }
  return dict;
}

void ConnectionManager::set_connector_status(Node& node, index syn_id, thread tid, const DictionaryDatum& dict)
{
  assert_valid_syn_id(syn_id);

  index gid = node.get_gid();
  validate_connector(tid, gid, syn_id);
  connections_[tid][gid][syn_id]->set_status(dict);
}


ArrayDatum ConnectionManager::find_connections(DictionaryDatum params)
{
  ArrayDatum connectome;
  
  long source;
  bool have_source = updateValue<long>(params, names::source, source);
  if (have_source)
    net_.get_node(source); // This throws if the node does not exist
  else
    throw UndefinedName(names::source.toString());
  
  long target;
  bool have_target = updateValue<long>(params, names::target, target);
  if (have_target)
    net_.get_node(target); // This throws if the node does not exist

  size_t syn_id = 0;
  Name synmodel_name;
  bool have_synmodel = updateValue<std::string>(params, names::synapse_type, synmodel_name);

  if (have_synmodel)
  {
    const Token synmodel = synapsedict_->lookup(synmodel_name);
    if (!synmodel.empty())
      syn_id = static_cast<long>(synmodel);
    else
      throw UnknownModelName(synmodel_name.toString());
  }

  for (thread t = 0; t < net_.get_num_threads(); ++t)
  {
    if (have_synmodel)
    {
      if (static_cast<size_t>(source) < connections_[t].size() && syn_id < connections_[t][source].size() && connections_[t][source][syn_id] != 0)
          find_connections(connectome, t, source, syn_id, params);
    }
    else
    {
      if (static_cast<size_t>(source) < connections_[t].size())
        for (syn_id = 0; syn_id < connections_[t][source].size(); ++syn_id)
          if (connections_[t][source][syn_id] != 0)
            find_connections(connectome, t, source, syn_id, params);
    }
  }
  
  return connectome;
}

void ConnectionManager::find_connections(ArrayDatum& connectome, thread t, index source, index syn_id, DictionaryDatum params)
{
  std::vector<long>* p = connections_[t][source][syn_id]->find_connections(params);
  for (size_t i = 0; i < p->size(); ++i)
    connectome.push_back(ConnectionDatum(ConnectionID(source, t, syn_id, (*p)[i])));
  delete p;
}

void ConnectionManager::connect(Node& s, Node& r, thread tid, index syn)
{
  index gid = s.get_gid();
  validate_connector(tid, gid, syn);
  connections_[tid][gid][syn]->register_connection(s, r);
}

void ConnectionManager::connect(Node& s, Node& r, thread tid, double_t w, double_t d, index syn)
{
  index gid = s.get_gid();
  validate_connector(tid, gid, syn);
  connections_[tid][gid][syn]->register_connection(s, r, w, d);
}

void ConnectionManager::connect(Node& s, Node& r, thread tid, DictionaryDatum& p, index syn)
{
  index gid = s.get_gid();
  validate_connector(tid, gid, syn);
  connections_[tid][gid][syn]->register_connection(s, r, p);
}

void ConnectionManager::send(thread t, index sgid, Event& e)
{
  if (sgid < connections_[t].size())
    for (size_t i = 0; i < connections_[t][sgid].size(); ++i)
      if (connections_[t][sgid][i] != 0)
        connections_[t][sgid][i]->send(e);
}

size_t ConnectionManager::get_num_connections() const
{
  size_t num_connections = 0;
  std::vector<ConnectorModel*>::const_iterator iter;
  for (iter = prototypes_.begin(); iter != prototypes_.end(); ++iter)
    num_connections += (*iter)->get_num_connections();

  return num_connections;
} 

} // namespace
