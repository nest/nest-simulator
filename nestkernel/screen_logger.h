#ifndef SCREEN_LOGGER_H
#define SCREEN_LOGGER_H

#include "logger.h"

namespace nest
{

class ScreenLogger : public Logger
{
public:
  ScreenLogger()
    : initialized_( false )
  {
  }

  ~ScreenLogger() throw()
  {
  }

  void enroll( const int virtual_process, RecordingDevice& device );
  void initialize();
  void finalize();
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& );

private:
  struct Parameters_
  {
    long precision_;

    Parameters_();

    void get( const ScreenLogger&, DictionaryDatum& ) const;
    void set( const ScreenLogger&, const DictionaryDatum& );
  };

  Parameters_ P_;

  bool initialized_;
};

} // namespace

#endif // SCREEN_LOGGER_H
