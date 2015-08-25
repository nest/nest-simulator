//#include "mpi.h"
#include <iostream>
#include <vector>
#include "timer/stopwatch.h"
#include "omp.h"
#include "mpi.h"

#include "communicator.h"
#include <fstream>


#ifdef IS_BLUEGENE_Q
#include <spi/include/kernel/memory.h>
#endif

#ifndef NMPI_CLASS
#define NMPI_CLASS

/*class NMPI 
{
public:
  NMPI();
  ~NMPI() {};
  
  static int SIZE;
  static int RANK;
 
};*/



#define _DEBUG_MODE 1

namespace tMPIo
{   
    /*template <typename T>
    void ppTable(T v, const char* info)
    {
      T r[NMPI::SIZE];
      MPI_Gather(&v, sizeof(T), MPI_CHAR, &r, sizeof(T), MPI_CHAR, 0, MPI_COMM_WORLD);
      
      if (NMPI::RANK==0) {
	std::cout << "=========================================" << std::endl;
	std::cout << "|  " << info << std::endl;
	std::cout << "=========================================" << std::endl;
	std::cout << "Node\t";
	for (int i=0;i<NMPI::SIZE;i++)
	  std::cout << i << "\t";
	std::cout << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	std::cout << "Value\t";
	for (int i=0;i<NMPI::SIZE;i++)
	  std::cout << r[i] << "\t";
	std::cout << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
      }
    }
    
    template <typename T>
    void ppTable(T* v, const unsigned int& n, const char* info)
    {
      int ns[NMPI::SIZE];
      int displ[NMPI::SIZE];
      int recvcounts[NMPI::SIZE];
      MPI_Gather(&n, 1, MPI_INT, &ns, 1, MPI_INT, 0, MPI_COMM_WORLD);
      
      recvcounts[0]=ns[0]*sizeof(T);
      displ[0]=0;
      int recvbufsize = recvcounts[0];
      for (int i=1; i< NMPI::SIZE;i++) {
	recvcounts[i] = ns[i]*sizeof(T);
	displ[i]=displ[i-1]+recvcounts[i-1];
	recvbufsize+=ns[i];
      }
      T recvbuf[recvbufsize];
      
      MPI_Gatherv(v, sizeof(T)*n, MPI_CHAR, recvbuf, recvcounts, displ, MPI_CHAR, 0, MPI_COMM_WORLD);
      
      if (NMPI::RANK==0) {
	std::cout << "=========================================" << std::endl;
	std::cout << "|  " << info << std::endl;
	std::cout << "=========================================" << std::endl;
	std::cout << "Node\t";
	for (int i=0;i<NMPI::SIZE;i++)
	  std::cout << i << "\t";
	std::cout << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	std::cout << "Value\t";
	for (int i=0;i<NMPI::SIZE;i++)
	  std::cout << recvbuf[i] << "\t";
	std::cout << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
      }
    }*/
    
};

class TraceLogger 
{
  std::vector< std::vector<std::string> > labels_;
  std::vector< std::vector<nestio::Stopwatch::timestamp_t> > begin_;
  std::vector< std::vector<nestio::Stopwatch::timestamp_t> > end_;
  std::vector< std::vector<int> > dataset_;
  
  std::vector<nestio::Stopwatch::timestamp_t> offset;
 
public:
  TraceLogger()
  {
    #ifdef _DEBUG_MODE
    const int num_threads = omp_get_max_threads();
    labels_.resize(num_threads);
    begin_.resize(num_threads);
    end_.resize(num_threads);
    dataset_.resize(num_threads);
    
    
    offset.resize(num_threads);
    
    #pragma omp parallel
    {
      nestio::Stopwatch::timestamp_t tmp_t = nestio::Stopwatch::get_timestamp();
      const int thread_num = omp_get_thread_num();
      offset[thread_num] =  tmp_t;
    }
    #endif
  }
  
  ~TraceLogger()
  {
    #ifdef _DEBUG_MODE
    
    const uint32_t rank = nest::Communicator::get_rank();
    const uint32_t size = nest::Communicator::get_num_processes();
    
    begin(0,"writeLog");
    for(int r=0; r<size; r++ ) {
      MPI_Barrier( MPI_COMM_WORLD );
      if (r==rank) {
	std::ofstream traceFile;
	traceFile.open("traceFile.csv",std::ofstream::out | std::ofstream::app);
	std::stringstream ss2;
	ss2 << rank;
	
	end(0,"writeLog");
	
	print_all_csv(traceFile);
	traceFile.close();
      }
    }
    #endif
  }
  
  void store(const int& id, const std::string& label, const nestio::Stopwatch::timestamp_t& begin, const nestio::Stopwatch::timestamp_t& end)
  { 
    #ifdef _DEBUG_MODE
    const int thread_num = omp_get_thread_num();
    
    //std::cout << "begin rank=" << nest::Communicator::get_rank() << " thread=" << thread_num << " id=" << id << " label=" << label << std::endl;
    
    //std::cout << "thread_num=" << thread_num << std::endl;
    labels_[thread_num].push_back(label);
    begin_[thread_num].push_back(begin-offset[thread_num]);
    dataset_[thread_num].push_back(id);
    end_[thread_num].push_back(begin-offset[thread_num]+end);
    #endif
  }
  
  void begin(const int& id, const std::string& label)
  { 
    #ifdef _DEBUG_MODE
    const int thread_num = omp_get_thread_num();
    
    //std::cout << "begin rank=" << nest::Communicator::get_rank() << " thread=" << thread_num << " id=" << id << " label=" << label << std::endl;
    
    //std::cout << "thread_num=" << thread_num << std::endl;
    labels_[thread_num].push_back(label);
    begin_[thread_num].push_back(nestio::Stopwatch::get_timestamp()-offset[thread_num]);
    dataset_[thread_num].push_back(id);
    end_[thread_num].push_back(0);
    #endif
  }
  void end(const int& id, const std::string& label)
  {
    #ifdef _DEBUG_MODE
    const int thread_num = omp_get_thread_num();
    //std::cout << "end rank=" << nest::Communicator::get_rank() << " thread=" << thread_num << " id=" << id << " label=" << label << std::endl;
    for (int i=labels_[thread_num].size()-1;i>=0;i--)
    {
      if (labels_[thread_num][i] == label && dataset_[thread_num][i] == id && end_[thread_num][i]==0) {
	end_[thread_num][i] = nestio::Stopwatch::get_timestamp()-offset[thread_num];
	break;
      }
    }
    #endif
  }
  
  static void print_mem(const std::string& info)
  {
    #ifdef _DEBUG_MODE
    #ifdef IS_BLUEGENE_Q
    uint64_t shared, persist, heapavail, stackavail, stack, heap, guard, mmap;
    Kernel_GetMemorySize(KERNEL_MEMSIZE_SHARED, &shared);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_PERSIST, &persist);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPAVAIL, &heapavail);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_STACKAVAIL, &stackavail);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_STACK, &stack);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAP, &heap);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_GUARD, &guard);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_MMAP, &mmap);
    std::cout << info << "\trank= " << nest::Communicator::get_rank() << "\theap= " << (double)(heap)/(1024*1024) << "\tstack= " << (double)(stack)/(1024*1024) << "\thavail= " << (double)heapavail/(1024*1024) << "\tsavail= " << (double)stackavail/(1024*1024) << "\tshared= " << (double)shared/(1024*1024) << "\tpersist= " << (double)persist/(1024*1024) << "\tguard= " << (double)guard/(1024*1024) << "\tmmap= " << (double)mmap/(1024*1024) << std::endl;
    #endif
    #endif
  }
  
private:
  #ifdef _DEBUG_MODE
  void print_all_csv(std::ostream& os)
  {  
    for (int i=0; i<labels_.size(); i++) {
      for (int j=0; j<labels_[i].size(); j++) {
	os << nest::Communicator::get_rank() << ";";
	os << i << ";";
	os << labels_[i][j] << ";";
	os << dataset_[i][j] << ";";
	os << begin_[i][j] << ";";
	os << end_[i][j] << ";";
	os << std::endl;
      }
    }
  }
  #endif
  
};

#endif
