#include "H5SynMEMPedictor.h"

H5SynMEMPredictor::H5SynMEMPredictor(): predicted_mem_used(0), number_of_neurons(0), number_of_synapses(0), max_nos(1e9)
{
  updateMEM();
  measured_mem_free_begin = measured_mem_free;
}

void H5SynMEMPredictor::updateMEM()
{
  #ifdef IS_BLUEGENE_Q
  Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPAVAIL, &measured_mem_free);
  #else
  measured_mem_free = 0;
  measured_mem_free--;
  #endif
  
  
  predicted_mem_used=0;
}

int H5SynMEMPredictor::preNESTCreate(const uint32_t& non)
{
  updateMEM();
  predicted_mem_used += non * 1100;
  
  number_of_neurons += non;
  if (predicted_mem_used>measured_mem_free)
    return -1; 
  else
    return 0;
}

int H5SynMEMPredictor::preNESTConnect(const uint64_t& nos)
{
  updateMEM();
  predicted_mem_used += nos * 128;
  
  number_of_synapses += nos;
  if (predicted_mem_used>measured_mem_free)
    return -1; 
  else
    return 0;
}

void H5SynMEMPredictor::predictBestLoadNos(uint64_t& nos)
{
  updateMEM();
  
  nos = (measured_mem_free-predicted_mem_used)/128;
  
  if (nos > max_nos)
    nos = max_nos;
}