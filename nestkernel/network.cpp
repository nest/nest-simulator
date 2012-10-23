/*
 *  network.cpp
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

#include "instance.h"
#include "network.h"
#include "genericmodel.h"
#include "scheduler.h"
#include "compound.h"
#include "interpret.h"
#include "dict.h"
#include "dictstack.h"
#include "integerdatum.h"
#include "booldatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "tokenutils.h"
#include "tokenarray.h"
#include "exceptions.h"
#include "sliexceptions.h"
#include "processes.h"
#include "nestmodule.h"

#include <cmath>
#include <set>

namespace nest {


Network::Network(SLIInterpreter &i)
    : scheduler_(*this),
      interpreter_(i),
      connection_manager_(*this),
      root_(0),
      current_(0),
      data_path_(),
      data_prefix_(),
      overwrite_files_(false),
      dict_miss_is_error_(true)
{
  Node::net_ = this;
  Communicator::net_ = this;

  modeldict_ = new Dictionary();
  interpreter_.def("modeldict", new DictionaryDatum(modeldict_));

  Model* model = new GenericModel<Compound>("subnet");
  register_model(*model);

  model = new GenericModel<proxynode>("proxynode");
  register_model(*model, true);

  synapsedict_ = new Dictionary();
  interpreter_.def("synapsedict", new DictionaryDatum(synapsedict_));
  connection_manager_.init(synapsedict_);

  init_();
}

Network::~Network()
{
  destruct_nodes_();
  clear_models_();

  // Now we can delete the clean model prototypes
  vector< std::pair<Model *, bool> >::iterator i;
  for(i = pristine_models_.begin(); i != pristine_models_.end(); ++i)
    if ((*i).first != 0)
      delete (*i).first;
}
  
void Network::init_()
{
  /*
   * We initialise the network with one compound that is the root of the tree.
   * Note that we MUST NOT call add_node(), since it expects a properly
   * initialized network.
   */
  assert(pristine_models_.size() > 0);
  Model* rootmodel= pristine_models_[0].first;
  assert(rootmodel != 0);
  
  nodes_.reserve(1000);
  Compound *root_container = static_cast<Compound *>(rootmodel->allocate(0));
  nodes_.push_back(root_container);
  root_container->reserve(get_num_threads());

  for(thread t = 0; t < get_num_threads(); ++t)
  {
    Node* newnode = rootmodel->allocate(t);
    newnode->set_gid_(0);
    newnode->set_model_id(0);
    newnode->set_thread(t);
    newnode->set_vp(thread_to_vp(t));
    root_container->push_back(newnode);
  }

  current_ = root_ = static_cast<Compound *>((*root_container)[0]);

  /**
    Build modeldict and list of models from clean prototypes.

    We also clear the models_ list here. This is necessary, since the
    Network::Network(SLIIntepreter&) constructor places the subnet
    and proxynode models in models_ before calling init_(), while
    reset() calls init_() with an empty models_ list. This leads to
    inconsistent modeldicts before and after ResetKernel, see #333.

    @todo THIS IS A WORK-AROUND. The model name/model id setup, and the 
    initialization of models must be revised in connection with with the
    Network/Scheduler refactoring (#150).
  */
  models_.clear();
  modeldict_->clear();

  // Re-create the model list from the clean prototypes
  for(index i = 0; i < pristine_models_.size(); ++i)
    if (pristine_models_[i].first != 0)
    {
      std::string name = pristine_models_[i].first->get_name();
      models_.push_back(pristine_models_[i].first->clone(name));
      if (!pristine_models_[i].second)
        modeldict_->insert(name, models_.size() - 1);
    }

  // data_path and data_prefix can be set via environment variables
  DictionaryDatum dict(new Dictionary);
  char *data_path = std::getenv("NEST_DATA_PATH");
  if (data_path)
    (*dict)["data_path"] = std::string(data_path);
  char *data_prefix = std::getenv("NEST_DATA_PREFIX");
  if (data_prefix)
    (*dict)["data_prefix"] = std::string(data_prefix);
  if (!dict->empty())
    set_data_path_prefix_(dict);

#ifdef HAVE_MUSIC
  music_in_portlist_.clear();
#endif
}

void Network::destruct_nodes_()
{
  // We call the destructor for each node excplicitly.
  // This destroys the objects without releasing their memory.
  // since the Memory is owned by the Model objects, we must
  // not call delete on the Node objects!
  for(size_t n = 0; n < nodes_.size(); ++n)
  {
    assert(nodes_[n] != 0);
    for(size_t t = 0; t < nodes_[n]->size(); ++t)
      (*nodes_[n])[t]->~Node();
    nodes_[n]->~Node();
  }

  std::vector<Node * > nodes;
  nodes_.swap(nodes);
}

void Network::clear_models_()
{
  // We delete all models, which will also delete all nodes. The built-in models
  // will be recovered from the pristine_models_ list in init_()
  for (vector<Model*>::iterator m = models_.begin(); m != models_.end(); ++m)
    if (*m != 0)
      delete *m;

  models_.clear();
}

void Network::reset()
{
  destruct_nodes_();
  clear_models_();

  // We free all Node memory and set the number of threads.
  vector< std::pair<Model *, bool> >::iterator m;
  for (m = pristine_models_.begin(); m != pristine_models_.end(); ++m)
  {
    (*m).first->clear(); // delete all nodes, because cloning the model may have created instances.
    (*m).first->set_threads();
  }

  scheduler_.reset();
  connection_manager_.reset();

  init_();
}

void Network::reset_kernel()
{
  scheduler_.set_threads(1);
  data_path_ = "";
  data_prefix_ = "";
  overwrite_files_ = false;
  dict_miss_is_error_ = true;

  reset();
}

void Network::reset_network()
{
  if ( !scheduler_.get_simulated() )
    return;  // nothing to do
  
  /* Reinitialize state on all nodes, force init_buffers() on next Simulate
     Finding all nodes is non-trivial:
     - Nodes with proxies are found in nodes_. This is also true for any nodes
       that are part of Compounds.
     - Nodes without proxies are not registered in nodes_. Instead, a Compound
       is created as container, and this container is stored in nodes_. The
       container then contains the actual nodes, which need to be reset. Such
       compounds have model_id==-1.
     Thus, we iterate nodes_; additionally, we iterate the content of a Compound
     if the Compound's replica_container flag is set. Other compounds are not
     iterated, since their nodes are registered in nodes_ directly.
   */
  for ( vector<Node*>::iterator it = nodes_.begin();
        it != nodes_.end() ; ++it )
  {
    Node& n = **it;
    if ( n.size() == 0 )
    {                     // not a compound
      n.init_state();
      n.unset(Node::buffers_initialized);
    }
    else if ( n.get_model_id() == -1 )
    {
      Compound* const c = dynamic_cast<Compound*>(&n);
      assert(c);
      for ( vector<Node*>::iterator cit = c->begin() ; cit != c->end() ; ++cit )
      {
        (*cit)->init_state();
        (*cit)->unset(Node::buffers_initialized);
      }
    }
  }
  
  // clear global spike buffers
  scheduler_.clear_pending_spikes();
  
  // ConnectionManager doesn't support resetting dynamic synapses yet
  message(SLIInterpreter::M_WARNING, "ResetNetwork", 
          "Synapses with internal dynamics (facilitation, STDP) are not reset.\n"
          "This will be implemented in a future version of NEST.");
}

int Network::get_model_id(const char name[]) const
{
  const std::string model_name(name);
  for(int i=0; i< (int)models_.size(); ++i)
  {
    assert(models_[i] != NULL);
    if(model_name == models_[i]->get_name())
      return i;
  }
  return -1;
}


index Network::add_node(long_t mod, long_t n)
{
  assert(current_ != 0);
  assert(root_ != 0);

  if(mod >= (long_t)models_.size() || mod < 0)
    throw UnknownModelID(mod);

  if (n < 1)
    throw BadProperty();

  const thread n_threads = get_num_threads();
  assert(n_threads > 0);

  const index min_gid = nodes_.size();
  const index max_gid = min_gid + n;

  Model* model = models_[mod];
  Model* container_model = models_[0];
  assert(model != 0);
  assert(container_model != 0);
  
  /* current_ points to the instance of the current subnet on thread 0.
     The following code makes subnet a pointer to the wrapper Compound
     containing the instances of the current subnet on all threads.
   */
  index subnet_gid = current_->get_gid();
  Compound* subnet = dynamic_cast<Compound *>(nodes_[subnet_gid]);
  assert(subnet != 0);
  assert(subnet->size() == static_cast<size_t>(n_threads));
  assert((*subnet)[0] == current_);
  
  if((max_gid > nodes_.max_size()) || (max_gid<min_gid))
    {
      message(SLIInterpreter::M_ERROR, " Network::add:node", "Requestend number of nodes will overflow the memory.");
      message(SLIInterpreter::M_ERROR, " Network::add:node", "No nodes were created.");
      throw KernelException("OutOfMemory");
    } 

  nodes_.reserve(max_gid);

  if (model->has_proxies())
  {
    // In this branch we create nodes for all GIDs which are on a local thread
    // and proxies for all GIDs which are on remote processes.
    int proxy_model_id = get_model_id("proxynode");
    assert(proxy_model_id > 0);
    Model *proxy_model = models_[proxy_model_id];
    assert(proxy_model != 0);
    
    const int n_per_process = n / scheduler_.get_num_processes();
    const int n_per_thread = n_per_process / n_threads + 1;
    const int proxies_per_thread = (n - n_per_process) / n_threads + 1;
    for(thread t = 0; t < n_threads; ++t)
    {
      // Model::reserve() reserves memory for n ADDITIONAL nodes
      model->reserve(t, n_per_thread);
      proxy_model->reserve(t, proxies_per_thread);
    }

    for(size_t gid = min_gid; gid < max_gid; ++gid)
    {
      thread vp = (current_->get_children_on_same_vp()) ? current_->get_children_vp() 
                                                        : suggest_vp(gid);
      thread t = vp_to_thread(vp);

      Node *newnode = 0;
      if(is_local_vp(vp))
	newnode = model->allocate(t);
      else
	newnode = proxy_model->allocate(t);
      
      newnode->set_gid_(gid);
      newnode->set_model_id(mod);
      newnode->set_thread(t);
      newnode->set_vp(vp);
      nodes_.push_back(newnode);   // put into global list
      current_->add_node(newnode); // and into current subnet, thread 0.
    }
  } 
  else if (!model->one_node_per_process())
  {
    // We allocate space for n containers which will hold the threads sorted.
    // We use Compounds to store the instances for each thread to exploit the
    // very efficient memory allocation for nodes. 
    //
    // These wrapper compounds are registered in the global nodes_ array to 
    // provide access to the instances both for manipulation by SLI functions 
    // and so that Scheduler::calibrate() can discover the instances and 
    // register them for updating.
    //
    // The instances are also registered with the instance of the current subnet
    // for the thread to which the created instance belongs. This is mainly
    // important so that the compound structure is preserved on all VPs.
    // Node enumeration is done on by the registration with the per-thread 
    // instances.
    //
    // The wrapper can be addressed under the GID assigned to no-proxy node 
    // created. If this no-proxy node is NOT a compound (e.g. a device), then
    // each instance can be retrieved by giving the respective thread-id to
    // get_node(). Instances of compounds cannot be addressed individually.
    //
    // The allocation of the wrapper compounds is spread over threads to 
    // balance memory load.
    size_t container_per_thread = n / n_threads + 1;
    
    // since we create the n nodes on each thread, we reserve the full load.
    for(thread t = 0; t < n_threads; ++t)
    {
      // Model::reserve() reserves memory for n ADDITIONAL nodes
      model->reserve(t,n);
      container_model->reserve(t, container_per_thread);
      static_cast<Compound *>((*subnet)[t])->reserve(n);
    }
    
    // The following loop creates n nodes. For each node, a wrapper is created
    // and filled with one instance per thread, in total n * n_thread nodes in
    // n wrappers. 
    for(index gid = min_gid; gid < max_gid; ++gid)
    {
      thread thread_id = vp_to_thread(suggest_vp(gid));

      // Create wrapper and register with global nodes_ array.
      Compound *container= static_cast<Compound *>(container_model->allocate(thread_id));
      container->set_model_id(-1); // mark as pseudo-container wrapping replicas, see reset_network()
      container->reserve(n_threads); // space for one instance per thread
      nodes_.push_back(container); 

      // Generate one instance of desired model per thread
      for(thread t = 0; t < n_threads; ++t)
      { 
        Node *newnode = model->allocate(t);
        newnode->set_gid_(gid);        // all instances get the same global id.
        newnode->set_model_id(mod);
        newnode->set_thread(t);
        newnode->set_vp(thread_to_vp(t));
        
        // If the instance is a Compound, set Child-VP-Assignment policies.
        Compound* newcomp = 0;
        if ((newcomp = dynamic_cast<Compound*>(newnode)) != 0)
        {
          if (current_->get_children_on_same_vp())
          {
            newcomp->set_children_on_same_vp(true);
            newcomp->set_children_vp(current_->get_children_vp());
          }
          else
            newcomp->set_children_vp(suggest_vp(gid));
        }
        
        // Register instance with wrapper
        container->push_back(newnode); 
        
        // Register instance with per-thread instance of enclosing subnet.
        static_cast<Compound *>((*subnet)[t])->add_node(newnode); 
      }
    }
  }
  else
  {
    for(index gid = min_gid; gid < max_gid; ++gid)
    {
      Node *newnode = model->allocate(0);
      newnode->set_gid_(gid);
      newnode->set_model_id(mod);
      newnode->set_thread(0);
      newnode->set_vp(thread_to_vp(0));
        
      // Register instance with wrapper
      nodes_.push_back(newnode);
      
      // and into current subnet, thread 0.
      current_->add_node(newnode);
    }
  }
  
  //set off-grid spike communication if necessary
  if (model->is_off_grid())
  {
    scheduler_.set_off_grid_communication(true);
  
    message(SLIInterpreter::M_INFO, "network::add_node",
            "Precise neuron models exist: the kernel property off_grid_spiking "
            "has been set to true.");
  }

  return max_gid - 1;
}

void Network::init_state(index GID)
{
  Node *n= get_node(GID);
  if(n == 0 )
    throw UnknownNode(GID);

  n->init_state();
}

void Network::go_to(index n)
{
  if(Compound *target=dynamic_cast<Compound*>(get_node(n)))
    current_ = target;
  else
    throw SubnetExpected();
}


void Network::go_to(TokenArray p)
{
  std::vector<size_t> adr;
  p.toVector(adr);
  go_to(adr);
}


void Network::go_to(vector<size_t> const &p)
{
  if(Compound *target=dynamic_cast<Compound*>(get_node(p)))
    current_ = target;
  else
    throw SubnetExpected();
}


Node* Network::get_node(TokenArray p, thread thr) const
{
  std::vector<size_t> adr;
  p.toVector(adr);
  return get_node(adr, thr);
}


Node* Network::get_node(vector<size_t> const &p, thread thr) const
{
  assert(root_ != NULL);
  Compound *position = current_;

  Node *new_position = position;
  
  // First, we determine the global ID of the node. Then, we use the
  // thread to determine the correct pointer.

  for(size_t i=0; i< p.size(); ++i)
  {
    // Any entry in an address will jump to the root node.
    if(p[i]==0)
    {
      new_position = root_;
      position = dynamic_cast<Compound*>(new_position);
      // move directly to next vector element 
      continue;
    }

    /* Range checking:
       Since indices in address arrays start with one, 
       we have to use an offset of 1 with respect to
       the standard c/c++ indices.
       Accordingly, the correct range for SLI indices is
       1...size().
       If the range is violated, we throw an UnknownNode 
       exception.
    */
   
    if(p[i]<1 || p[i]>(*position).size())
      throw UnknownNode();
    
    // The following access is safe, since the range has been
    // checked above.
    new_position=(*position)[p[i] -1];

    // If the following happens, there is a "hole" in the Network tree, 
    // however, this case can occur if a Node has been deleted.
    // Thus, we must handle this case gracefully and just throw an 
    // UnknownNode exception.
    if(new_position == 0)
      throw UnknownNode();
      
    if(Compound *pos=dynamic_cast<Compound*>(new_position))
      position=pos;
    else if(i < p.size() - 1)
      throw UnknownNode();
  }

  assert(new_position != 0);
  index gid = new_position->get_gid();
  if(thr > 0 && nodes_[gid]->size() > 0)
  {
    assert(thr < (thread)nodes_[gid]->size());
    return (*nodes_[gid])[thr];
  }

  return new_position;
}


Node* Network::get_node(index n, thread thr) const
{
  if(n >= nodes_.size() || nodes_[n] == 0)
    throw UnknownNode(n);

  // non-compound node
  if(nodes_[n]->size() == 0)
    return nodes_[n];

  if (thr < 0 || thr >= (thread)nodes_[n]->size())
    throw UnknownNode();

  // compound node
  return (*nodes_[n])[thr];
}

const Compound* Network::get_thread_siblings(index n) const
{
  if(nodes_[n]->size() == 0)
    throw NoThreadSiblingsAvailable(n);
  
  return dynamic_cast<Compound*>(nodes_[n]);
}

vector<size_t> Network::get_adr(Node const* node) const
{
  std::vector<size_t> adr;
  while(node != 0)
  {
    if(node->get_parent())
      adr.push_back(node->get_lid()+1);
    else
      adr.push_back(0); // root node has index 0 by definition
    node = node->get_parent();
  }
  std::reverse(adr.begin(),adr.end());
  return adr;
}

bool Network::model_in_use(index i) const
{
  bool found = false;

  for (vector<Node *>::const_iterator it = nodes_.begin();
       it != nodes_.end() && !found;
       it++)
    if ( (*it)->get_model_id() == static_cast<int>(i) )
      found = true;

  return found;
}

void Network::simulate(Time const& t)
{
  scheduler_.simulate(t);
}

void Network::resume()
{
  scheduler_.resume();
}

void Network::memory_info()
{
  std::cout.setf(std::ios::left);
  std::vector<index> idx(models_.size());

  for (index i = 0; i < models_.size(); ++i)
    idx[i] = i;

  std::sort(idx.begin(), idx.end(), ModelComp(models_));
  
  std::string sep("--------------------------------------------------");
  
  std::cout << sep << std::endl;
  std::cout << std::setw(25) << "Name"
            << std::setw(13) << "Capacity"
            << std::setw(13) << "Available"
            << std::endl;
  std::cout << sep << std::endl;

  for (index i = 0; i < models_.size(); ++i)
  {
    Model * mod = models_[idx[i]];
    if (mod->mem_capacity() != 0)
      std::cout << std::setw(25) << mod->get_name()
                << std::setw(13) << mod->mem_capacity() * mod->get_element_size()
                << std::setw(13) << mod->mem_available() * mod->get_element_size()
                << std::endl;
  }

  std::cout << sep << std::endl;
  std::cout.unsetf(std::ios::left);
}

void Network::print(TokenArray p, int depth)
{
  Compound *target = dynamic_cast<Compound*>(get_node(p));
  if(target != NULL)
    std::cout << target->print_network(depth + 1, 0);
  else
    throw SubnetExpected();
}

void Network::set_status(index gid, const DictionaryDatum& d)
{
  if (gid >= size())
    throw UnknownNode(gid);

  // we first handle normal nodes, except the root (GID 0)
  if ( gid > 0 )
  {
    Node &target= *(nodes_[gid]);

    if ( target.size() == 0 )
      set_status_single_node_(target, d);
    else
      for(size_t t=0; t < target.size(); ++t)
      {
        // non-root container for devices without proxies and compounds
        // we iterate over all threads
        assert(target[t] != 0);
        set_status_single_node_(*target[t], d);
      }
    return;
  }
  
  /* Code below is executed only for the root node, gid == 0

     In this case, we must
     - set scheduler properties
     - set properties for the compound representing each thread
     - set the data_path, data_prefix and overwrite_files properties
     
     The main difficulty here is to handle the access control for
     dictionary items, since the dictionary is read in several places.

     We proceed as follows:
     - clear access flags
     - set scheduler properties; this must be first, anyways
     - set data_path, data_prefix, overwrite_files
     - at this point, all non-compound property flags are marked accessed
     - loop over all per-thread compounds
     - the first per-thread compound will flag all compound properties as read
     - now, all dictionary entries must be flagged as accessed, otherwise the
       dictionary contains unknown entries. Thus, set_status_single_node_
       will not throw an exception
     - since all items in the root node are of type Compound, all read the same
       properties and we can leave the access flags set 
   */   
  d->clear_access_flags();
  scheduler_.set_status(d); // careful, this may invalidate all node pointers!
  set_data_path_prefix_(d);
  updateValue<bool>(d, "overwrite_files", overwrite_files_);
  updateValue<bool>(d, "dict_miss_is_error", dict_miss_is_error_);
  
  std::string tmp;
  if ( !d->all_accessed(tmp) )  // proceed only if there are unaccessed items left
  {
    // Fetch the target pointer here. We cannot do it above, since 
    // Scheduler::set_status() may modify the root compound if the number
    // of threads changes. HEP, 2008-10-20
    Node &target= *(nodes_[gid]);

    for(size_t t=0; t < target.size(); ++t)
    {
      // Root container for per-thread subnets. We must prevent clearing of access
      // flags before each compound's properties are set by passing false as last arg
      // we iterate over all threads
      assert(target[t] != 0);
      set_status_single_node_(*target[t], d, false);
    }
  }  
}

void Network::set_status_single_node_(Node& target, const DictionaryDatum& d, bool clear_flags)
{
  // proxies have no properties
  if ( !target.is_proxy() )
  {
    if ( clear_flags )
      d->clear_access_flags();
    target.set_status_base(d);
    std::string missed;
    if ( !d->all_accessed(missed) )
    {
      if ( dict_miss_is_error() )
        throw UnaccessedDictionaryEntry(missed);
      else
        message(SLIInterpreter::M_WARNING, "Network::set_status", 
                ("Unread dictionary entries: " + missed).c_str());
    }
  }
}

void Network::set_data_path_prefix_(const DictionaryDatum& d)
{
  std::string tmp;
  if ( updateValue<std::string>(d, "data_path", tmp) )
  {
    DIR* testdir = opendir(tmp.c_str());
    if ( testdir != NULL )
    {
      data_path_ = tmp;  // absolute path & directory exists
      closedir(testdir); // we only opened it to check it exists
    }
    else
    {
      std::string msg;

      switch(errno)
      {
	case ENOTDIR:
	  msg = String::compose("'%1' is not a directory.", tmp);
	  break;
	case ENOENT:
	  msg = String::compose("Directory '%1' does not exist.", tmp);
	  break;
	default:
	  msg = String::compose("Errno %1 received when trying to open '%2'", errno, tmp);
	  break;
      }

      message(SLIInterpreter::M_ERROR, "SetStatus", "Variable data_path not set: " + msg);
    }
  }

  if ( updateValue<std::string>(d, "data_prefix", tmp) )
  {
    if ( tmp.find('/') == std::string::npos )
      data_prefix_ = tmp;
    else
      message(SLIInterpreter::M_ERROR, "SetStatus", "Data prefix must not contain path elements.");
  }
}

DictionaryDatum Network::get_status(index idx) const
{
  Node *target = get_node(idx);
  assert(target != 0);

  DictionaryDatum d;
  assert(target !=0);

  d = target->get_status_base();

  if(target == root_)
  {
    scheduler_.get_status(d);
    connection_manager_.get_status(d);
    (*d)["network_size"] = size();
    (*d)["data_path"] = data_path_;
    (*d)["data_prefix"] = data_prefix_;
    (*d)["overwrite_files"] = overwrite_files_;
    (*d)["dict_miss_is_error"] = dict_miss_is_error_;
  }

  return d;
}

// gid gid
void Network::connect(index source_id, index target_id, index syn)
{
  Node* target_ptr = 0;
  target_ptr = get_node(target_id);
  
  if (target_ptr->is_proxy())
    return;
  
  Node* source_ptr = 0;
  //target_thread defaults to 0 for devices
  thread target_thread = target_ptr->get_thread();
  source_ptr = get_node(source_id, target_thread); 
 
  //normal nodes and devices with proxies
  if (target_ptr->has_proxies())
  {
    connect(*source_ptr, *target_ptr, target_thread, syn);
  }
  else if (target_ptr->local_receiver()) //normal devices
  {
    if(source_ptr->is_proxy())
      return;

    if ((source_ptr->get_thread() != target_thread) && (source_ptr->has_proxies()))
    {
      target_thread = source_ptr->get_thread();
      target_ptr = get_node(target_id, target_thread);
    }

    connect(*source_ptr, *target_ptr, target_thread, syn);
  }
  else //globally receiving devices iterate over all target threads
  {
    if (!source_ptr->has_proxies()) //we do not allow to connect a device to a global receiver at the moment
      throw IllegalConnection();

    const thread n_threads = get_num_threads();
    for (thread t = 0; t < n_threads; t++)
    {
      target_ptr = get_node(target_id, t);
      connect(*source_ptr, *target_ptr, t, syn);
    } 
  }
}

// gid gid weight delay
void Network::connect(index source_id, index target_id, double_t w, double_t d, index syn)
{
  Node* target_ptr = 0;
  target_ptr = get_node(target_id);
  
  if (target_ptr->is_proxy())
    return;
  
  Node* source_ptr = 0;
  //target_thread defaults to 0 for devices
  thread target_thread = target_ptr->get_thread();
  source_ptr = get_node(source_id, target_thread); 
 
  //normal nodes and devices with proxies
  if (target_ptr->has_proxies())
  {
    connect(*source_ptr, *target_ptr, target_thread, w, d, syn);
  }
  else if (target_ptr->local_receiver()) //normal devices
  {
    if(source_ptr->is_proxy())
      return;

    if ((source_ptr->get_thread() != target_thread) && (source_ptr->has_proxies()))
    {
      target_thread = source_ptr->get_thread();
      target_ptr = get_node(target_id, target_thread);
    }

    connect(*source_ptr, *target_ptr, target_thread, w, d, syn);
  }
  else //globally receiving devices iterate over all target threads
  {
    if (!source_ptr->has_proxies()) //we do not allow to connect a device to a global receiver at the moment
      return;
    const thread n_threads = get_num_threads();
    for (thread t = 0; t < n_threads; t++)
    {
      target_ptr = get_node(target_id, t);
      connect(*source_ptr, *target_ptr, t, w, d, syn);
    } 
  }
}

// gid gid dict
bool Network::connect(index source_id, index target_id, DictionaryDatum& params, index syn)
{
  Node* target_ptr = 0;
  target_ptr = get_node(target_id);
  
  if (target_ptr->is_proxy())
    return false;
  
  Node* source_ptr = 0;
  //target_thread defaults to 0 for devices
  thread target_thread = target_ptr->get_thread();
  source_ptr = get_node(source_id, target_thread);
 
  //normal nodes and devices with proxies
  if (target_ptr->has_proxies())
  {
    connect(*source_ptr, *target_ptr, target_thread, params, syn);
  }
  else if (target_ptr->local_receiver()) //normal devices
  {
    if(source_ptr->is_proxy())
      return false;

    if ((source_ptr->get_thread() != target_thread) && (source_ptr->has_proxies()))
    {
      target_thread = source_ptr->get_thread();
      target_ptr = get_node(target_id, target_thread);
    }

    connect(*source_ptr, *target_ptr, target_thread, params, syn);
  }
  else //globally receiving devices iterate over all target threads
  {
    if (!source_ptr->has_proxies()) //we do not allow to connect a device to a global receiver at the moment
      return false;
    const thread n_threads = get_num_threads();
    for (thread t = 0; t < n_threads; t++)
    {
      target_ptr = get_node(target_id, t);
      connect(*source_ptr, *target_ptr, t, params, syn);
    } 
  }
  
  // We did not exit prematurely due to proxies, so we have connected.
  return true;
}

// -----------------------------------------------------------------------------

void Network::divergent_connect(index source_id, TokenArray target_ids, TokenArray weights, TokenArray delays, index syn)
{
  bool complete_wd_lists = (target_ids.size() == weights.size() && weights.size() != 0 && weights.size() == delays.size());
  bool short_wd_lists = (target_ids.size() != weights.size() && weights.size() == 1 && delays.size() == 1);
  bool no_wd_lists = (weights.size() == 0 && delays.size() == 0);

  // check if we have consistent lists for weights and delays
  if (! (complete_wd_lists || short_wd_lists || no_wd_lists))
  {
    message(SLIInterpreter::M_ERROR, "DivergentConnect", "weights and delays must be either doubles or lists of equal size. "
            "If given as lists, their size must be 1 or the same size as targets.");
    throw DimensionMismatch();
  }
  
  Node* source = get_node(source_id);
    
  Compound *source_comp=dynamic_cast<Compound *>(source);
  if(source_comp !=0)
  {
    message(SLIInterpreter::M_INFO, "DivergentConnect", "Source ID is a subnet; I will iterate it.");
    
    NodeList source_nodes(*source_comp);
    for(NodeList::iterator src=source_nodes.begin(); src!= source_nodes.end(); ++src)
      divergent_connect((*src)->get_gid(), target_ids, weights, delays, syn);

    return;
  }

  // We retrieve pointers for all targets, this implicitly checks if they
  // exist and throws UnknownNode if not.
  std::vector<Node*> targets(target_ids.size());
  for (index i = 0; i < target_ids.size(); ++i)
    targets[i] = get_node(getValue<long>(target_ids[i]));

  for(index i = 0; i < target_ids.size(); ++i)
  {
    if (targets[i]->is_proxy())
      continue;

    thread target_thread = targets[i]->get_thread();
 
    if (source->get_thread() != target_thread)
      source = get_node(source_id, target_thread);

    if (!targets[i]->has_proxies() && source->is_proxy())
      continue;

    try
    {
      if (complete_wd_lists)
	connect(*source, *targets[i], target_thread, weights.get(i), delays.get(i), syn);
      else if (short_wd_lists)
	connect(*source, *targets[i], target_thread, weights.get(0), delays.get(0), syn);
      else 
	connect(*source, *targets[i], target_thread, syn);
    }
    catch (IllegalConnection& e)
    {
      std::string msg = String::compose("Global target ID %1: Target does not support event.", target_ids[i]);
      message(SLIInterpreter::M_WARNING, "DivergentConnect", msg.c_str());
      message(SLIInterpreter::M_WARNING, "DivergentConnect", "Connection will be ignored.");
      continue;
    }
    catch (UnknownReceptorType& e)
    {
      std::string msg = String::compose("In Connection from global source ID %1 to target ID %2:", source_id, target_ids[i]);
      message(SLIInterpreter::M_WARNING, "Connect", msg.c_str());
      message(SLIInterpreter::M_WARNING, "Connect", "Target does not support requested receptor type.");
      message(SLIInterpreter::M_WARNING, "Connect", "Connection will be ignored.");
      interpreter_.raiseerror(e.what());
      continue;
    }
  }
}

void Network::random_divergent_connect(index source_id, TokenArray target_ids, index n, TokenArray weights, TokenArray delays, bool allow_multapses, bool allow_autapses, index syn)
{
  Node *source = get_node(source_id);

  // check if we have consistent lists for weights and delays
  if (! (weights.size() == n || weights.size() == 0) && (weights.size() == delays.size()))
  {
    message(SLIInterpreter::M_ERROR, "ConvergentConnect", "weights and delays must be lists of size n.");
    throw DimensionMismatch();
  }
    
  Compound *source_comp=dynamic_cast<Compound *>(source);
  if(source_comp !=0)
  {
    message(SLIInterpreter::M_INFO, "RandomDivergentConnect", "Source ID is a subnet; I will iterate it.");
    
    NodeList source_nodes(*source_comp);
    for(NodeList::iterator src=source_nodes.begin(); src!= source_nodes.end(); ++src)
      random_divergent_connect((*src)->get_gid(),target_ids, n, weights, delays, allow_multapses, allow_autapses, syn);

    return;
  }

  librandom::RngPtr rng = get_grng();

  TokenArray chosen_targets;

  std::set<long> ch_ids; //ch_ids used for multapses identification

  long n_rnd = target_ids.size();

  for (size_t j = 0; j < n; ++j)
  {
    long t_id;
      
    do 
    {
      t_id  = rng->ulrand(n_rnd);
    }
    while ( ( !allow_autapses && ((index)target_ids[t_id]) == source_id )
            || ( !allow_multapses && ch_ids.find( t_id ) != ch_ids.end() ) );
      
    if (!allow_multapses)
      ch_ids.insert(t_id);
      
    chosen_targets.push_back(target_ids[t_id]);
  }
  
  divergent_connect(source_id, chosen_targets, weights, delays, syn);
}

// -----------------------------------------------------------------------------

void Network::convergent_connect(TokenArray source_ids, index target_id, TokenArray weights, TokenArray delays, index syn)
{
  bool complete_wd_lists = (source_ids.size() == weights.size() && weights.size() != 0 && weights.size() == delays.size());
  bool short_wd_lists = (source_ids.size() != weights.size() && weights.size() == 1 && delays.size() == 1);
  bool no_wd_lists = (weights.size() == 0 && delays.size() == 0);

  // check if we have consistent lists for weights and delays
  if (! (complete_wd_lists || short_wd_lists || no_wd_lists))
  {
    message(SLIInterpreter::M_ERROR, "ConvergentConnect", "weights and delays must be either doubles or lists of equal size. "
            "If given as lists, their size must be 1 or the same size as sources.");
    throw DimensionMismatch();
  }

  Node* target = get_node(target_id);
  if (target->is_proxy())
    return;

  Compound *target_comp = dynamic_cast<Compound *>(target);
  if(target_comp != 0)
  {
    message(SLIInterpreter::M_INFO, "ConvergentConnect", "Target node is a subnet; I will iterate it.");
    
    NodeList target_nodes(*target_comp);
    for(NodeList::iterator tgt = target_nodes.begin(); tgt != target_nodes.end(); ++tgt)
      convergent_connect(source_ids, (*tgt)->get_gid(), weights, delays, syn);

    return;
  }

  // We retrieve pointers for all sources, this implicitly checks if they
  // exist and throws UnknownNode if not.
  std::vector<Node*> sources(source_ids.size());
  for (index i = 0; i < source_ids.size(); ++i)
    sources[i] = get_node(getValue<long>(source_ids[i]));

  for(index i = 0; i < sources.size(); ++i)
  {
    thread target_thread = target->get_thread();

    if (!target->has_proxies())
    {
      target_thread = sources[i]->get_thread();
      
      // If target is on the wrong thread, we need to get the right one now.
      if (target->get_thread() != target_thread)
        target = get_node(target_id, target_thread);

      if (sources[i]->is_proxy())
        continue;
    }
    
    // The source node may still be on a wrong thread, so we need to get the right
    // one now. As get_node() is quite expensive, so we only call it if we need to
    if (sources[i]->get_thread() != target_thread)
      sources[i] = get_node(source_ids[i], target_thread);

    try
    {
      if (complete_wd_lists)
        connect(*sources[i], *target, target_thread, weights.get(i), delays.get(i), syn);
      else if (short_wd_lists)
	connect(*sources[i], *target, target_thread, weights.get(0), delays.get(0), syn);
      else 
        connect(*sources[i], *target, target_thread, syn);
    }
    catch (IllegalConnection& e)
    {
      std::string msg = String::compose("Global target ID %1: Target does not support event.", target_id);
      message(SLIInterpreter::M_WARNING, "ConvergentConnect", msg.c_str());
      message(SLIInterpreter::M_WARNING, "ConvergentConnect", "Connection will be ignored.");
      continue;
    }
    catch (UnknownReceptorType& e)
    {
      std::string msg = String::compose("In Connection from global source ID %1 to target ID %2:", source_ids[i], target_id);
      message(SLIInterpreter::M_WARNING, "ConvergentConnect", msg.c_str());
      message(SLIInterpreter::M_WARNING, "ConvergentConnect", "Target does not support requested receptor type.");
      message(SLIInterpreter::M_WARNING, "ConvergentConnect", "Connection will be ignored.");
      interpreter_.raiseerror(e.what());
      continue;
    }
  }
}


void Network::random_convergent_connect(TokenArray source_ids, index target_id, index n, TokenArray weights, TokenArray delays, bool allow_multapses, bool allow_autapses, index syn)
{
  Node* target = get_node(target_id);

  if (target->is_proxy())
    return;

  // check if we have consistent lists for weights and delays
  if (! (weights.size() == n || weights.size() == 0) && (weights.size() == delays.size()))
  {
    message(SLIInterpreter::M_ERROR, "ConvergentConnect", "weights and delays must be lists of size n.");
    throw DimensionMismatch();
  }
    
  Compound *target_comp=dynamic_cast<Compound *>(target);
  if(target_comp !=0)
  {
    message(SLIInterpreter::M_INFO, "RandomConvergentConnect","Target ID is a subnet; I will iterate it.");
    
    NodeList target_nodes(*target_comp);
    for(NodeList::iterator tgt = target_nodes.begin(); tgt != target_nodes.end(); ++tgt)
      random_convergent_connect(source_ids, (*tgt)->get_gid(), n, weights, delays, allow_multapses, allow_autapses, syn);

    return;
  }

  librandom::RngPtr rng = get_rng(target->get_thread());
  TokenArray chosen_sources;

  std::set<long> ch_ids;
  
  long n_rnd = source_ids.size();
  
  for (size_t j = 0; j < n; ++j)
  {
    long s_id;
      
    do 
    {
      s_id  = rng->ulrand(n_rnd);
    }
    while ( ( !allow_autapses && ((index)source_ids[s_id]) == target_id )
            || ( !allow_multapses && ch_ids.find( s_id ) != ch_ids.end() ) );
      
    if (!allow_multapses)
      ch_ids.insert(s_id);
      
    chosen_sources.push_back(source_ids[s_id]);
  }
  
  convergent_connect(chosen_sources, target_id, weights, delays, syn);
}
   
// -----------------------------------------------------------------------------

void Network::compound_connect(Compound &sources, Compound &targets, int radius, index syn)
{
  vector<Node*>::const_iterator it_target_row;
  for(it_target_row = targets.begin(); it_target_row != targets.end(); ++it_target_row)
  {
    std::deque<index> scope;
    std::deque<bool> scopemiss;

    int row = (*it_target_row)->get_lid();

    Node* nodetemp = targets.at(row);
    Compound* ct = dynamic_cast<Compound *>(nodetemp);
    if(ct==0)
    {
      message(SLIInterpreter::M_ERROR, "CompoundConnect", "targets subnetwork must be compound.");
      continue;
    }
      
    //Traverse the target columns for given row.
    vector<Node*>::const_iterator it_target_col;
    for(it_target_col=ct->begin(); it_target_col!=ct->end(); ++it_target_col)
    {
      int column= (*it_target_col)->get_lid();

      if(column==0)
      {
	//Reset scope for beginning of row
	for(int n=-radius; n<=radius; ++n)
	{
	  for(int k=-radius; k<=radius; ++k)
	  { 
	    //Test if source boundary has been crossed
	    if(n>=0&&row+k>=0&&row+k<(int_t)sources.size())
	    {
	      Node* nodetemp =sources.at(row+k);
	      Compound* cn = dynamic_cast<Compound *>(nodetemp);
	      if(cn==0)
	      {
		message(SLIInterpreter::M_ERROR, "CompoundConnect", "targets subnetwork must be compound.");
		continue;
	      }
	      //Insert source node id in scope.
	      if(n<(int_t)cn->size()&&cn->at(n) != NULL)
	      {
		scope.push_back(cn->at(n)->get_gid());
		scopemiss.push_back(false);
	      }
	      else
	      {
		scope.push_back(0);
		scopemiss.push_back(true);
	      }
	    }
	    else 		  
	    {
	      scope.push_back(0);
	      scopemiss.push_back(true);
	    }
	  } 
	}
	  
	std::vector<index> scope_out;
	      
	for(int i=0; i<(int_t)scope.size(); ++i)
	{
	  if(scopemiss[i]!=true)		
	    scope_out.push_back(scope[i]);
	}
	      
	TokenArray scope_final(*(new vector<index>(scope_out.begin(), scope_out.end())));
        TokenArray weights, delays;

	convergent_connect(scope_final, ct->at(column)->get_gid(), weights, delays, syn);
      }
      else
      {
	//Adjust scope (shift rightwards).
	for(int k=-radius; k<=radius; ++k)
	{	   
	  //Boundary test
	  if(row+k>=0&&row+k<(int_t)sources.size())
	  {
	    Node* nodetemp =sources.at(row+k);
	    Compound* cn = dynamic_cast<Compound *>(nodetemp);
	    if(cn==0)
	    {
	      message(SLIInterpreter::M_ERROR, "CompoundConnect", "targets subnetwork must be compound.");
	      continue;
	    }
	    //Insert source node id in scope.
	    if(column+radius<(int_t)cn->size()&&cn->at(column+radius) != NULL)
	    {
	      scope.push_back(cn->at(column+radius)->get_gid());
	      scopemiss.push_back(false);
	    }
		      
	    else
	    {
	      scope.push_back(0);
	      scopemiss.push_back(true);
	    }
	  }
	  else 	
	  {	  
	    scope.push_back(0);
	    scopemiss.push_back(true);
	  }
		  
	  scope.pop_front();
	  scopemiss.pop_front();
	}
	      
	std::vector<index> scope_out;

	for(int i=0; i<(int_t)scope.size(); ++i)
	{
	  if(scopemiss[i]!=true)		
	    scope_out.push_back(scope[i]);
	}
	      
	TokenArray scope_final(*(new vector<index>(scope_out.begin(), scope_out.end())));
        TokenArray weights, delays;
	convergent_connect(scope_final, ct->at(column)->get_gid(), weights, delays, syn);
      }
    }
  }
}

void Network::message(int level, const char from[], const char text[])
{
  interpreter_.message(level, from, text);
}

void Network::message(int level, const std::string& loc, const std::string& msg)
{
  message(level, loc.c_str(), msg.c_str());
}

index Network::copy_model(index old_id, std::string new_name)
{
  // we can assert here, as nestmodule checks this for us
  assert (! modeldict_->known(new_name));

  Model* new_model = get_model(old_id)->clone(new_name);
  models_.push_back(new_model);
  int new_id = models_.size() - 1;
  modeldict_->insert(new_name, new_id);
  return new_id;
}

index Network::register_model(Model& m, bool private_model)
{
  std::string name = m.get_name();

  if ( !private_model && modeldict_->known(name) )
  {
    delete &m;
    throw NamingConflict("A model called '" + name + "' already exists.\n"
                         "Please choose a different name!");
  }

  const index id = models_.size();
  m.set_model_id(id);

  pristine_models_.push_back(std::pair<Model*, bool>(&m, private_model));
  models_.push_back(m.clone(name));

  if ( !private_model )
    modeldict_->insert(name, id);
  
  return id;
}

void Network::unregister_model(index m_id)
{
  Model* m = get_model(m_id);  // may throw UnknownModelID
  std::string name = m->get_name();

  if (model_in_use(m_id))
    throw ModelInUse(name);

  modeldict_->remove(name);

  // ungegister from the pristine_models_ list
  delete pristine_models_[m_id].first;
  pristine_models_[m_id].first = 0;

  // ungegister from the models_ list
  delete models_[m_id];
  models_[m_id] = 0;
}

void Network::try_unregister_model(index m_id)
{
  Model* m = get_model(m_id);  // may throw UnknownModelID
  std::string name = m->get_name();

  if (model_in_use(m_id))
    throw ModelInUse(name);
}

int Network::execute_sli_protected(DictionaryDatum state, Name cmd)
{
  static Mutex sli_mutex;
  SLIInterpreter& i=interpreter_;

  sli_mutex.lock();
  
  i.DStack->push(state); // push state dictionary as top namespace
  size_t exitlevel=i.EStack.load();
  i.EStack.push( new NameDatum(cmd));
  int result=i.execute_(exitlevel);
  i.DStack->pop();        // pop neuron's namespace
  
  if (state->known("error"))
    {
      assert(state->known(names::global_id));
      index g_id= (*state)[names::global_id];
      std::string model= getValue<std::string>((*state)[names::model]);
      std::string msg=String::compose("Error in %1 with global id %2.",model,g_id);
      
      message(SLIInterpreter::M_ERROR, cmd.toString().c_str(), msg.c_str());
      message(SLIInterpreter::M_ERROR, "execute_sli_protected", "Terminating.");
	
    scheduler_.terminate();
    }
  sli_mutex.unlock();

  return result;
}

#ifdef HAVE_MUSIC
void Network::register_music_in_port(std::string portname)
{
  std::map< std::string, std::pair<size_t, double_t> >::iterator it;
  it = music_in_portlist_.find(portname);
  if (it == music_in_portlist_.end())
    music_in_portlist_[portname] = std::pair<size_t, double_t>(1, 0.0);
  else
    music_in_portlist_[portname].first++;
}

void Network::unregister_music_in_port(std::string portname)
{
  std::map< std::string, std::pair<size_t, double_t> >::iterator it;
  it = music_in_portlist_.find(portname);
  if (it == music_in_portlist_.end())
    throw MUSICPortUnknown(portname);
  else
    music_in_portlist_[portname].first--;

  if (music_in_portlist_[portname].first == 0)
    music_in_portlist_.erase(it);
}

void Network::register_music_event_in_proxy(std::string portname, int channel, nest::Node *mp)
{
  std::map< std::string, MusicEventHandler >::iterator it;
  it = music_in_portmap_.find(portname);
  if (it == music_in_portmap_.end())
  {
    MusicEventHandler tmp(portname, music_in_portlist_[portname].second, this);
    tmp.register_channel(channel, mp);
    music_in_portmap_[portname] = tmp;
  }
  else
    it->second.register_channel(channel, mp);
}

void Network::set_music_in_port_acceptable_latency(std::string portname, double latency)
{
  std::map< std::string, std::pair<size_t, double_t> >::iterator it;
  it = music_in_portlist_.find(portname);
  if (it == music_in_portlist_.end())
    throw MUSICPortUnknown(portname);
  else
    music_in_portlist_[portname].second = latency;
}

void Network::publish_music_in_ports_()
{
  std::map< std::string, MusicEventHandler >::iterator it;
  for (it = music_in_portmap_.begin(); it != music_in_portmap_.end(); ++it)
    it->second.publish_port();
}

void Network::update_music_event_handlers_(Time const & origin, const long_t from, const long_t to)
{
  std::map< std::string, MusicEventHandler >::iterator it;
  for (it = music_in_portmap_.begin(); it != music_in_portmap_.end(); ++it)
    it->second.update(origin, from, to);
}
#endif

} //end of namespace
