//#include "mpi.h"
#include <iostream>
#include <vector>
#include "timer/stopwatch.h"
#include "omp.h"
#include "mpi.h"

#include "communicator.h"
#include <fstream>

#define HAVE_SIONLIB 1

#ifdef HAVE_SIONLIB
#include <sion.h>
#endif

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

#ifdef HAVE_SIONLIB
class SionWriter
{
private:
  int sid_;
  unsigned int n_datasets_;
  struct LOGHEAD
  {
    int size;
    int n_items;
    char datatype[256];
    char name[256];
  };
  std::vector<LOGHEAD> logheads;
  
  template <typename Container>
  size_t value_size(const Container &)
  {
      return sizeof(typename Container::value_type);
  }

  template <typename Container>
  const char* value_name(const Container &)
  {
      return typeid(typename Container::value_type).name();
  }
  
  
public:
  SionWriter(const char* filename)
  {	
    /* SION parameters */
    int sid, numFiles, globalrank;
    MPI_Comm lComm;
    sion_int64 chunksize, left, bwrote;
    sion_int32 fsblksize;
    char fname[256], *newfname=NULL;

    /* open parameters */
    chunksize = 10; globalrank = nest::Communicator::get_rank();
    strcpy(fname, filename);
    numFiles = 1;
    fsblksize = -1;
    
    sid_ = sion_paropen_ompi(fname, "bw", &numFiles,
    MPI_COMM_WORLD, &lComm,
    &chunksize, &fsblksize,
    &globalrank,
    NULL, &newfname);
  }
  ~SionWriter()
  {	
    sion_parclose_mpi(sid_);
  }
  
  
  template <typename Container>
  int registerVector(const char* name,const Container & vec)
  {
    LOGHEAD logheadentry;
    
    logheadentry.size = value_size(vec);
    logheadentry.n_items = vec.size();
    strncpy(logheadentry.datatype,value_name(vec),255);
    strncpy(logheadentry.name,name,255);
    
    logheads.push_back(logheadentry);
    
    return logheads.size()-1;
  }
  
  void writeHeader()
  {
    unsigned int version=1;
    unsigned int n_head=logheads.size();   
    
    sion_fwrite(&version,sizeof(int), 1, sid_);
    sion_fwrite(&n_head,sizeof(int), 1, sid_);
    for (int i=0;i<n_head;i++) {
      sion_fwrite(&logheads[i].size,sizeof(int), 1, sid_);
      sion_fwrite(&logheads[i].n_items,sizeof(int), 1, sid_);
      sion_fwrite(logheads[i].datatype,sizeof(char), 256, sid_);
      sion_fwrite(logheads[i].name,sizeof(char), 256, sid_);
    }
  }
  template <typename Container>
  void writeVector(const int& index,const Container & vec)
  {
    sion_fwrite(&vec[0],logheads[index].size, logheads[index].n_items, sid_);
  }
  
};
#endif // HAVE_SIONLIB

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
    #ifdef HAVE_SION
    writeSionLog
    #else //HAVE_SION
    writeCSVLog();
    #endif //HAVE_SION
    #endif
  }
  
  
  void writeSionLog()
  {
    const uint32_t rank = nest::Communicator::get_rank();
    char recv_trace_filename[256] = {0};
    if (rank==0)
    {
      std::string trace_filename = findNotExistingFilename("traceFile_", ".sion");
      strncpy(recv_trace_filename, trace_filename.c_str(), 255);
    }
    MPI_Bcast(recv_trace_filename, 256, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    #pragma omp parallel
    {
      const int thread_num = omp_get_thread_num();
      
      SionWriter sw(recv_trace_filename);
      
      sw.registerVector("labels", labels_[thread_num]);
      sw.registerVector("dataset", dataset_[thread_num]);
      sw.registerVector("begin", begin_[thread_num]);
      sw.registerVector("end", end_[thread_num]);
      
      sw.writeHeader();
      
      sw.writeVector(0,labels_[thread_num]);
      sw.writeVector(0,dataset_[thread_num]);
      sw.writeVector(0,begin_[thread_num]);
      sw.writeVector(0,end_[thread_num]);
    }
  }
  
  void writeCSVLog()
  {
    const uint32_t size = nest::Communicator::get_num_processes();
    const uint32_t rank = nest::Communicator::get_rank();
    
    // find not used filename
    char recv_trace_filename[256] = {0};
    if (rank==0)
    {
      std::string trace_filename = findNotExistingFilename("traceFile_", ".csv");
      strncpy(recv_trace_filename, trace_filename.c_str(), 255);
    }
    MPI_Bcast(recv_trace_filename, 256, MPI_CHAR, 0, MPI_COMM_WORLD);  
    
    begin(0,"writeLog");
    for(int r=0; r<size; r++ ) {
      MPI_Barrier( MPI_COMM_WORLD );
      if (r==rank) {
	std::ofstream traceFile;
	traceFile.open(recv_trace_filename,std::ofstream::out | std::ofstream::app);
	std::stringstream ss2;
	ss2 << rank;
	
	end(0,"writeLog");
	
	print_all_csv(traceFile);
	traceFile.close();
      }
    }
  }
  
  std::string findNotExistingFilename(const char* prefix, const char* suffix)
  {
    const uint32_t rank = nest::Communicator::get_rank();
    std::string trace_filename;
      int i=0;
      do {
	std::stringstream ss;
	ss << prefix << std::setfill('0') << std::setw(3) << i << ".csv";
	i++;
	trace_filename = ss.str();
      } while (is_file_exist(trace_filename.c_str()));
      
      return trace_filename;   
  }
  
  bool is_file_exist(const char *fileName)
  {
      std::ifstream infile(fileName);
      return infile.good();
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
