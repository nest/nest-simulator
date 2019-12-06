/*
 *  sliregexp.cc
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

#include "sliregexp.h"

// C includes:
#include <regex.h>

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "lockptrdatum_impl.h"
#include "stringdatum.h"


SLIType RegexpModule::RegexType;

template class lockPTRDatum< Regex, &RegexpModule::RegexType >;

typedef lockPTRDatum< Regex, &RegexpModule::RegexType > RegexDatum;

Regex::Regex()
{
}

Regex::~Regex()
{
  regfree( &r );
}

regex_t*
Regex::get( void )
{
  return &r;
}


RegexpModule::~RegexpModule()
{
  RegexType.deletetypename();
}

void
RegexpModule::init( SLIInterpreter* i )
{
  Dictionary* regexdict = new Dictionary;

  regexdict->insert( REG_NOTBOL_name, new IntegerDatum( REG_NOTBOL ) );
  regexdict->insert( REG_NOTEOL_name, new IntegerDatum( REG_NOTEOL ) );
  regexdict->insert( REG_ESPACE_name, new IntegerDatum( REG_ESPACE ) );
  regexdict->insert( REG_BADPAT_name, new IntegerDatum( REG_BADPAT ) );
  regexdict->insert( REG_EXTENDED_name, new IntegerDatum( REG_EXTENDED ) );
  regexdict->insert( REG_ICASE_name, new IntegerDatum( REG_ICASE ) );
  regexdict->insert( REG_NOSUB_name, new IntegerDatum( REG_NOSUB ) );
  regexdict->insert( REG_NEWLINE_name, new IntegerDatum( REG_NEWLINE ) );
  regexdict->insert( REG_ECOLLATE_name, new IntegerDatum( REG_ECOLLATE ) );
  regexdict->insert( REG_ECTYPE_name, new IntegerDatum( REG_ECTYPE ) );
  regexdict->insert( REG_EESCAPE_name, new IntegerDatum( REG_EESCAPE ) );
  regexdict->insert( REG_ESUBREG_name, new IntegerDatum( REG_ESUBREG ) );
  regexdict->insert( REG_EBRACK_name, new IntegerDatum( REG_EBRACK ) );
  regexdict->insert( REG_EPAREN_name, new IntegerDatum( REG_EPAREN ) );
  regexdict->insert( REG_EBRACE_name, new IntegerDatum( REG_EBRACE ) );
  regexdict->insert( REG_BADBR_name, new IntegerDatum( REG_BADBR ) );
  regexdict->insert( REG_ERANGE_name, new IntegerDatum( REG_ERANGE ) );
  regexdict->insert( REG_BADRPT_name, new IntegerDatum( REG_BADRPT ) );

  i->def( regexdict_name, new DictionaryDatum( regexdict ) );

  RegexType.settypename( "regextype" );
  RegexType.setdefaultaction( SLIInterpreter::datatypefunction );

  i->createcommand( "regcomp_", &regcompfunction );
  i->createcommand( "regexec_", &regexecfunction );
  i->createcommand( "regerror_", &regerrorfunction );
}


const std::string
RegexpModule::name( void ) const
{
  return std::string( "POSIX-Regexp" );
}

const std::string
RegexpModule::commandstring( void ) const
{
  return std::string( "(regexp) run" );
}


void
RegexpModule::RegcompFunction::execute( SLIInterpreter* i ) const
{
  // string integer regcomp -> Regex true
  //                           Regex integer false
  assert( i->OStack.load() >= 2 );

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );

  assert( sd != NULL );
  assert( id != NULL );


  Regex* MyRegex = new Regex;
  int e = regcomp( MyRegex->get(), sd->c_str(), id->get() );
  i->OStack.pop( 2 );
  Token rt( new RegexDatum( MyRegex ) );
  i->OStack.push_move( rt );
  if ( not e )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    Token it( new IntegerDatum( e ) );
    i->OStack.push_move( it );
    i->OStack.push( i->baselookup( i->false_name ) );
  };
  i->EStack.pop();
}

void
RegexpModule::RegerrorFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  RegexDatum* rd = dynamic_cast< RegexDatum* >( i->OStack.pick( 1 ).datum() );

  assert( rd != NULL );
  assert( id != NULL );

  char* error_buffer = new char[ 256 ];
  regerror( id->get(), rd->get()->get(), error_buffer, 256 );
  Token sd( new StringDatum( error_buffer ) );
  delete[]( error_buffer );

  // The lockPTR rd gets locked when calling the get() function.
  // In order to be able to use (delete) this object we need to
  // unlock it again.
  rd->unlock();

  i->OStack.pop( 2 );
  i->OStack.push_move( sd );
  i->EStack.pop();
}

void
RegexpModule::RegexecFunction::execute( SLIInterpreter* i ) const
{
  // regex string integer integer regexec -> array integer
  // regex string 0       integer regexec -> integer

  assert( i->OStack.load() >= 4 );

  RegexDatum* rd = dynamic_cast< RegexDatum* >( i->OStack.pick( 3 ).datum() );
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* sized = dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* eflagsd = dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( rd != NULL );
  assert( sd != NULL );
  assert( sized != NULL );
  assert( eflagsd != NULL );

  int size = sized->get();
  regmatch_t* pm = new regmatch_t[ size ];

  Regex* r = rd->get();
  assert( r != NULL );
  rd->unlock();

  int e = regexec( r->get(), sd->c_str(), size, pm, eflagsd->get() );
  Token id( new IntegerDatum( e ) );
  i->OStack.pop( 4 );
  if ( size )
  {
    ArrayDatum* PushArray = new ArrayDatum();
    for ( int k = 0; k <= ( size - 1 ); k++ )
    {
      ArrayDatum* ThisEntry = new ArrayDatum();
      Token so( new IntegerDatum( pm[ k ].rm_so ) );
      ThisEntry->push_back_move( so );
      Token eo( new IntegerDatum( pm[ k ].rm_eo ) );
      ThisEntry->push_back_move( eo );
      Token entry_token( ThisEntry );
      PushArray->push_back_move( entry_token );
    };
    Token array_token( PushArray );
    i->OStack.push_move( array_token );
  };
  delete[]( pm );
  i->OStack.push_move( id );
  i->EStack.pop();
}
