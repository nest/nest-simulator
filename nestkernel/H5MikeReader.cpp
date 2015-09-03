#include <iostream>
#include "H5Synapses/H5Synapses.h"
#include "H5Synapses/HDF5Mike.h"
#include <new>

void outOfMem() {  
#pragma omp single
  {
    std::cerr << "Out of memory\t";
    std::cerr << "rank=" << nest::Communicator::get_rank() << "\t";
    std::cerr << H5SynMEMPredictor::instance->toString() << std::endl;
  } 
  throw std::bad_alloc();
   
  //exit(1);
}

void H5MikeReader(const std::string& con_dir, const std::string& coord_file)
{
  std::set_new_handler(outOfMem);
  
  //omp_set_dynamic(true);
  
  
  const int& num_threads = nest::NestModule::get_network().get_num_threads();
  omp_set_num_threads(num_threads);
  
  std::cout << "H5MikeReader(";
  std::cout << "con_dir=" << con_dir;
  std::cout << ", coord_file=" << coord_file;
  std::cout << ") with " << num_threads << " threads" << std::endl;
  
  //if (hdf5files.size()>4)
  //  hdf5files.resize(4);
  
  H5Synapses h5Synapses;
  h5Synapses.run(con_dir, coord_file);
  
  omp_set_dynamic(false);
  omp_set_num_threads(1);
};