#ifndef LOGGER_H
#define LOGGER_H

#include <vector>

#include "event.h"

namespace nest
{

class RecordingDevice;

class Logger
{
public:
  Logger() {}
  virtual ~Logger() throw() {};

  virtual void enroll( const int virtual_process, RecordingDevice& device ) = 0;
  virtual void enroll( const int virtual_process,
    RecordingDevice& device,
    const std::vector< Name >& value_names ) = 0;

  virtual void initialize() = 0;
  virtual void finalize() = 0;

  virtual void write( const RecordingDevice& device, const Event& event ) = 0;
  virtual void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& ) = 0;
};

} // namespace

#endif // LOGGER_H
