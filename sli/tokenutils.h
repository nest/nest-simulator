/*
 *  tokenutils.h
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

#ifndef TOKENUTILS_H
#define TOKENUTILS_H

// C++ includes:
#include <string>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "namedatum.h"
#include "sliexceptions.h"
#include "token.h"

/**
 * @defgroup TokenHandling Handling classes Token and Dictionary.
 *
 * Accessing the underlying vales of class Token and Dictionary
 * entries can be a somewhat tricky issue, depending on how the data
 * type is actually implemented. The programmer needs detailed
 * knowledge of the implementation (which usually involves an
 * intermediate class which is derived from the generic class Datum.)
 * However, the programmer in almost all cases is only interested in
 * how to get and modify the underlying fundamental C++-types.
 *
 * The utility functions described in this group aim at simplifying
 * the access to the underlying values by providing template
 * specializations for each fundamental C++-type. The programmer can
 * simply specify the fundamental C++-type to handle, while the
 * implementation details are hidden.
 *
 * @note
 * Some of the utility functions described here have since been superceded
 * by new type conversion operators in class Token (see there). These
 * operators allow to use class Token directly at positions, where a
 * fundamental C++-datatype is required. Together with the indexing
 * operator for class Dictionary, it all boils down to
 * comprehensive calls like the following:
 * @code
 * int myvar = dict["myentry"];
 * dict["entry2"] = 23;
 * Token1 = Token2 + 1
 * @endcode
 * @note
 * It is left to the programmer's choice what method to use. It is
 * recommendable to use the (implicit) type conversion operators where
 * their operation is obvious to the reader of the source code.
 * Sometimes using the type conversion operators tends to obscure the
 * meaning of code. In these cases, the programmer is kindly asked to
 * comment on the code, or to use the more explicit template functions
 * described in this group.
 * @note
 * R. Kupper, 24-09-2003
 */

/**
 * @defgroup TokenUtils How to access the value contained in a Token.
 * @ingroup TokenHandling
 *
 * Class Token defines the standard user interface for accessing SLI
 * Datum types from tokens (see there). However, this user interface returns
 * objects of class Datum, from which the actual value would still need to be
 * extracted. The utilitiy functions described in this group shortcut
 * this step and provide direct access to the underlying fundamental
 * values contained in a token.
 */

/** getValue provides easy read-access to a Token's contents.
    getValue returns the value of the Datum contained inside the Token.

    \ingroup TokenUtils

    The general getValue function assumes that the datum was directly
    derived from a C++ type (most probably some container or stream).
    All other cases will be handled by specialization of this template.

    For example, AggregateDatum types are directly derived from a C++ type.

    \verbatim
           SLI Datum          derived from C++ type
          ------------------------------------------
           BoolDatum          Name
           HandleDatum        DatumHandle (whatever that might be...)
           LiteralDatum       Name
           NameDatum          Name
           ParserDatum        Parser
           StringDatum        string
           SymbolDatum        Name

           (What else?)
    \endverbatim

    Hence, getValue may be used in the following ways:

    \verbatim
        call                      can be used on Token containing SLI-type
       --------------------------------------------------------------------
        Name   getValue<Name>     {Bool|Literal|Name|Symbol}Datum
        DatumHandle
           getValue<DatumHandle>  HandleDatum
        Parser getValue<Parser>   ParserDatum
        string getValue<string>   StringDatum
    \endverbatim

    The following specialized variants of getValue() can be used in addition:

    \verbatim
        call                            can be used on Token containing SLI-type
       -------------------------------------------------------------------------
        long             GetValue<long>              IntegerDatum
        double           GetValue<double>            DoubleDatum
        bool             GetValue<bool>              BoolDatum
        string           GetValue<string>            NameDatum
        string           GetValue<string>            LiteralDatum
        string           GetValue<string>            SymbolDatum
        vector<long>     GetValue<vector<long> >     ArrayDatum
        vector<double>   GetValue<vector<double> >   ArrayDatum
    \endverbatim

    What about the rest? (ElementFactoryDatum, el_prtdatum, FunctionDatum,
    GenericDatum, NumericDatum, lockPTRDatum, ReferenceDatum, SmartPtrDatum,
    TrieDatum)

    @throws TypeMismatch The specified fundamental datatype does not
    match the Token's contents, or a template specialization for this
    type is missing.
**/
template < typename FT >
FT
getValue( const Token& t )
{
  FT* value = dynamic_cast< FT* >( t.datum() );
  if ( value == NULL )
  {
    throw TypeMismatch();
  }
  return *value;
}

/** setValue provides easy write-access to a Token's contents.
    setValue updates the value of the Datum contained inside the Token.

    \ingroup TokenUtils

    setValue(Token, value) can be called on the same value/Datum pairs getValue
    handles. Note that the template parameter to setValue does not need to be
    specified explicitely, as it can be derived from the second argument.

    @throws TypeMismatch The specified fundamental datatype does not
    match the Token's contents, or a template specialization for this
    type is missing.
**/
template < typename FT >
void
setValue( const Token& t, FT const& value )
{
  FT* old = dynamic_cast< FT* >( t.datum() );
  if ( old == NULL )
  {
    throw TypeMismatch();
  }

  *old = value;
}

/** Create a new Token from a fundamental data type.
    Specify the desired Datum type as the second template parameter!
    @ingroup TokenUtils
**/
template < typename FT, class D >
Token
newToken2( FT const& value )
{
  Token t( new D( value ) );

  // this is for typechecking reasons:
  getValue< FT >( t );

  return t;
}


/** Create a new Token from a fundamental data type.
    This template is specialized for the most fundamental types. If it
    does not work, use newToken2() and specify the Datum type explicitely.
    @ingroup TokenUtils
**/
template < typename FT >
Token
newToken( FT const& value )
{
  return newToken2< FT, NameDatum >( value );
}

// specializations below this line: -----------------------
template <>
long getValue< long >( const Token& );
template <>
void setValue< long >( const Token&, long const& value );

template <>
Token newToken< long >( long const& value );

template <>
double getValue< double >( const Token& );

template <>
void setValue< double >( const Token&, double const& value );

template <>
float getValue< float >( const Token& );

template <>
void setValue< float >( const Token&, float const& value );


template <>
Token newToken< double >( double const& value );

template <>
bool getValue< bool >( const Token& );
template <>
void setValue< bool >( const Token&, bool const& value );

template <>
Token newToken< bool >( bool const& value );


// These will handle StringDatum, NameDatum,
// LiteralDatum and SymbolDatum tokens:
template <>
std::string getValue< std::string >( const Token& );
template <>
void setValue< std::string >( const Token&, std::string const& value );


template <>
Token newToken< std::string >( std::string const& value );


// To get NameDatum, LiteralDatum or SymbolDatum tokens from a string,
// use newToken2<FT,D> instead (e.g. newToken2<string,LiteralDatum>);


// These will convert homogeneous int arrays to vectors:
template <>
std::vector< long > getValue< std::vector< long > >( const Token& );
template <>
void setValue< std::vector< long > >( const Token&, std::vector< long > const& value );

template <>
Token newToken< std::vector< long > >( std::vector< long > const& value );


// These will convert homogeneous double arrays to vectors:
template <>
std::vector< double > getValue< std::vector< double > >( const Token& );
template <>
void setValue< std::vector< double > >( const Token&, std::vector< double > const& value );

template <>
Token newToken< std::vector< double > >( std::vector< double > const& value );

#endif
