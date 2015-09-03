#include <iostream>
#include <iomanip>

#include "recording_device.h"
#include "screen_logger.h"

void
nest::ScreenLogger::enroll( const int virtual_process, RecordingDevice& device )
{
}

void
nest::ScreenLogger::enroll( const int virtual_process,
  RecordingDevice& device,
  const std::vector< Name >& value_names )
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
nest::ScreenLogger::write( const RecordingDevice& device, const Event& event )
{
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

#pragma omp critical
  std::cout << sender << "\t" << stamp.get_ms() - offset << std::endl;
}

void
nest::ScreenLogger::write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& values )
{
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

#pragma omp critical
  {
    std::cout << sender << "\t" << stamp.get_ms() - offset;

    for ( std::vector< double_t >::const_iterator val = values.begin(); val != values.end(); ++val )
    {
      std::cout << "\t" << *val;
    }

    std::cout << std::endl;
  }
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
