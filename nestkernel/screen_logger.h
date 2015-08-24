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

  void signup( const int virtual_process, const RecordingDevice& device );
  void initialize();
  void finalize();
  void write_event( const RecordingDevice& device, const Event& event );
  void write_value( const double& value );
  void write_end();

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
