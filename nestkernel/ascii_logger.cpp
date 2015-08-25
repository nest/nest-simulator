#include "recording_device.h"
#include "ascii_logger.h"

void
nest::ASCIILogger::enroll( const int virtual_process, RecordingDevice& device )
{
  const Node& node = device.get_node();
  const int gid = node.get_gid();

  // is virtual_process == virtual process we are in?
  // FIXME: critical?

  if ( files_.find( virtual_process ) == files_.end() )
  {
    files_.insert( std::make_pair( virtual_process, file_map::mapped_type() ) );
  }

  if ( files_[ virtual_process ].find( gid ) == files_[ virtual_process ].end() )
  {
    files_[ virtual_process ].insert(
      std::make_pair( gid, std::make_pair( &device, new std::ofstream() ) ) );
  }
}

void
nest::ASCIILogger::initialize()
{
#pragma omp parallel
  {
    Network& network = *( Node::network() );
    thread t = network.get_thread_id();
    int vp = network.thread_to_vp( t );

    // extract the inner map (containing the registered devices) for the specific VP
    typedef typename file_map::mapped_type inner_map;
    inner_map inner = files_[vp];

    // iterate over registed devices and their corresponding fstreams
    for ( inner_map::iterator jj = inner.begin(); jj != inner.end(); ++jj )
    {
      int gid = jj->first;
      std::ofstream& file = *( jj->second.second );
      RecordingDevice& device = *( jj->second.first );

      // initialize file according to parameters
      std::string filename;

      // do we need to (re-)open the file
      bool newfile = false;

      if ( !file.is_open() )
      {
        newfile = true; // no file from before
        filename = build_filename_( device );
        device.set_filename( filename );
      }

      else
      {
        std::string newname = build_filename_( device );
        if ( newname != device.get_filename() )
        {
          std::string msg =
            String::compose( "Closing file '%1', opening file '%2'", device.get_filename(), newname );
          Node::network()->message( SLIInterpreter::M_INFO, "RecordingDevice::calibrate()", msg );

          file.close(); // close old file
          device.set_filename( newname );
          newfile = true;
        }
      }

      if ( newfile )
      {
        assert( !file.is_open() );

        if ( Node::network()->overwrite_files() )
        {
          file.open( filename.c_str() );
        }
        else
        {
          // try opening for reading
          std::ifstream test( filename.c_str() );
          if ( test.good() )
          {
            std::string msg = String::compose(
              "The device file '%1' exists already and will not be overwritten. "
              "Please change data_path, data_prefix or label, or set /overwrite_files "
              "to true in the root node.",
              filename );
            Node::network()->message(
              SLIInterpreter::M_ERROR, "RecordingDevice::calibrate()", msg );
            throw IOError();
          }
          else
            test.close();

          // file does not exist, so we can open
          file.open( filename.c_str() );
        }

        if ( P_.fbuffer_size_ != P_.fbuffer_size_old_ )
        {
          if ( P_.fbuffer_size_ == 0 )
            file.rdbuf()->pubsetbuf( 0, 0 );
          else
          {
            std::vector< char >* buffer = new std::vector< char >( P_.fbuffer_size_ );
            file.rdbuf()->pubsetbuf( reinterpret_cast< char* >( &buffer[ 0 ] ), P_.fbuffer_size_ );
          }

          P_.fbuffer_size_old_ = P_.fbuffer_size_;
        }
      }

      if ( !file.good() )
      {
        std::string msg = String::compose(
          "I/O error while opening file '%1'. "
          "This may be caused by too many open files in networks "
          "with many recording devices and threads.",
          filename );
        Node::network()->message( SLIInterpreter::M_ERROR, "RecordingDevice::calibrate()", msg );

        if ( file.is_open() )
          file.close();
        filename.clear();
        throw IOError();
      }

      /* Set formatting */
      file << std::fixed;
      file << std::setprecision( 3 );

      if ( P_.fbuffer_size_ != P_.fbuffer_size_old_ )
      {
        std::string msg = String::compose(
          "Cannot set file buffer size, as the file is already "
          "openeded with a buffer size of %1. Please close the "
          "file first.",
          P_.fbuffer_size_old_ );
        Node::network()->message( SLIInterpreter::M_ERROR, "RecordingDevice::calibrate()", msg );
        throw IOError();
      }
    }
  }
}

void
nest::ASCIILogger::finalize()
{
#pragma omp parallel
  {
    Network& network = *( Node::network() );
    thread t = network.get_thread_id();
    int vp = network.thread_to_vp( t );

    // extract the inner map (containing the registered devices) for the specific VP
    typedef typename file_map::mapped_type inner_map;
    inner_map inner = files_[ vp ];
    // iterate over registed devices and their corresponding fstreams
    for ( inner_map::iterator jj = inner.begin(); jj != inner.end(); ++jj )
    {
      int gid = jj->first;
      std::ofstream& file = *( jj->second.second );
      RecordingDevice& device = *( jj->second.first );

      if ( file.is_open() )
      {
        if ( P_.close_after_simulate_ )
        {
          file.close();
        }
        else
        {
          if ( P_.flush_after_simulate_ )
            file.flush();

		  // FIXME: can this ever happen / does the message make sense?
          if ( !file.good() )
          {
            std::string msg =
              String::compose( "I/O error while closing file '%1'", device.get_filename() );
            Node::network()->message( SLIInterpreter::M_ERROR, "RecordingDevice::finalize()", msg );

            throw IOError();
          }
        }
      }
    }
  }
}

void
nest::ASCIILogger::write( const RecordingDevice& device, const Event& event )
{
  const Node& node = device.get_node();
  int vp = node.get_vp();
  int id = node.get_gid();

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ vp ][ id ].second );
  file << sender << "\t" << stamp.get_ms() - offset << "\n";
}

void
nest::ASCIILogger::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double_t >& values )
{
  const Node& node = device.get_node();
  int vp = node.get_vp();
  int id = node.get_gid();

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ vp ][ id ].second );
  file << sender << "\t" << stamp.get_ms() - offset;

  for ( std::vector< double_t >::const_iterator val = values.begin(); val != values.end(); ++val )
  {
    file << "\t" << *val;
  }

  file << "\n";
}

const std::string
nest::ASCIILogger::build_filename_( const RecordingDevice& device ) const
{
  const Node& node = device.get_node();

  // number of digits in number of virtual processes
  const int vpdigits = static_cast< int >(
    std::floor( std::log10( static_cast< float >( Communicator::get_num_virtual_processes() ) ) )
    + 1 );
  const int gidigits = static_cast< int >(
    std::floor( std::log10( static_cast< float >( Node::network()->size() ) ) ) + 1 );

  std::ostringstream basename;
  const std::string& path = Node::network()->get_data_path();
  if ( !path.empty() )
    basename << path << '/';
  basename << Node::network()->get_data_prefix();

  const std::string& label = device.get_label();
  if ( !label.empty() )
    basename << label;
  else
    basename << node.get_name();

  int vp = node.get_vp();
  int gid = node.get_gid();

  basename << "-" << std::setfill( '0' ) << std::setw( gidigits ) << gid << "-"
           << std::setfill( '0' ) << std::setw( vpdigits ) << vp;

  return basename.str() + '.' + P_.file_ext_;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::ASCIILogger::Parameters_::Parameters_()
  : precision_( 3 )
  , file_ext_( "dat" )
  , fbuffer_size_( BUFSIZ ) // default buffer size as defined in <cstdio>
  , close_after_simulate_( false )
  , flush_after_simulate_( true )
{
}

void
nest::ASCIILogger::Parameters_::get( const ASCIILogger& al, DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::file_extension ] = file_ext_;
  ( *d )[ names::fbuffer_size ] = fbuffer_size_;
  ( *d )[ names::close_after_simulate ] = close_after_simulate_;
  ( *d )[ names::flush_after_simulate ] = flush_after_simulate_;
}

void
nest::ASCIILogger::Parameters_::set( const ASCIILogger& al, const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
  updateValue< std::string >( d, names::file_extension, file_ext_ );
  updateValue< bool >( d, names::close_after_simulate, close_after_simulate_ );
  updateValue< bool >( d, names::flush_after_simulate, flush_after_simulate_ );

  long fbuffer_size;
  if ( updateValue< long >( d, names::fbuffer_size, fbuffer_size ) )
  {
    if ( fbuffer_size < 0 )
      throw BadProperty( "/fbuffer_size must be <= 0" );
    else
    {
      fbuffer_size_old_ = fbuffer_size_;
      fbuffer_size_ = fbuffer_size;
    }
  }
}
