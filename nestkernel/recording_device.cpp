/*
 *  recording_device.cpp
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

#include "recording_device.h"

// C++ includes:
#include <iomanip>
#include <iostream> // using cerr for error message.

// Generated includes:
#include "config.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"
#include "fdstream.h"
#include "iostreamdatum.h"
#include "sliexceptions.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::RecordingDevice::Parameters_::Parameters_( const std::string& file_ext,
  bool withtime,
  bool withgid,
  bool withweight,
  bool withtargetgid,
  bool withport,
  bool withrport )
  : to_file_( false )
  , to_screen_( false )
  , to_memory_( true )
  , to_accumulator_( false )
  , time_in_steps_( false )
  , precise_times_( false )
  , withgid_( withgid )
  , withtime_( withtime )
  , withweight_( withweight )
  , withtargetgid_( withtargetgid )
  , withport_( withport )
  , withrport_( withrport )
  , precision_( 3 )
  , scientific_( false )
  , user_set_precise_times_( false )
  , user_set_precision_( false )
  , binary_( false )
  , fbuffer_size_( -1 ) // -1 marks use of default buffer
  , label_()
  , file_ext_( file_ext )
  , filename_()
  , close_after_simulate_( false )
  , flush_after_simulate_( true )
  , flush_records_( false )
  , close_on_reset_( true )
  , use_gid_in_filename_( true )
{
}

nest::RecordingDevice::State_::State_()
  : events_( 0 )
  , event_senders_()
  , event_targets_()
  , event_ports_()
  , event_rports_()
  , event_times_ms_()
  , event_times_steps_()
  , event_times_offsets_()
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::RecordingDevice::Parameters_::get( const RecordingDevice& rd,
  DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;

  ( *d )[ names::withtime ] = withtime_;
  ( *d )[ names::withgid ] = withgid_;
  ( *d )[ names::withweight ] = withweight_;
  ( *d )[ names::withtargetgid ] = withtargetgid_;
  ( *d )[ names::withport ] = withport_;
  ( *d )[ names::withrport ] = withrport_;

  ( *d )[ names::time_in_steps ] = time_in_steps_;
  if ( rd.mode_ == RecordingDevice::WEIGHT_RECORDER
    or rd.mode_ == RecordingDevice::SPIKE_DETECTOR
    or rd.mode_ == RecordingDevice::SPIN_DETECTOR )
  {
    ( *d )[ names::precise_times ] = precise_times_;
  }
  // We must maintain /to_file, /to_screen, and /to_memory, because
  // the new /record_to feature is not working in Pynest.
  ( *d )[ names::to_screen ] = to_screen_;
  ( *d )[ names::to_memory ] = to_memory_;
  ( *d )[ names::to_file ] = to_file_;
  if ( rd.mode_ == RecordingDevice::MULTIMETER )
  {
    ( *d )[ names::to_accumulator ] = to_accumulator_;
  }
  ArrayDatum ad;
  if ( to_file_ )
  {
    ad.push_back( LiteralDatum( names::file ) );
  }
  if ( to_memory_ )
  {
    ad.push_back( LiteralDatum( names::memory ) );
  }
  if ( to_screen_ )
  {
    ad.push_back( LiteralDatum( names::screen ) );
  }
  if ( rd.mode_ == RecordingDevice::MULTIMETER )
  {
    if ( to_accumulator_ )
    {
      ad.push_back( LiteralDatum( names::accumulator ) );
    }
  }
  ( *d )[ names::record_to ] = ad;

  ( *d )[ names::file_extension ] = file_ext_;
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::scientific ] = scientific_;

  ( *d )[ names::binary ] = binary_;
  ( *d )[ names::fbuffer_size ] = fbuffer_size_;

  ( *d )[ names::close_after_simulate ] = close_after_simulate_;
  ( *d )[ names::flush_after_simulate ] = flush_after_simulate_;
  ( *d )[ names::flush_records ] = flush_records_;
  ( *d )[ names::close_on_reset ] = close_on_reset_;

  ( *d )[ names::use_gid_in_filename ] = use_gid_in_filename_;

  if ( to_file_ && not filename_.empty() )
  {
    initialize_property_array( d, names::filenames );
    append_property( d, names::filenames, filename_ );
  }
}

void
nest::RecordingDevice::Parameters_::set( const RecordingDevice& rd,
  Buffers_& B,
  const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::label, label_ );
  updateValue< bool >( d, names::withgid, withgid_ );
  updateValue< bool >( d, names::withtime, withtime_ );
  updateValue< bool >( d, names::withweight, withweight_ );
  updateValue< bool >( d, names::withtargetgid, withtargetgid_ );
  updateValue< bool >( d, names::withport, withport_ );
  updateValue< bool >( d, names::withrport, withrport_ );
  updateValue< bool >( d, names::time_in_steps, time_in_steps_ );
  if ( rd.mode_ == RecordingDevice::SPIKE_DETECTOR
    or rd.mode_ == RecordingDevice::WEIGHT_RECORDER
    or rd.mode_ == RecordingDevice::SPIN_DETECTOR )
  {
    if ( d->known( names::precise_times ) )
    {
      user_set_precise_times_ = true;
      updateValue< bool >( d, names::precise_times, precise_times_ );
    }
  }
  updateValue< std::string >( d, names::file_extension, file_ext_ );
  if ( d->known( names::precision ) )
  {
    user_set_precision_ = true;
    updateValue< long >( d, names::precision, precision_ );
  }
  updateValue< bool >( d, names::scientific, scientific_ );

  updateValue< bool >( d, names::binary, binary_ );

  long fbuffer_size = -1;
  if ( updateValue< long >( d, names::fbuffer_size, fbuffer_size ) )
  {
    if ( B.fs_.is_open() )
    {
      throw BadProperty( "fbuffer_size cannot be set on open files." );
    }

    // prohibit negative sizes but allow Create_l_i_D to reset -1 on prototype
    if ( fbuffer_size < 0
      and not( rd.node_.get_vp() == -1 and fbuffer_size == -1 ) )
    {
      throw BadProperty( "fbuffer_size must be >= 0." );
    }
    fbuffer_size_ = fbuffer_size;
  }

  updateValue< bool >( d, names::close_after_simulate, close_after_simulate_ );
  updateValue< bool >( d, names::flush_after_simulate, flush_after_simulate_ );
  updateValue< bool >( d, names::flush_records, flush_records_ );
  updateValue< bool >( d, names::close_on_reset, close_on_reset_ );

  bool tmp_use_gid_in_filename = true;
  updateValue< bool >( d, names::use_gid_in_filename, tmp_use_gid_in_filename );

  // In Pynest we cannot use /record_to, because we have no way to pass
  // values as LiteralDatum. Thus, we must keep the boolean flags.
  // We must have || rec_change at the end, otherwise short-circuiting may
  // mean that some flags are not read.
  bool rec_change = false;
  rec_change =
    updateValue< bool >( d, names::to_screen, to_screen_ ) || rec_change;
  rec_change =
    updateValue< bool >( d, names::to_memory, to_memory_ ) || rec_change;
  rec_change = updateValue< bool >( d, names::to_file, to_file_ ) || rec_change;
  if ( rd.mode_ == RecordingDevice::MULTIMETER )
  {
    rec_change = updateValue< bool >(
                   d, names::to_accumulator, to_accumulator_ ) || rec_change;
  }

  const bool have_record_to = d->known( names::record_to );
  if ( have_record_to )
  {
    // clear all flags
    to_file_ = to_screen_ = to_memory_ = to_accumulator_ = false;

    // check for flags present in array, could be far more elegant ...
    ArrayDatum ad = getValue< ArrayDatum >( d, names::record_to );
    for ( Token* t = ad.begin(); t != ad.end(); ++t )
    {
      if ( *t == LiteralDatum( names::file )
        || *t == Token( names::file.toString() ) )
      {
        to_file_ = true;
      }
      else if ( *t == LiteralDatum( names::memory )
        || *t == Token( names::memory.toString() ) )
      {
        to_memory_ = true;
      }
      else if ( *t == LiteralDatum( names::screen )
        || *t == Token( names::screen.toString() ) )
      {
        to_screen_ = true;
      }
      else if ( rd.mode_ == RecordingDevice::MULTIMETER
        && ( *t == LiteralDatum( names::accumulator )
                  || *t == Token( names::accumulator.toString() ) ) )
      {
        to_accumulator_ = true;
      }
      else
      {
        if ( rd.mode_ == RecordingDevice::MULTIMETER )
        {
          throw BadProperty(
            "/to_record must be array, allowed entries: /file, /memory, "
            "/screen, /accumulator." );
        }
        else
        {
          throw BadProperty(
            "/to_record must be array, allowed entries: /file, /memory, "
            "/screen." );
        }
      }
    }
  }

  if ( ( rec_change || have_record_to ) && to_file_ && to_memory_ )
  {
    LOG( M_INFO,
      "RecordingDevice::set_status",
      "Data will be recorded to file and to memory." );
  }

  if ( to_accumulator_
    && ( to_file_ || to_screen_ || to_memory_ || withgid_ || withweight_ ) )
  {
    to_file_ = to_screen_ = to_memory_ = withgid_ = withweight_ = false;
    LOG( M_WARNING,
      "RecordingDevice::set_status()",
      "Accumulator mode selected. All incompatible properties "
      "(to_file, to_screen, to_memory, withgid, withweight) "
      "have been set to false." );
  }

  if ( not tmp_use_gid_in_filename and label_.empty() )
  {
    throw BadProperty(
      "If /use_gid_in_filename is false, /label must be specified." );
  }
  else
  {
    use_gid_in_filename_ = tmp_use_gid_in_filename;
  }
}

void
nest::RecordingDevice::State_::get( DictionaryDatum& d,
  const Parameters_& p ) const
{
  // if we already have the n_events entry, we add to it, otherwise we create it
  if ( d->known( names::n_events ) )
  {
    ( *d )[ names::n_events ] =
      getValue< long >( d, names::n_events ) + events_;
  }
  else
  {
    ( *d )[ names::n_events ] = events_;
  }

  DictionaryDatum dict;

  // if we already have the events dict, we use it, otherwise we create it
  if ( not d->known( names::events ) )
  {
    dict = DictionaryDatum( new Dictionary );
  }
  else
  {
    dict = getValue< DictionaryDatum >( d, names::events );
  }

  if ( p.withgid_ )
  {
    assert( not p.to_accumulator_ );
    initialize_property_intvector( dict, names::senders );
    append_property(
      dict, names::senders, std::vector< long >( event_senders_ ) );
  }

  if ( p.withweight_ )
  {
    assert( not p.to_accumulator_ );
    initialize_property_doublevector( dict, names::weights );
    append_property(
      dict, names::weights, std::vector< double >( event_weights_ ) );
  }

  if ( p.withtargetgid_ )
  {
    assert( not p.to_accumulator_ );
    initialize_property_intvector( dict, names::targets );
    append_property(
      dict, names::targets, std::vector< long >( event_targets_ ) );
  }

  if ( p.withport_ )
  {
    assert( not p.to_accumulator_ );
    initialize_property_intvector( dict, names::ports );
    append_property( dict, names::ports, std::vector< long >( event_ports_ ) );
  }

  if ( p.withrport_ )
  {
    assert( not p.to_accumulator_ );
    initialize_property_intvector( dict, names::rports );
    append_property(
      dict, names::rports, std::vector< long >( event_rports_ ) );
  }

  if ( p.withtime_ )
  {
    if ( p.time_in_steps_ )
    {
      initialize_property_intvector( dict, names::times );
      // When not accumulating, we just add time data. When accumulating, we
      // must add time data only from one thread and ensure that time data from
      // other threads is either empty of identical to what is present.
      if ( not p.to_accumulator_ )
      {
        append_property(
          dict, names::times, std::vector< long >( event_times_steps_ ) );
      }
      else
      {
        provide_property(
          dict, names::times, std::vector< long >( event_times_steps_ ) );
      }

      if ( p.precise_times_ )
      {
        initialize_property_doublevector( dict, names::offsets );
        if ( not p.to_accumulator_ )
        {
          append_property( dict,
            names::offsets,
            std::vector< double >( event_times_offsets_ ) );
        }
        else
        {
          provide_property( dict,
            names::offsets,
            std::vector< double >( event_times_offsets_ ) );
        }
      }
    }
    else
    {
      initialize_property_doublevector( dict, names::times );
      if ( not p.to_accumulator_ )
      {
        append_property(
          dict, names::times, std::vector< double >( event_times_ms_ ) );
      }
      else
      {
        provide_property(
          dict, names::times, std::vector< double >( event_times_ms_ ) );
      }
    }
  }

  ( *d )[ names::events ] = dict;
}

void
nest::RecordingDevice::State_::set( const DictionaryDatum& d )
{
  long ne = 0;
  if ( updateValue< long >( d, names::n_events, ne ) )
  {
    if ( ne == 0 )
    {
      events_ = 0;
    }
    else
    {
      throw BadProperty( "n_events can only be set to 0." );
    }
  }
}

nest::RecordingDevice::Buffers_::Buffers_()
  : fs_()
  , fbuffer_( 0 )
  , fbuffer_size_( -1 )
{
}

nest::RecordingDevice::Buffers_::~Buffers_()
{
  if ( fbuffer_ )
  {
    delete[] fbuffer_;
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor and destructor for device
 * ---------------------------------------------------------------- */

nest::RecordingDevice::RecordingDevice( const Node& n,
  Mode mode,
  const std::string& file_ext,
  bool withtime,
  bool withgid,
  bool withweight,
  bool withtargetgid,
  bool withport,
  bool withrport )
  : Device()
  , node_( n )
  , mode_( mode )
  , P_( file_ext,
      withtime,
      withgid,
      withweight,
      withtargetgid,
      withport,
      withrport )
  , S_()
  , B_()
{
}

nest::RecordingDevice::RecordingDevice( const Node& n,
  const RecordingDevice& d )
  : Device( d )
  , node_( n )
  , mode_( d.mode_ )
  , P_( d.P_ )
  , S_( d.S_ )
  , B_() // do not copy
{
}

/* ----------------------------------------------------------------
 * Device initialization functions
 * ---------------------------------------------------------------- */

void
nest::RecordingDevice::init_parameters( const RecordingDevice& pr )
{
  Device::init_parameters( pr );

  P_ = pr.P_;
  S_ = pr.S_;
}

void
nest::RecordingDevice::init_state( const RecordingDevice& pr )
{
  Device::init_state( pr );
  S_ = pr.S_;
}

void
nest::RecordingDevice::init_buffers()
{
  Device::init_buffers();

  // we only close files here, opening is left to calibrate()
  // we must not touch B_.fbuffer_ here, as it will be used
  // as long as B_.fs_ exists.
  if ( P_.close_on_reset_ && B_.fs_.is_open() )
  {
    B_.fs_.close();
    P_.filename_.clear(); // filename_ only visible while file open
  }
}

void
nest::RecordingDevice::calibrate()
{
  Device::calibrate();

  if ( P_.to_file_ )
  {
    // do we need to (re-)open the file
    bool newfile = false;

    if ( not B_.fs_.is_open() )
    {
      newfile = true; // no file from before
      P_.filename_ = build_filename_();
    }
    else
    {
      std::string newname = build_filename_();
      if ( newname != P_.filename_ )
      {
        std::string msg = String::compose(
          "Closing file '%1', opening file '%2'", P_.filename_, newname );
        LOG( M_INFO, "RecordingDevice::calibrate()", msg );

        B_.fs_.close(); // close old file
        P_.filename_ = newname;
        newfile = true;
      }
    }

    if ( newfile )
    {
      assert( not B_.fs_.is_open() );

      // pubsetbuf() must be called before the opening file: otherwise, it has
      // no effect (libstdc++) or may lead to undefined behavior (libc++), see
      // http://en.cppreference.com/w/cpp/io/basic_filebuf/setbuf.
      if ( P_.fbuffer_size_ >= 0 )
      {
        if ( B_.fbuffer_ )
        {
          delete[] B_.fbuffer_;
          B_.fbuffer_ = 0;
        }
        // invariant: B_.fbuffer_ == 0

        if ( P_.fbuffer_size_ > 0 )
        {
          B_.fbuffer_ = new char[ P_.fbuffer_size_ ];
        }
        // invariant: ( P_.fbuffer_size_ == 0 and B_.fbuffer_ == 0 )
        //            or
        //            ( P_.fbuffer_size_ > 0 and B_.fbuffer_ != 0 )

        B_.fbuffer_size_ = P_.fbuffer_size_;

        std::basic_streambuf< char >* res =
          B_.fs_.rdbuf()->pubsetbuf( B_.fbuffer_, B_.fbuffer_size_ );

        if ( res == 0 )
        {
          LOG( M_ERROR,
            "RecordingDevice::calibrate()",
            "Failed to set file buffer." );
          throw IOError();
        }
      }

      if ( kernel().io_manager.overwrite_files() )
      {
        if ( P_.binary_ )
        {
          B_.fs_.open( P_.filename_.c_str(), std::ios::out | std::ios::binary );
        }
        else
        {
          B_.fs_.open( P_.filename_.c_str() );
        }
      }
      else
      {
        // try opening for reading
        std::ifstream test( P_.filename_.c_str() );
        if ( test.good() )
        {
          std::string msg = String::compose(
            "The device file '%1' exists already and will not be overwritten. "
            "Please change data_path, data_prefix or label, or set "
            "/overwrite_files to true in the root node.",
            P_.filename_ );
          LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
          throw IOError();
        }
        else
        {
          test.close();
        }

        // file does not exist, so we can open
        if ( P_.binary_ )
        {
          B_.fs_.open( P_.filename_.c_str(), std::ios::out | std::ios::binary );
        }
        else
        {
          B_.fs_.open( P_.filename_.c_str() );
        }
      }
    }

    if ( not B_.fs_.good() )
    {
      std::string msg = String::compose(
        "I/O error while opening file '%1'. "
        "This may be caused by too many open files in networks "
        "with many recording devices and threads.",
        P_.filename_ );
      LOG( M_ERROR, "RecordingDevice::calibrate()", msg );

      if ( B_.fs_.is_open() )
      {
        B_.fs_.close();
      }
      P_.filename_.clear();
      throw IOError();
    }

    /* Set formatting
       Formatting is not applied to std::cout for screen output,
       since different devices may have different settings and
       this would lead to a mess.
     */
    if ( P_.scientific_ )
    {
      B_.fs_ << std::scientific;
    }
    else
    {
      B_.fs_ << std::fixed;
    }

    B_.fs_ << std::setprecision( P_.precision_ );
  }
}

void
nest::RecordingDevice::post_run_cleanup()
{
  if ( B_.fs_.is_open() )
  {
    if ( P_.flush_after_simulate_ )
    {
      B_.fs_.flush();
    }

    if ( not B_.fs_.good() )
    {
      std::string msg =
        String::compose( "I/O error while opening file '%1'", P_.filename_ );
      LOG( M_ERROR, "RecordingDevice::post_run_cleanup()", msg );

      throw IOError();
    }
  }
}

void
nest::RecordingDevice::finalize()
{
  if ( B_.fs_.is_open() )
  {
    if ( P_.close_after_simulate_ )
    {
      B_.fs_.close();
      return;
    }

    if ( P_.flush_after_simulate_ )
    {
      B_.fs_.flush();
    }
    if ( not B_.fs_.good() )
    {
      std::string msg =
        String::compose( "I/O error while opening file '%1'", P_.filename_ );
      LOG( M_ERROR, "RecordingDevice::finalize()", msg );

      throw IOError();
    }
  }
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;    // temporary copy in case of errors
  ptmp.set( *this, B_, d ); // throws if BadProperty
  State_ stmp = S_;
  stmp.set( d );

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Device::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

  if ( not P_.to_file_ && B_.fs_.is_open() )
  {
    B_.fs_.close();
    P_.filename_.clear();
  }

  if ( S_.events_ == 0 )
  {
    S_.clear_events();
  }
}


void
nest::RecordingDevice::record_event( const Event& event, bool endrecord )
{
  ++S_.events_;
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();
  const double weight = event.get_weight();
  const long port = event.get_port();
  const long rport = event.get_rport();

  index target = -1;
  if ( P_.withtargetgid_ )
  {
    const WeightRecorderEvent* wr_e =
      dynamic_cast< const WeightRecorderEvent* >( &event );
    if ( wr_e != 0 )
    {
      target = wr_e->get_receiver_gid();
    }
    else
    {
      target = event.get_receiver_gid();
    }
  }

  if ( P_.to_screen_ )
  {
    print_id_( std::cout, sender );
    print_target_( std::cout, target );
    print_port_( std::cout, port );
    print_rport_( std::cout, rport );
    print_time_( std::cout, stamp, offset );
    print_weight_( std::cout, weight );
    if ( endrecord )
    {
      std::cout << '\n';
    }
  }

  if ( P_.to_file_ )
  {
    print_id_( B_.fs_, sender );
    print_target_( B_.fs_, target );
    print_port_( B_.fs_, port );
    print_rport_( B_.fs_, rport );
    print_time_( B_.fs_, stamp, offset );
    print_weight_( B_.fs_, weight );
    if ( endrecord )
    {
      B_.fs_ << '\n';
      if ( P_.flush_records_ )
      {
        B_.fs_.flush();
      }
    }
  }

  // storing data when recording to accumulator relies on the fact
  // that multimeter will call us only once per accumulation step
  if ( P_.to_memory_ || P_.to_accumulator_ )
  {
    store_data_( sender, stamp, offset, weight, target, port, rport );
  }
}

void
nest::RecordingDevice::print_id_( std::ostream& os, index gid )
{
  if ( P_.withgid_ )
  {
    os << gid << '\t';
  }
}

void
nest::RecordingDevice::print_time_( std::ostream& os,
  const Time& t,
  double offs )
{
  if ( not P_.withtime_ )
  {
    return;
  }
  if ( P_.time_in_steps_ )
  {
    os << t.get_steps() << '\t';
    if ( P_.precise_times_ )
    {
      os << offs << '\t';
    }
  }
  else if ( P_.precise_times_ )
  {
    os << t.get_ms() - offs << '\t';
  }
  else
  {
    os << t.get_ms() << '\t';
  }
}

void
nest::RecordingDevice::print_weight_( std::ostream& os, double weight )
{
  if ( P_.withweight_ )
  {
    os << weight << '\t';
  }
}

void
nest::RecordingDevice::print_target_( std::ostream& os, index gid )
{
  if ( P_.withtargetgid_ )
  {
    os << gid << '\t';
  }
}

void
nest::RecordingDevice::print_port_( std::ostream& os, long port )
{
  if ( P_.withport_ )
  {
    os << port << '\t';
  }
}

void
nest::RecordingDevice::print_rport_( std::ostream& os, long rport )
{
  if ( P_.withrport_ )
  {
    os << rport << '\t';
  }
}

void
nest::RecordingDevice::store_data_( index sender,
  const Time& t,
  double offs,
  double weight,
  index target,
  long port,
  long rport )
{
  if ( P_.withgid_ )
  {
    S_.event_senders_.push_back( sender );
  }
  if ( P_.withtime_ )
  {
    if ( P_.time_in_steps_ )
    {
      S_.event_times_steps_.push_back( t.get_steps() );
      if ( P_.precise_times_ )
      {
        S_.event_times_offsets_.push_back( offs );
      }
    }
    else if ( P_.precise_times_ )
    {
      S_.event_times_ms_.push_back( t.get_ms() - offs );
    }
    else
    {
      S_.event_times_ms_.push_back( t.get_ms() );
    }
  }

  if ( P_.withweight_ )
  {
    S_.event_weights_.push_back( weight );
  }
  if ( P_.withtargetgid_ )
  {
    S_.event_targets_.push_back( target );
  }
  if ( P_.withport_ )
  {
    S_.event_ports_.push_back( port );
  }
  if ( P_.withrport_ )
  {
    S_.event_rports_.push_back( rport );
  }
}


const std::string
nest::RecordingDevice::build_filename_() const
{
  // number of digits in number of virtual processes
  const int vpdigits = static_cast< int >(
    std::floor( std::log10( static_cast< float >(
      kernel().vp_manager.get_num_virtual_processes() ) ) ) + 1 );
  const int gidigits = static_cast< int >(
    std::floor( std::log10(
      static_cast< float >( kernel().node_manager.size() ) ) ) + 1 );

  std::ostringstream basename;
  const std::string& path = kernel().io_manager.get_data_path();
  if ( not path.empty() )
  {
    basename << path << '/';
  }
  basename << kernel().io_manager.get_data_prefix();


  if ( not P_.label_.empty() )
  {
    basename << P_.label_;
  }
  else
  {
    basename << node_.get_name();
  }

  if ( not P_.use_gid_in_filename_ and not P_.label_.empty() )
  {
    basename << "-" << std::setfill( '0' ) << std::setw( vpdigits )
             << node_.get_vp();
  }
  else
  {
    basename << "-" << std::setfill( '0' ) << std::setw( gidigits )
             << node_.get_gid() << "-" << std::setfill( '0' )
             << std::setw( vpdigits ) << node_.get_vp();
  }
  return basename.str() + '.' + P_.file_ext_;
}

void
nest::RecordingDevice::State_::clear_events()
{
  events_ = 0;
  event_senders_.clear();
  event_times_ms_.clear();
  event_times_steps_.clear();
  event_times_offsets_.clear();
  event_weights_.clear();
  event_targets_.clear();
  event_ports_.clear();
  event_rports_.clear();
}
