#include <iostream>      
#include "nmpi.h"
#include "HDF5Mike.h"
#include <algorithm> 
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "timer/scopetimer.h"
#include "timer/stopwatch.h"
#include "timer/seriestimer.h"
#include <fstream>

#include <getopt.h>

#include "H5Synapses.h"

#include "omp.h"


#ifdef _NEST2FILE 
std::ofstream NEST_ofs;
#endif

/*void NESTCreateNeurons(const int& non)
{
  #ifdef _NEST2FILE 
  NEST_ofs << "nest.Create(\"iaf_neuron\","<< non << ")" << std::endl;
  #endif
}

void NESTConnect(std::vector<NESTNodeSynapse>& synapses)
{
  for (int i=0; i<synapses.size(); i++) {
    if (synapses[i].target_neuron_  % NMPI::SIZE == NMPI::RANK){
      //std::cout << "Connect NEST from " << synapses[i].source_neuron_ << "\tto " << synapses[i].target_neuron_;
      //std::cout <<  " \033[0;32m" << "+" << "\033[0;39m" << std::endl;
      #ifdef _NEST2FILE 
      NEST_ofs << "nest.Connect(" << synapses[i].source_neuron_ << "," << synapses[i].target_neuron_ << ")" << std::endl;
      #endif
    }
    else {
      std::cout << "rank=" << NMPI::RANK <<" i=" << i <<  " Connect NEST from " << synapses[i].source_neuron_ << "\tto " << synapses[i].target_neuron_ << "\tnode " << synapses[i].node_id_;
      std::cout << " \033[0;31m" << "-" << "\033[0;39m" << std::endl;
    }
  }
}*/

/*function... might want it in some class?*/
int getdir (std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cerr << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
	std::string tmp(dirp->d_name);
	if (tmp.find(".h5")!=std::string::npos)
	  files.push_back(dir+"/"+tmp);
    }
    closedir(dp);
    return 0;
}



int main(int argc, char** argv) {
  
  std::string path;
  int max_files=-1;
  int c;
  while ((c = getopt (argc, argv, "vbp:m:")) != -1)
    switch (c)
    {
      case 'v':
	  path = "/gpfs/bbp.cscs.ch/project/proj30/genbrain";
	break;
      case 'b':
	  path = "/gpfs/bbp.cscs.ch/scratch/gss/bgq/schumann";
	break;
      case 'p':
	  path = optarg;
	break;
      case 'm':
	  max_files = atoi(optarg);
	break;
      default:
	break;
    }
  
  MPI_Init(&argc, &argv);
  
  std::cout << "Start H5Synapses" << std::endl;
  std::cout << "max threads=" << omp_get_max_threads() << std::endl;
  
  #ifdef _NEST2FILE 
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::stringstream ss;
  ss << "NEST_nodeconfig_" << rank;
  NEST_ofs.open(ss.str().c_str() , std::ofstream::out);
  #endif
  
  //nest::SeriesTimer timer_loadSynapses;
  //nest::SeriesTimer timer_sort;
  //nest::SeriesTimer timer_commuicate;
  //nest::SeriesTimer timer_connect;
  //nest::SeriesTimer timer_wait;
  std::vector<std::string> hdf5files;
  getdir(path+"/connectome_output_4", hdf5files);
  
  if (max_files>0 && max_files < hdf5files.size())
    hdf5files.resize(max_files);
  
  std::cout << "hdf5files len="<< hdf5files.size() << std::endl;
  
  H5Synapses h5Synapses;
  h5Synapses.run(hdf5files, path+"/cell_body_positions.h5");
  
  #ifdef _NEST2FILE 
  NEST_ofs.close();
  #endif
  
  MPI_Finalize();
  return 0;
}
