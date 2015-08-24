#include "recording_device.h"
#include "screen_logger.h"

void
nest::ScreenLogger::signup( const int virtual_process, const RecordingDevice& device )
{
}

void
nest::ScreenLogger::initialize()
{
  if ( initialized_ )
    return;

  std::cout << std::fixed;
  std::cout << std::setprecision( P_.precision_ );

  initialized_ = true;
}

void
nest::ScreenLogger::finalize()
{
}

void
nest::ScreenLogger::write_event( const RecordingDevice& device, const Event& event )
{
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::cout << sender << "\t" << stamp.get_ms() - offset;
}

void
nest::ScreenLogger::write_value( const RecordingDevice& device, const double& value )
{
  std::cout << "\t" << value;
}

void
nest::ScreenLogger::write_end( const RecordingDevice& device )
{
  std::cout << std::endl;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::ScreenLogger::Parameters_::Parameters_()
  : precision_( 3 )
{
}

void
nest::ScreenLogger::Parameters_::get( const ScreenLogger& sl, DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
}

void
nest::ScreenLogger::Parameters_::set( const ScreenLogger& sl, const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
}
