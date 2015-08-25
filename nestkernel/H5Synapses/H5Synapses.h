#include <vector>
#include <deque>
#include "nmpi.h"
#include "NESTNodeSynapse.h"

#include "H5SynMEMPedictor.h"

#include "HDF5Mike.h"

#ifndef H5Synapses_CLASS
#define H5Synapses_CLASS

//void NESTConnect(std::vector<NESTNodeSynapse>& synapses);
//void NESTCreateNeurons(const int& non);

enum CommunicateSynapses_Status {NOCOM,SEND, RECV, SENDRECV, UNSET};


/**
 * H5Synapses - load Synapses from HDF5 and distribute to nodes
 * 
 */

template<class T>
class GIDVector: public std::vector<T> 
{
private:
  int offset_;
  
public:
  GIDVector(): offset_(0)
  {}
  
  T& operator[] (int ix)
  {
    return std::vector<T>::operator[](ix+offset_);
  }
  
  setOffset(const int& offset)
  {
    offset_ = offset;
  }
};

class H5Synapses
{
private:
  GIDVector<char> neuron_type_;
  
  GIDVector<Coords> neurons_pos_;
  
  
  uint32_t numberOfNeurons;
  
  TraceLogger tracelogger;
  
  H5SynMEMPredictor memPredictor;
  
  struct SynapseModelProperties
  {
    nest::index synmodel_id; // NEST reference
    double min_delay; // 
    double C_delay;
    
    inline double get_delay_from_distance(const double& distance) const
    {
      const double delay = distance * C_delay;
      if (delay > min_delay)
	return delay;
      else
	return min_delay;
    }
  };
  SynapseModelProperties* synmodel_props;
  
  void CreateNeurons(const uint32_t& non);
  
  void singleConnect(const NESTNodeSynapse& synapse, nest::Node* const target_node, const nest::thread target_thread, nestio::Stopwatch::timestamp_t& connect_dur);
  
  void ConnectNeurons(const std::deque<NESTNodeSynapse>& synapses);
  void threadConnectNeurons(const std::deque<NESTNodeSynapse>& synapses);
  
  void freeSynapses(std::deque<NESTNodeSynapse>& synapses);
  CommunicateSynapses_Status CommunicateSynapses(std::deque<NESTNodeSynapse>& synapses);
  
public:
  H5Synapses();
  ~H5Synapses();
  void run(const std::string& con_dir, const std::string& hdf5_coord_file);
};

#endif