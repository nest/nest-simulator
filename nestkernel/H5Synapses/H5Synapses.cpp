#include "H5Synapses.h"

#include <iostream>      
#include "nmpi.h"
#include <algorithm> 
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "timer/stopwatch.h"

#include "network.h"
#include "node.h"
#include "nestmodule.h"

#include "nest_names.h"

#include "poisson_randomdev.h"


#include "communicator.h"


#include <stdio.h>

#ifdef IS_BLUEGENE_Q
#include <spi/include/kernel/memory.h>
#endif


#define _DEBUG_MODE 1

/*
 * 
 * 
 */
void H5Synapses::CreateNeurons(const uint32_t& non)
{
  // check if possible
  // 
  if (memPredictor.preNESTCreate(non)==0)
  {
    const std::string modname = "aeif_cond_exp";
    const Token model = nest::NestModule::get_network().get_modeldict().lookup(modname);      
    // create
    const nest::index model_id = static_cast<nest::index>(model);
  
    nest::NestModule::get_network().add_node(model_id, non); 
  
    neuron_type_.resize(non, 0);
    
    // access NEST random generator
    librandom::RngPtr grng = nest::NestModule::get_network().get_grng();
    librandom::PoissonRandomDev poisson(grng, 0.16251892949777494);
    
    uint32_t i=(uint32_t)poisson(); // +1 NEST offset
    while (i<non)
    {
      neuron_type_[i]=1;
      i+=1+(uint32_t)poisson(); 
    }
    
    uint32_t non_ex=0;
    uint32_t non_in=0;
    for (i=0; i<non; i++)
      if (neuron_type_[i]==1)
	non_ex++;
      else
	non_in++;
      
    std::cout << "CreateNeurons \trank= " << nest::Communicator::get_rank() << "\tnon_ex=" << non_ex << "\tnon_in=" << non_in << std::endl;  
  }
}

void H5Synapses::singleConnect(const NESTNodeSynapse& synapse, nest::Node* const target_node, const nest::thread target_thread, nestio::Stopwatch::timestamp_t& connect_dur)
{
  nest::index source = synapse.source_neuron_;
  
  
  // check whether the target is on this process
  if (nest::NestModule::get_network().is_local_node(target_node))
  {
    // calculate delay of synapse:
    Coords& s = neurons_pos_[source];
    Coords& t = neurons_pos_[target_node->get_gid()];
    
    const double distance = sqrt ( (s.x_-t.x_)*(s.x_-t.x_) + (s.y_-t.y_)*(s.y_-t.y_) + (s.z_-t.z_)*(s.z_-t.z_) );
    
    //nest::index synmodel_id;
    
    //set synapse type and check for delay boundary
    
      // current selection of synapse is based on source neuron
      SynapseModelProperties& synmodel_prop = synmodel_props[neuron_type_[source]];
         
      //if (target_thread != section_ptr)
	//std::cout << "ConnectNeurons thread Ouups!!" << std::endl;
      
      nestio::Stopwatch::timestamp_t begin= nestio::Stopwatch::get_timestamp();
    
      nest::NestModule::get_network().connect(source, target_node, target_thread, synmodel_prop.synmodel_id, synmodel_prop.get_delay_from_distance(distance));
     
      begin = nestio::Stopwatch::get_timestamp() - begin;
      if (begin > 0)
	connect_dur+= begin;
  }
  else
  {
    std::cout << "singleConnect Ouups!!" << std::endl;
  }
}

void H5Synapses::threadConnectNeurons(const std::deque<NESTNodeSynapse>& synapses)
{
  const int& num_processes = nest::Communicator::get_num_processes();
  const int& num_vp = nest::Communicator::get_num_virtual_processes(); 
  
  if (memPredictor.preNESTConnect(synapses.size())==0)
  {
    #pragma omp parallel default(shared)
    {
      nestio::Stopwatch::timestamp_t connect_dur=0;
      nestio::Stopwatch::timestamp_t before_connect=nestio::Stopwatch::get_timestamp();
      
      
      const int tid = nest::NestModule::get_network().get_thread_id();
      
      if (num_vp != (int)(num_processes*omp_get_num_threads()))
	std::cout << "ERROR: NEST threads " << num_vp << " are not equal to OMP threads " << omp_get_num_threads() << std::endl;
    
      //without preprocessing:
      //only connect neurons which are on local thread otherwise skip
      
      for (int i=0;i<synapses.size();i++) {
	
	const nest::index target = synapses[i].target_neuron_;
	
	nest::Node* const target_node = nest::NestModule::get_network().get_node(target);
	const nest::thread target_thread = target_node->get_thread();
	
	if (target_thread == tid)  // ((synapses[i].target_neuron_ % num_vp) / num_processes == section_ptr) // synapse belongs to local thread, connect function is thread safe for this condition
	{
	  singleConnect(synapses[i], target_node, target_thread, connect_dur);
	}
      }
      tracelogger.store(tid,"nest::connect", before_connect, connect_dur);
    }
    TraceLogger::print_mem("threadConnectNeurons");
  }
}

void H5Synapses::ConnectNeurons(const std::deque<NESTNodeSynapse>& synapses)
{
  int num_processes = nest::Communicator::get_num_processes();  
  
  nestio::Stopwatch::timestamp_t connect_dur=0;
  nestio::Stopwatch::timestamp_t before_connect=nestio::Stopwatch::get_timestamp();
  
  if (memPredictor.preNESTConnect(synapses.size())==0)
  {
    for (int i=0; i< synapses.size(); i++) {
      const nest::index target = synapses[i].target_neuron_;
      nest::Node* const target_node = nest::NestModule::get_network().get_node(target);
      const nest::thread target_thread = target_node->get_thread();
      
      
      singleConnect(synapses[i], target_node, target_thread, connect_dur);
    }
  
    tracelogger.store(0,"nest::connect", before_connect, connect_dur);
  }
}

/**
 * 
 */
CommunicateSynapses_Status H5Synapses::CommunicateSynapses(std::deque<NESTNodeSynapse>& synapses)
{
  uint32_t num_processes = nest::Communicator::get_num_processes();
  
  std::stringstream sstream;
  int sendcounts[num_processes], recvcounts[num_processes], rdispls[num_processes+1], sdispls[num_processes+1];
  for (int32_t i=0;i<num_processes;i++) {
    sendcounts[i]=0;
    sdispls[i]=0;
    recvcounts[i]=-999;
    rdispls[i]=-999;
  }
  
  uint32_t* send_buffer = new uint32_t[synapses.size()*3];
  
  uint32_t* ptr_send_buffer=send_buffer;
  for (uint32_t i=0; i<synapses.size(); i++) {
    NESTNodeSynapse& syn = synapses[i];
    sendcounts[syn.node_id_]+=3;
    syn.serialize(ptr_send_buffer);
    ptr_send_buffer+=3;
  }
  
  tracelogger.begin(0, "mpi wait");
  MPI_Alltoall(sendcounts, 1, MPI_INT, recvcounts, 1, MPI_INT, MPI_COMM_WORLD);
  tracelogger.end(0, "mpi wait");
  
  rdispls[0] = 0;
  sdispls[0] = 0;
  for (uint32_t i=1;i<num_processes+1;i++) {
    sdispls[i] = sdispls[i-1] + sendcounts[i-1];
    rdispls[i] = rdispls[i-1] + recvcounts[i-1];
  }  
  
  const int32_t recv_synpases_count = rdispls[num_processes]/3;
  
  
  //implement check if recv counts does not fit in memory??
  uint32_t* recvbuf= new uint32_t[rdispls[num_processes]];
 
    
  MPI_Alltoallv(send_buffer, sendcounts, sdispls, MPI_UNSIGNED, recvbuf, recvcounts, rdispls, MPI_UNSIGNED, MPI_COMM_WORLD);
  delete[] send_buffer;
  
  //std::vector<NESTNodeSynapse> own_synapses(recv_synpases_count);  
  //synapses.swap(own_synapses);
  
  
  //fill deque with recevied entries
  synapses.resize(recv_synpases_count); 
  
  uint32_t* ptr_recv_buffer=recvbuf;
  for (uint32_t i=0; i<synapses.size(); i++) {
    NESTNodeSynapse& syn = synapses[i];
    syn.deserialize(ptr_recv_buffer);
    ptr_recv_buffer+=3;
  }
  delete[] recvbuf;

  if (sdispls[num_processes]>0 && rdispls[num_processes]>0)
    return SENDRECV;
  else if (sdispls[num_processes]>0)
    return SEND;
  else if (rdispls[num_processes]>0)
    return RECV;
  else
    return NOCOM;
}

H5Synapses::H5Synapses()
{
  //create synapse model SynapseModelProperties
  
  synmodel_props = new SynapseModelProperties[2];
  
  //for loop is comming soon ;)
  //synapses should be stored in a list - maybe from hdf5 files or from sli script - both fine
  {
    const Token synmodel = nest::NestModule::get_network().get_synapsedict().lookup("syn_in");
    synmodel_props[0].synmodel_id = static_cast<nest::index>(synmodel);
    synmodel_props[0].min_delay = 0.4;
    synmodel_props[0].C_delay = 0.001;
  }
  {
    const Token synmodel = nest::NestModule::get_network().get_synapsedict().lookup("syn_ex");
    synmodel_props[1].synmodel_id = static_cast<nest::index>(synmodel);
    synmodel_props[1].min_delay = 0.75;
    synmodel_props[1].C_delay = 0.001;
  }
}

H5Synapses::~H5Synapses()
{
  delete[] synmodel_props;
}

void H5Synapses::freeSynapses(std::deque<NESTNodeSynapse>& synapses)
{
  //std::deque<NESTNodeSynapse> empty_synapses_vec(0);
  //empty_synapses_vec.reserve(synapses.size());
  //synapses.swap(empty_synapses_vec);
  
  synapses.clear();
}

void H5Synapses::run(const std::string& con_dir, const std::string& hdf5_coord_file)
{
  int rank = nest::Communicator::get_rank();
  int size = nest::Communicator::get_num_processes();
  
  std::cout << "Start H5Synapses" << std::endl;
  std::cout << "max threads=" << omp_get_max_threads() << std::endl;
    
    
  TraceLogger::print_mem("NEST base"); 
  if (rank==0) {
     numberOfNeurons= HDF5Mike::getNumberOfNeurons(hdf5_coord_file.c_str());
     HDF5Mike::loadAllNeuronCoords(hdf5_coord_file.c_str(), numberOfNeurons, neurons_pos_);
  }
  
  MPI_Bcast(&numberOfNeurons, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  
  
  if (rank>0)
    neurons_pos_.resize(numberOfNeurons); 
  
  MPI_Bcast(&neurons_pos_[0], numberOfNeurons*sizeof(Coords), MPI_BYTE, 0, MPI_COMM_WORLD); 
  TraceLogger::print_mem("with neuron pos");
  
  // Create Neurons
  CreateNeurons(numberOfNeurons);
  neurons_pos_.setOffset(-1);
  neuron_type_.setOffset(-1);
  
  // oberserver variables for validation
  // sum over all after alg has to be equal
  uint64_t n_readSynapses=0;
  uint64_t n_SynapsesInDatasets=0;
  uint64_t n_conSynapses=0;
  
  tracelogger.begin(0,"run");
  
  CommunicateSynapses_Status com_status=UNSET;
  std::deque<NESTNodeSynapse> synapses;
  
  HDF5Mike h5Mike(con_dir,n_readSynapses,n_SynapsesInDatasets);
  
  
  uint64_t nos = 1e6; 
  
  //load datasets from files
  while (!h5Mike.endOfMikeFiles())
  {
    memPredictor.predictBestLoadNos(nos);
    
    tracelogger.begin(0,"loadSynapses");
    h5Mike.iterateOverSynapsesFromFiles(synapses, nos);      
    tracelogger.end(0,"loadSynapses");
      
    tracelogger.begin(0,"sort");
    std::sort(synapses.begin(), synapses.end());
    tracelogger.end(0,"sort");
    
    tracelogger.begin(0,"communicate");
    com_status = CommunicateSynapses(synapses);
    tracelogger.end(0,"communicate"); 
    
    n_conSynapses+=synapses.size();
    
    tracelogger.begin(0,"connect");
    threadConnectNeurons(synapses);
    tracelogger.end(0,"connect");
    
    freeSynapses(synapses);
  }
  
  //recieve datasets from other nodes
  //necessary because datasets may be distributed unbalanced
  while (com_status != NOCOM) {
    tracelogger.begin(0,"communicate");
    com_status = CommunicateSynapses(synapses);
    tracelogger.end(0,"communicate");
    
    n_conSynapses+=synapses.size();
    
    tracelogger.begin(0,"connect");
    threadConnectNeurons(synapses);
    tracelogger.end(0,"connect");
    
    freeSynapses(synapses);
  }
  
  tracelogger.end(0,"run");
  

  std::cout << "rank="<< rank << "\tn_readSynapses=" << n_readSynapses << "\tn_conSynapses=" << n_conSynapses <<  std::endl;
}