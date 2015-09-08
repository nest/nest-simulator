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
  void enroll( const int virtual_process,
    RecordingDevice& device,
    const std::vector< Name >& value_names );
  void initialize();
  void finalize();
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

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

inline void
ScreenLogger::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // SCREEN_LOGGER_H
