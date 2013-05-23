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
#include <algorithm>
// OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif

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

  std::vector< google::sparsetable< std::vector< syn_id_connector > > > tmp(
  net_.get_num_threads(), google::sparsetable< std::vector< syn_id_connector > >());

  connections_.swap(tmp);

  num_connections_ = 0;
  num_conn_changed_since_counted_ = false;
}

void ConnectionManager::delete_connections_()
{
  for (tVVVConnector::iterator it = connections_.begin(); it != connections_.end(); ++it)
    for (tVVConnector::nonempty_iterator iit = it->nonempty_begin(); iit != it->nonempty_end(); ++iit)
      for ( tVConnector::iterator iiit = iit->begin(); iiit != iit->end(); ++iiit)
	delete (*iiit).connector;
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

bool ConnectionManager::get_user_set_delay_extrema() const
{
  bool user_set_delay_extrema = false;
  
  for (size_t syn_id = 0; syn_id < prototypes_.size(); ++syn_id)
    if (prototypes_[syn_id] != 0)
        user_set_delay_extrema |= prototypes_[syn_id]->get_user_set_delay_extrema();
  
  return user_set_delay_extrema;
}

index ConnectionManager::validate_connector(thread tid, index gid, index syn_id)
{
  assert_valid_syn_id(syn_id);

  if (connections_[tid].size() < net_.size())
    connections_[tid].resize(net_.size());

  int syn_vec_index = get_syn_vec_index(tid, gid, syn_id);
  if ( syn_vec_index == -1 )
  {
    struct syn_id_connector sc = {syn_id, prototypes_[syn_id]->get_connector()};

    connections_[tid].mutating_get(gid).push_back(sc);
    syn_vec_index = connections_[tid].get(gid).size() - 1;
  }
  return static_cast<index>(syn_vec_index);
}

index ConnectionManager::copy_synapse_prototype(index old_id, std::string new_name)
{
  // we can assert here, as nestmodule checks this for us
  assert (! synapsedict_->known(new_name));

  ConnectorModel* new_prototype = get_synapse_prototype(old_id).clone(new_name);
  int new_id = prototypes_.size();
  prototypes_.push_back(new_prototype);
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
  int syn_vec_index = get_syn_vec_index (tid,gid,syn_id);
  assert_valid_syn_id(syn_id);
  DictionaryDatum dict(new Dictionary);
  connections_[tid].get(gid)[syn_vec_index].connector->get_synapse_status(dict, p);
  (*dict)[names::source] = gid;
  (*dict)[names::synapse_model] = LiteralDatum(get_synapse_prototype(syn_id).get_name());

  return dict;
}

void ConnectionManager::set_synapse_status(index gid, index syn_id, port p, thread tid, const DictionaryDatum& dict)
{
  assert_valid_syn_id(syn_id);
  int syn_vec_index = get_syn_vec_index (tid,gid,syn_id);
  connections_[tid].get(gid)[syn_vec_index].connector->set_synapse_status(dict, p);
}


DictionaryDatum ConnectionManager::get_connector_status(const Node& node, index syn_id)
{
  assert_valid_syn_id(syn_id);

  DictionaryDatum dict(new Dictionary);
  index gid = node.get_gid();
  for (thread tid = 0; tid < net_.get_num_threads(); tid++)
  {
    index syn_vec_index = validate_connector(tid, gid, syn_id);
    connections_[tid].get(gid)[syn_vec_index].connector->get_status(dict);
  }
  return dict;
}

DictionaryDatum ConnectionManager::get_connector_status(index gid, index syn_id)
{
  assert_valid_syn_id(syn_id);

  DictionaryDatum dict(new Dictionary);
  for (thread tid = 0; tid < net_.get_num_threads(); tid++)
  {
    index syn_vec_index = validate_connector(tid, gid, syn_id);
    connections_[tid].get(gid)[syn_vec_index].connector->get_status(dict);
  }
  return dict;
}

void ConnectionManager::set_connector_status(Node& node, index syn_id, thread tid, const DictionaryDatum& dict)
{
  assert_valid_syn_id(syn_id);

  index gid = node.get_gid();
  index syn_vec_index = validate_connector(tid, gid, syn_id);
  connections_[tid].get(gid)[syn_vec_index].connector->set_status(dict);
}

ArrayDatum ConnectionManager::find_connections(DictionaryDatum params)
{
  ArrayDatum connectome;
  ulong_t source=0L;
  bool have_source = updateValue<long>(params, names::source, source);
  if (have_source)
    net_.get_node(source); // This throws if the node does not exist
  else
    throw UndefinedName(names::source.toString());
  
  ulong_t target=0L;
  bool have_target = updateValue<long>(params, names::target, target);
  if (have_target)
    net_.get_node(target); // This throws if the node does not exist

  size_t syn_id = 0;
  Name synmodel_name;
  bool have_synmodel = updateValue<std::string>(params, names::synapse_model, synmodel_name);

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
      int syn_vec_index = get_syn_vec_index(t, source, syn_id);
      if (source < connections_[t].size() && syn_vec_index != -1)
        find_connections(connectome, t, source, syn_vec_index, syn_id, params);
    }
    else
    {
      if (static_cast<size_t>(source) < connections_[t].size())
        for (syn_id = 0; syn_id < prototypes_.size(); ++syn_id)
        {
          int syn_vec_index = get_syn_vec_index(t, source, syn_id);
          if (syn_vec_index != -1)
            find_connections(connectome, t, source, syn_vec_index, syn_id, params);
        }
    }
  }
  
  return connectome;
}

void ConnectionManager::find_connections(ArrayDatum& connectome, thread t, index source, int syn_vec_index, index syn_id, DictionaryDatum params)
{
  std::vector<long>* p = connections_[t].get(source)[syn_vec_index].connector->find_connections(params);
  for (size_t i = 0; i < p->size(); ++i)
    connectome.push_back(ConnectionDatum(ConnectionID(source, 0, t, syn_id, (*p)[i])));
  delete p;
}

ArrayDatum ConnectionManager::get_connections(DictionaryDatum params) const
{
  ArrayDatum connectome;

  const Token source_t=   params->lookup(names::source);
  const Token target_t=   params->lookup(names::target);
  const Token syn_model_t=params->lookup(names::synapse_model);
  const TokenArray *source_a=0;
  const TokenArray *target_a=0;


  if (not source_t.empty())
      source_a=dynamic_cast<TokenArray const*>(source_t.datum());
  if (not target_t.empty())
      target_a=dynamic_cast<TokenArray const*>(target_t.datum());

  size_t syn_id = 0;

#ifdef _OPENMP
  std::string msg;
  msg = String::compose( "Setting OpenMP num_threads to %1.",net_.get_num_threads());
  net_.message(SLIInterpreter::M_DEBUG, "ConnectionManager::get_connections", msg);
  omp_set_num_threads(net_.get_num_threads());
#endif

  // First we check, whether a synapse model is given.
  // If not, we will iterate all.
  if (not syn_model_t.empty())
  {
      Name synmodel_name = getValue<Name>(syn_model_t);
      const Token synmodel = synapsedict_->lookup(synmodel_name);
      if (!synmodel.empty())
	  syn_id = static_cast<size_t>(synmodel);
      else
	  throw UnknownModelName(synmodel_name.toString());
      get_connections(connectome, source_a, target_a, syn_id);
  }
  else
  {  
      for (syn_id = 0; syn_id < prototypes_.size(); ++syn_id)
	{
	  ArrayDatum conn;
	  get_connections(conn, source_a, target_a, syn_id);
	  if (conn.size()>0)
	    connectome.push_back(new ArrayDatum(conn));
	}
  }

  return connectome;
}

void ConnectionManager::get_connections(ArrayDatum& connectome, TokenArray const *source, TokenArray const *target, size_t syn_id) const
{ 
    connectome.reserve(prototypes_[syn_id]->get_num_connections());

    if ( source==0 and target == 0)
    {
#ifdef _OPENMP
#pragma omp parallel
	{
	    thread t=omp_get_thread_num();
#else
        for (thread t = 0; t < net_.get_num_threads(); ++t)
	{
#endif
	    ArrayDatum conns_in_thread;
	    size_t num_connections_in_thread=0;
	    // Count how many connections we will have.
	    for(index source_id=1; source_id< connections_[t].size();++source_id)
	    {
              int syn_vec_index = get_syn_vec_index(t, source_id, syn_id);
              if(syn_vec_index > -1)
                num_connections_in_thread += connections_[t].get(source_id)[syn_vec_index].connector->get_num_connections();
	    }
		
#ifdef _OPENMP
#pragma omp critical
#endif
	    conns_in_thread.reserve(num_connections_in_thread);
	    for (index source_id=1; source_id< connections_[t].size(); ++source_id)
	    {
              int syn_vec_index = get_syn_vec_index(t, source_id, syn_id);
              if(syn_vec_index > -1)
                get_connections(conns_in_thread, source_id, t, syn_id);
	    }
	    if (conns_in_thread.size()>0)
	    {
#ifdef _OPENMP
#pragma omp critical
#endif
	       connectome.append_move(conns_in_thread);
	    }
	}
	return;
    }
    else if(source == 0 and target !=0)
    {
	connectome.reserve(prototypes_[syn_id]->get_num_connections());
#ifdef _OPENMP
#pragma omp parallel
	{
	    thread t=omp_get_thread_num();
#else
	for (thread t = 0; t < net_.get_num_threads(); ++t)
	{
#endif
	    ArrayDatum conns_in_thread;
	    size_t num_connections_in_thread=0;
	    // Count how many connections we will have maximally.
	    for(index source_id=1; source_id< connections_[t].size();++source_id)
	    {
              int syn_vec_index = get_syn_vec_index(t, source_id, syn_id);
              if(syn_vec_index > -1)
                num_connections_in_thread += connections_[t].get(source_id)[syn_vec_index].connector->get_num_connections();
	    }
		
#ifdef _OPENMP
#pragma omp critical
#endif
	    conns_in_thread.reserve(num_connections_in_thread);

	    for (index source_id=1; source_id< connections_[t].size(); ++source_id)
	    {
              int syn_vec_index = get_syn_vec_index(t, source_id, syn_id);
              if(syn_vec_index > -1)
              {
                for (index t_id=0; t_id< target->size(); ++t_id)
                {
                  size_t target_id = target->get(t_id);
                  connections_[t].get(source_id)[syn_vec_index].connector->get_connections(source_id, target_id, t, syn_id, conns_in_thread);
                }
              }
	    }
	    if (conns_in_thread.size()>0)
	    {
#ifdef _OPENMP
#pragma omp critical
#endif
	      connectome.append_move(conns_in_thread);
	    }
	}
	return;
      }
      else if(source !=0 )
      {
#ifdef _OPENMP
#pragma omp parallel
	  {
	      size_t t=omp_get_thread_num();
#else
	  for (size_t t = 0; t < net_.get_num_threads(); ++t)
	  {
#endif
	      ArrayDatum conns_in_thread;
	      size_t num_connections_in_thread=0;
	      // Count how many connections we will have.
	      for(index source_id=1; source_id< connections_[t].size();++source_id)
	      {
                int syn_vec_index = get_syn_vec_index(t, source_id, syn_id);
                if(syn_vec_index > -1)
                  num_connections_in_thread += connections_[t].get(source_id)[syn_vec_index].connector->get_num_connections();
	      }
		
#ifdef _OPENMP
#pragma omp critical
#endif
	      conns_in_thread.reserve(num_connections_in_thread);
	      for( index s=0; s< source->size(); ++s)
	      {
		  size_t source_id= source->get(s);
                  int syn_vec_index = get_syn_vec_index(t, source_id, syn_id);
                  if (source_id < connections_[t].size() && syn_vec_index > -1)
		  {
                    if (target == 0)
                    {
                      get_connections(conns_in_thread, source_id, t, syn_id);
                    }
                    else
                    {
                      for (index t_id=0; t_id< target->size(); ++t_id)
                      {
                        size_t target_id = target->get(t_id);
                        connections_[t].get(source_id)[syn_vec_index].connector->get_connections(source_id, target_id, t, syn_id, conns_in_thread );
                      }
                    }
		  }
	      }
	    
	      if (conns_in_thread.size()>0)
	      {
#ifdef _OPENMP
#pragma omp critical
#endif
                connectome.append_move(conns_in_thread);
	      }
	  }
	  return;
     } // else
}


// Return connections to all targets 
void ConnectionManager::get_connections(ArrayDatum& connectome, index source, thread t, index syn_id) const
{
  int syn_vec_index = get_syn_vec_index(t, source, syn_id);
  size_t n_ports=connections_[t].get(source)[syn_vec_index].connector->get_num_connections(); 
  connectome.reserve(n_ports);
  connections_[t].get(source)[syn_vec_index].connector->get_connections(source,t,syn_id,connectome);
}

void ConnectionManager::connect(Node& s, Node& r, index s_gid, thread tid, index syn)
{
  index syn_vec_index = validate_connector(tid, s_gid, syn);
  connections_[tid].get(s_gid)[syn_vec_index].connector->register_connection(s, r);
  num_conn_changed_since_counted_ = true;
}

void ConnectionManager::connect(Node& s, Node& r, index s_gid, thread tid, double_t w, double_t d, index syn)
{
  index syn_vec_index = validate_connector(tid, s_gid, syn);
  connections_[tid].get(s_gid)[syn_vec_index].connector->register_connection(s, r, w, d);
  num_conn_changed_since_counted_ = true;
}

void ConnectionManager::connect(Node& s, Node& r, index s_gid, thread tid, DictionaryDatum& p, index syn)
{
  index syn_vec_index = validate_connector(tid, s_gid, syn);
  connections_[tid].get(s_gid)[syn_vec_index].connector->register_connection(s, r, p);
  num_conn_changed_since_counted_ = true;
}


// connect with a list of connection status dicts
bool ConnectionManager::connect(ArrayDatum& conns)
{
    std::string msg;
// #ifdef _OPENMP
//     net_.message(SLIInterpreter::M_INFO, "ConnectionManager::Connect", msg);
// #endif

// #ifdef _OPENMP
// #pragma omp parallel shared

// #endif
    {
	for(Token *ct=conns.begin(); ct != conns.end(); ++ct)
	{
	    DictionaryDatum cd=getValue<DictionaryDatum>(*ct);
	    index target_gid=static_cast<size_t>((*cd)[names::target]);
	    Node *target_node = net_.get_node(target_gid);
	    size_t thr=target_node->get_thread();
	    
// #ifdef _OPENMP
// 	    size_t my_thr=omp_get_thread_num();
// 	    if(my_thr == thr)
// #endif
	    {		

 		size_t syn_id=0;
 		index source_gid=(*cd)[names::source];

 		Token synmodel = cd->lookup(names::synapse_model);
 		if(! synmodel.empty())
 		{
 		    std::string synmodel_name=getValue<std::string>(synmodel);
 		    synmodel = synapsedict_->lookup(synmodel_name);
 		    if (!synmodel.empty())
 			syn_id = static_cast<size_t>(synmodel);
 		    else
 			throw UnknownModelName(synmodel_name);
 		}
 		Node *source_node = net_.get_node(source_gid);
//#pragma omp critical
 		connect(*source_node, *target_node, source_gid, thr, cd, syn_id);
	    }
	}
    }
    return true;
}

void ConnectionManager::send(thread t, index sgid, Event& e)
{
  if (sgid < connections_[t].size())
    for (size_t i = 0; i < connections_[t].get(sgid).size(); ++i)
      connections_[t].get(sgid)[i].connector->send(e);
}

size_t ConnectionManager::get_num_connections() const
{
  if (num_conn_changed_since_counted_)
    count_connections();

  return num_connections_;
}

void ConnectionManager::count_connections() const
{
  // we need a local variable, as OpenMP reduction does not like
  // working with the global variable directly
  size_t num_connections = 0;

#ifdef _OPENMP
#pragma omp parallel reduction(+: num_connections)
  {
    size_t t = omp_get_thread_num();
#else
  for (index t = 0; t < net_.get_num_threads(); ++t)
  {
#endif
    std::vector<size_t> num_connections_per_syn_id(prototypes_.size(), 0);

    tVVConnector::const_nonempty_iterator iter;      
    for (iter = connections_[t].nonempty_begin(); iter != connections_[t].nonempty_end(); ++iter)
      for (size_t syn_id = 0; syn_id < (*iter).size(); ++syn_id)
        num_connections_per_syn_id[(*iter)[syn_id].syn_id] += (*iter)[syn_id].connector->get_num_connections();

    for (size_t syn_id = 0; syn_id < prototypes_.size(); ++syn_id)
    {
      prototypes_[syn_id]->set_num_connections(num_connections_per_syn_id[syn_id]);
      num_connections += num_connections_per_syn_id[syn_id];
    }
  }

  num_connections_ = num_connections;
  num_conn_changed_since_counted_ = false;
}

} // namespace
