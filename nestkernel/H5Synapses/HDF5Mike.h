#include "hdf5.h"
#include <deque>
#include <vector>

#include "nmpi.h"

#include "NESTNodeSynapse.h"

#ifndef HDF5MIKE_CLASS
#define HDF5MIKE_CLASS

/*
 *  struct to store name + size of file
 */
struct SFile_
{  
  std::string name;
  uint64_t size;
  
  bool operator<(const SFile_& rhs) const
  {
    return size < rhs.size;
  }
};


class HDF5Mike
{
private:
  uint32_t i_hdf5files;
  uint32_t i_datasets;
  uint32_t i_target;
  uint32_t i_source;  
  
  //variables for testing
  uint64_t& n_readSynapses;
  uint64_t& n_SynapsesInDatasets;
  
  uint32_t rank;
  uint32_t size;
  
  hid_t       file_id_;        /* handles */
  herr_t      status_;
  hid_t       gid_;
  
  
  hsize_t     number_datasets;
  uint32_t number_target_neurons, number_source_neurons;
  std::vector<uint32_t> buffer_source_neurons;
  std::vector<uint32_t> buffer_target_neurons;
  
  
  //coord file and files of synapses
  std::string coord_file_name;
  std::vector<SFile_> hdf5files;
  
  //help variable for hdf5 
  int dataset_ptr;
  bool open_dataset;
  
  inline hid_t openDataset(const char* datasetname)
  {
    return H5Dopen2 (file_id_, datasetname, H5P_DEFAULT);
  }
  
  inline void closeDataSet(hid_t& dataset_id)
  {
    status_ = H5Dclose (dataset_id);
  }
  
  void openFile(const std::string& filename);
  void closeFile();
  
  void loadSourceNeuonIds(const hid_t& dataset_id, const uint32_t& n);
  
  int getdir (const std::string dir, std::vector<SFile_>& files);
  
  void preLoadBalancing();
  
  /*
   * read dataset from hdf5 file
   */
  void loadDataset2Buffers(const uint32_t& id);
    
public:
  HDF5Mike(const std::string& con_dir,uint64_t& n_readSynapses,uint64_t& n_SynapsesInDatasets);
  ~HDF5Mike();  
  
  /*
   * 
   */
  bool endOfMikeFiles();
  
  void iterateOverAllSynapsesFromCurrentFile(std::deque<NESTNodeSynapse>& synapses);
  
  void iterateOverSynapsesFromFiles(std::deque<NESTNodeSynapse>& synapses, const uint64_t& numer_of_synapses);
  
  /*
   * extract the number of neurons from coord file
   */
  static int getNumberOfNeurons(const char* coord_file_name);
  
  
  /*
   * load coordinates for neurons
   */
  static void loadAllNeuronCoords(const char* coord_file_name, const uint32_t& numberOfNeurons, std::vector<Coords>& neurons_pos);
};


#endif