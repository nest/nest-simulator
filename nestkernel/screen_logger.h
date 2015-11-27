/*
 *  screen_logger.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCREEN_LOGGER_H
#define SCREEN_LOGGER_H

#include "logger.h"

namespace nest
{

class ScreenLogger : public Logger
{
public:
  ScreenLogger()
  {
  }

  ~ScreenLogger() throw()
  {
  }

  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device, const std::vector< Name >& value_names );

  void initialize();
  void finalize();
  void synchronize() {}

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
};

inline void
ScreenLogger::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // SCREEN_LOGGER_H
