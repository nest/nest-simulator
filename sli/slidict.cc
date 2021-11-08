/*
 *  slidict.cc
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

/*
    SLI Dictionary access
*/

#include "slidict.h"

// C++ includes:
#include <typeinfo>

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dictdatum.h"
#include "dictstack.h"
#include "integerdatum.h"
#include "iostreamdatum.h"
#include "namedatum.h"
#include "tokenutils.h"

/** @BeginDocumentation
   Name: dict - Create new, empty dictionary

   Synopsis: dict -> <<>>

   Description:
    dict creates a new dictionary with no entries. This command is
    equivalent to
    <<>>

   Diagnostics: No errors are raised.

   Author: Marc-Oliver Gewaltig

   SeeAlso: <<>>, clonedict, cleardict

*/

void
DictFunction::execute( SLIInterpreter* i ) const
{
  // create a new dictionary

  i->EStack.pop(); // never forget me
  i->OStack.push( DictionaryDatum( new Dictionary ) );
}

/** @BeginDocumentation
   Name: put_d - Add an entry to a dictionary

   Synopsis: <<dict>> /key val -> <<dict>>

   Description:
     put_d adds the pair /key value to the supplied dictionary.
     If /key was already defined in that dictionary, its old
     value is lost.

     In contrast to PostScript dictionaries, only literal names can
     be used as keys in a dictionary.

   Parameters:
     dict - a dictionary
     /key - a literal name
     val  - an object of arbitrary type

   Examples:

     userdict /a 1 put_d
     << >> /a 1 put_d

   Diagnostics:
     No errors are raised.

   Author:
     Marc-Oliver Gewaltig

   SeeAlso: get_d, known, dict, <<>>, clonedict

*/
void
DictputFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() >= 3 )
  {
    //  call: dict key val
    DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.pick( 2 ).datum() );
    if ( dict != 0 )
    {
      LiteralDatum* key = dynamic_cast< LiteralDatum* >( i->OStack.pick( 1 ).datum() );
      if ( key != 0 )
      {
        ( *dict )->insert_move( *key, i->OStack.top() );
#ifdef DICTSTACK_CACHE
        if ( ( *dict )->is_on_dictstack() )
        {
          i->DStack->clear_token_from_cache( *key );
        }
#endif
        i->OStack.pop( 3 );
        i->EStack.pop(); // never forget me
        return;
      }
      else
      {
        throw ArgumentType( 1 );
      }
    }
    else
    {
      throw ArgumentType( 2 );
    }
  }
  else
  {
    throw StackUnderflow( 3, i->OStack.load() );
  }
}

/** @BeginDocumentation
   Name: get_d - look a name up in a dictionary

   Synopsis: dict /key get_d -> any

   Description:
     get_d tries to find /key in the supplied dictionary. If it
     is present, the associated value is returned. If /key is
     not in the dictionary, an UndefinedNameError is raised

   Parameters:
      dict - a dictionary
      /key - a literal name

   Examples:

      systemdict /PI get_d -> 3.1415...
      <</a 1>> /a get -> 1

   Diagnostics:
     UndefinedNameError

   Author:
     Marc-Oliver Gewaltig

   SeeAlso: put_d, lookup, known

*/
void
DictgetFunction::execute( SLIInterpreter* i ) const
{
  //  call: dict key -> val
  if ( i->OStack.load() >= 2 )
  {
    DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.pick( 1 ).datum() );
    if ( dict != 0 )
    {
      LiteralDatum* key = dynamic_cast< LiteralDatum* >( i->OStack.pick( 0 ).datum() );
      if ( key != 0 )
      {
        Token value = ( *dict )->lookup2( *key );
        i->EStack.pop(); // never forget me
        i->OStack.pop( 2 );
        i->OStack.push_move( value );
        return;
      }
      else
      {
        throw ArgumentType( 0 );
      }
    }
    else
    {
      throw ArgumentType( 1 );
    }
  }
  else
  {
    throw StackUnderflow( 2, i->OStack.load() );
  }
}
/** @BeginDocumentation
   Name: info - Display the contents of a dictionary

   Synopsis: ostream dict info -> -

   Description:
     Prints the contents of the dictionary to the supplied stream.

   Parameters:
     dict - a dictionary
     No return values are given.

   Examples:
   std::cout  <</a 1 /b 2.5 /c (Hallo World)>> info

   shows

--------------------------------------------------
Name                     Type                Value
--------------------------------------------------
a                        integertype         1
b                        doubletype          2.5
c                        stringtype          Hallo World
--------------------------------------------------
Total number of dictionary entries: 3

   SeeAlso: info_ds, topinfo_d, who, whos

*/
void
DictinfoFunction::execute( SLIInterpreter* i ) const
{
  //  call: ostream dict

  assert( i->OStack.load() > 1 );
  OstreamDatum* outd = dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );
  assert( outd != NULL );
  i->EStack.pop();
  ( *dict )->info( **outd );
  i->OStack.pop( 2 );
}

/** @BeginDocumentation
 Name: length_d - counts elements of a dictionary

 Synopsis: dict length_d -> int

 Examples: <</a 1 /b 2>> length_d -> 2
   modeldict length_d --> 34

 Author: docu by Sirko Straube

 Remarks: Use length if you are not sure of the data type.

 SeeAlso: length
*/

void
Length_dFunction::execute( SLIInterpreter* i ) const
{
  //  call: dict length_d -> int

  assert( i->OStack.load() > 0 );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );
  i->EStack.pop();
  Token st( new IntegerDatum( ( *dict )->size() ) );
  i->OStack.pop();
  i->OStack.push_move( st );
}

void
Empty_DFunction::execute( SLIInterpreter* i ) const
{
  // call: dict empty_D dict bool
  assert( i->OStack.load() > 0 );

  DictionaryDatum const* const dd = dynamic_cast< DictionaryDatum const* const >( i->OStack.top().datum() );

  assert( dd != NULL );

  i->OStack.push_by_pointer( new BoolDatum( ( *dd )->empty() ) );

  i->EStack.pop();
}


/** @BeginDocumentation
   Name: countdictstack - return number of dictionaries on the dictionary stack.

   Synopsis:
     No arguments.

   Description:
     Returns the current load of the dictionary stack. The default
     value is 2.

   Parameters:
     None.

   SeeAlso: dictstack, cleardictstack, whos

*/
void
CountdictstackFunction::execute( SLIInterpreter* i ) const
{
  //  call: Countdictstack -> int

  i->EStack.pop();
  Token st( new IntegerDatum( i->DStack->size() ) );
  i->OStack.push_move( st );
}

/** @BeginDocumentation
   Name: dictstack - return current dictionary stack as array

   Synopsis: dictstack -> array

   Description: Returns an array whose entries are references to the
     dictionaries on the dictionary stack. The dictionaries are stored from
     bottom to top, such that the first array element refers to the bottom of
     the stack and the last array element to the top.

   SeeAlso: currentdict, countdictstack, cleardictstack, whos
*/
void
DictstackFunction::execute( SLIInterpreter* i ) const
{
  //  call: -  dictstack ->  array

  i->EStack.pop();
  TokenArray ta;
  i->DStack->toArray( ta );
  Token st( new ArrayDatum( ta ) );
  i->OStack.push_move( st );
}

/** @BeginDocumentation
   Name: currentdict - return topmost dictionary of the dictionary stack

   Synopsis: currentdict -> dict

   Description: Returns a reference to the current dictionary.

   SeeAlso: dictstack, begin, end, cleardictstack, who, whos
*/
void
CurrentdictFunction::execute( SLIInterpreter* i ) const
{
  //  call: -  currentdict -> dict

  i->EStack.pop();
  Token dt;
  i->DStack->top( dt );
  i->OStack.push_move( dt );
}

/** @BeginDocumentation
   Name: cleardictstack - Pop all non standard dictionaries off the dictionary
                          stack.

   Description: Removes all non standard dictionaries off the dictionary stack.
     After this, only the systemdict and the userdict dictionaries remain on the
     dictionary stack.

   SeeAlso: dictstack, begin, end, currentdict, who, whos
*/
void
CleardictstackFunction::execute( SLIInterpreter* i ) const
{
  // Pop all non-permanent dictionaries
  i->EStack.pop();
  while ( i->DStack->size() > 2 )
  {
    i->DStack->pop();
  }
}

/** @BeginDocumentation
   Name: topinfo_d - print contents of top dictionary to stream

   Synopsis: ostream topinfo_d -> -

   Description: Print the contents of the top dictionary on the
     dictionary stack to the supplied stream.

   Parameters:
     ostream - a valid output stream

   SeeAlso: dictstack, currentdict, info, who, whos
*/
void
DicttopinfoFunction::execute( SLIInterpreter* i ) const
{
  //  call: ostream Dicttopinfo

  assert( i->OStack.load() > 0 );
  OstreamDatum* outd = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );
  assert( outd != NULL );
  i->EStack.pop();
  i->DStack->top_info( **outd );
  i->OStack.pop();
}

/** @BeginDocumentation
   Name: info_ds - print contents of all dictionaries on the dicitonary stack to
                   stream

   Synopsis: ostream info_ds -> -

   Description:
     info_ds prints the contents of all dictionaries on the dictionary
     stack to the supplied ostream. Dictionaries are printed from
     bottom to top.

   Parameters:
     ostream - a valid output stream

   SeeAlso:  dictstack, info, topinfo_d, who, whos
*/
void
WhoFunction::execute( SLIInterpreter* i ) const
{
  //  call: ostream

  assert( i->OStack.load() > 0 );
  OstreamDatum* outd = dynamic_cast< OstreamDatum* >( i->OStack.top().datum() );
  assert( outd != NULL );
  i->EStack.pop();
  i->DStack->info( **outd );
  i->OStack.pop();
}

/** @BeginDocumentation
   Name: begin - Make a dictionary the current dictionary.

   Synopsis: dict begin -> -

   Parameters:
     dict - a dictionary object

   Description:
     begin opens a dictionary by pushing it on the dictionary stack.
     After begin, the opened dictionary is searched first whenever
     a name is resolved.

   SeeAlso: end, get_d, dictstack, cleardictstack, countdictstack
*/
void
DictbeginFunction::execute( SLIInterpreter* i ) const
{
  //  call: dict

  if ( i->OStack.load() > 0 )
  {
    DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
    if ( dict != NULL )
    {
      i->EStack.pop();
      i->DStack->push( *dict );
      i->OStack.pop();
      return;
    }
    else
    {
      i->raiseerror( i->ArgumentTypeError );
    }
  }
  else
  {
    i->raiseerror( i->StackUnderflowError );
  }
}

/** @BeginDocumentation
   Name: end - Close the current (topmost) dictionary.

   Synopsis: - end -> -

   Description:
     Closes the current dictionary by removing the top element
     of the dictionary stack.
     Note that end cannot pop the dictionary stack beyond the two
     standard dictionaries systemdict and userdict. If this is tried,
     a DictStackUnderflow Error is raised.

   Diagnostics:
     DictStackUnderflow

   SeeAlso: begin, dictstack, cleardictstack, countdictstack
*/
void
DictendFunction::execute( SLIInterpreter* i ) const
{
  if ( i->DStack->size() > 2 ) // keep at least systemdict and userdict
  {                            // see sli-init.sli for details
    i->DStack->pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( "DictStackUnderflow" );
  }
}

/** @BeginDocumentation
   Name: undef - Remove a key from a dictionary.

   Synopsis: dict key undef -> -

   Parameters:
     dict - a (possibly empty) dictionary
     key  - a literal name to be removed.

   Description:
     undef removes the definition of a name from the supplied dictionary.
     The name does not have to be present in the dicitonary.

   Examples:
     SLI ] /d << /a 1 /b 2 >> def
     SLI ] d info
     - --------------------------------------------------
     Name                     Type                Value
     - --------------------------------------------------
     a                        integertype         1
     b                        integertype         2
     - --------------------------------------------------
     Total number of entries: 2
     SLI ] d /b undef
     SLI ] d info
     - --------------------------------------------------
     Name                     Type                Value
     - --------------------------------------------------
     a                        integertype         1
     - --------------------------------------------------
     Total number of entries: 1

     Diagnostics:
     None.

   Author: docu edited by Marc Oliver Gewaltig and Sirko Straube
   SeeAlso: get_d, put_d, known, lookup, info
*/
void
UndefFunction::execute( SLIInterpreter* i ) const
{
  //  call: dict key -> -

  if ( i->OStack.load() > 1 )
  {
    DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.pick( 1 ).datum() );
    if ( dict != NULL )
    {
      LiteralDatum* key = dynamic_cast< LiteralDatum* >( i->OStack.pick( 0 ).datum() );
      if ( key != NULL )
      {
        i->EStack.pop();
#ifdef DICTSTACK_CACHE
        if ( ( *dict )->is_on_dictstack() )
        {
          i->DStack->clear_token_from_cache( *key );
        }
#endif
        ( *dict )->erase( *key );
        i->OStack.pop( 2 );
        return;
      }
      else
      {
        throw ArgumentType( 0 );
      }
    }
    else
    {
      throw ArgumentType( 1 );
    }
  }
  else
  {
    throw StackUnderflow( 2, i->OStack.load() );
  }
}

/** @BeginDocumentation
   Name: <<>> - Create a new dictionary.

   Synopsis: << /key1 val1 ... /keyn valn >> -> dict

   Parameters: /key - literal name
               val  - Object of any type

   Description: Constructs a dictionary with the entries which are specified
    by key-value pairs.
    Note that the key-value pairs are evaluated BEFORE the dictionary is
    constructed.
    << >> operates the following way:
    The characters << correspond to a mark which is pushed on the stack.
    Next, all following key-value pairs are evaluated.
    >> finally counts the number of pairs on the stack and constructs a
    new dictionary.

   Examples:
<pre>
SLI ] << /a 1 /b 2 >> info
--------------------------------------------------
Name                     Type                Value
--------------------------------------------------
a                        integertype         1
b                        integertype         2
--------------------------------------------------
Total number of entries: 2

SLI ] << (a) (b) join cvlit 2 3 mul 2 add >> info
--------------------------------------------------
Name                     Type                Value
--------------------------------------------------
ab                       integertype         8
--------------------------------------------------
Total number of entries: 1
</pre>

   Diagnostics: An ArgumentType error is raised if the
   initializer list does not consist of proper /key value
   pairs.

   References: The Red Book
   SeeAlso: clonedict, begin, cleardictstack, dict, dictstack, info, end
*/
void
DictconstructFunction::execute( SLIInterpreter* i ) const
{
  // call: mark key1 val1 ... keyn valn -> dict

  size_t load = i->OStack.load();
  if ( load == 0 )
  {
    throw StackUnderflow( 1, 0 );
  }

  DictionaryDatum* dictd = new DictionaryDatum( new Dictionary );
  Token dict( dictd );

  LiteralDatum* key = NULL;
  static Token mark = i->baselookup( i->mark_name );

  size_t n = 0; //!< pick(1) is the first literal, then we count in steps of 2
  while ( ( n < load ) && not( i->OStack.pick( n ) == mark ) )
  {
    Token& val = ( i->OStack.pick( n ) );
    key = dynamic_cast< LiteralDatum* >( i->OStack.pick( n + 1 ).datum() );
    if ( key == NULL )
    {
      i->message( 30, "DictConstruct", "Literal expected. Maybe initializer list is in the wrong order." );
      i->raiseerror( i->ArgumentTypeError );
      delete dictd;
      return;
    }
    ( *dictd )->insert_move( *key, val );
    n += 2; // count number of elements
  }

  if ( n == load )
  {
    i->message( 30, "DictConstruct", "<< expected." );
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  if ( n % 2 != 0 ) // there must be an even number of objects
  {                 // above the mark
    i->message( 30, "DictConstruct", "Initializer list must be pairs of literal and value." );
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  i->EStack.pop();
  i->OStack.pop( n );
  i->OStack.top().move( dict );
}


/** @BeginDocumentation
   Name: known - check whether a name is defined in a dictionary or object

   Synopsis: dict /key known -> bool
             int  /key known -> bool

   Examples:
   modeldict /iaf_psc_alpha known -> true
   modeldict /parkinson_neuron known -> false
   /iaf_psc_alpha_ps GetDefaults /Interpol_Order known -> false

   Author: docu edited by Sirko Straube

   SeeAlso: lookup

*/
void
KnownFunction::execute( SLIInterpreter* i ) const
{
  //  call: dict key -> bool

  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.pick( 1 ).datum() );
  LiteralDatum* key = dynamic_cast< LiteralDatum* >( i->OStack.pick( 0 ).datum() );

  bool known = ( *dict )->known( *key );
  i->EStack.pop(); // never forget me
  i->OStack.pop( 1 );
  i->OStack.top() = new BoolDatum( known );
}

/** @BeginDocumentation
   Name: cleardict - Clears the contents of a dictionary

   Synopsis: dict cleardict

   SeeAlso: <<>>, clonedict, dictstack

*/
void
CleardictFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );
#ifdef DICTSTACK_CACHE
  if ( ( *dict )->is_on_dictstack() )
  {
    i->DStack->clear_dict_from_cache( *dict );
  }
#endif
  ( *dict )->clear();
  i->EStack.pop(); // never forget me
  i->OStack.pop();
}

/** @BeginDocumentation
   Name: clonedict - create a copy of a dictionary

   Synopsis: dict1 clonedict -> dict1 dict2

   Description:
     clonedict creates a new dictionary dict2 with the
     same entries as dict1.

   Parameters:
     dict1 - a dictionary

   Examples:
     << /a 1 >> clonedict

   Diagnostics:
     None.

   Author:
     Marc-Oliver Gewaltig

   SeeAlso: <<>>, dict, cleardict
*/
void
ClonedictFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );

  i->OStack.push( DictionaryDatum( new Dictionary( *( *dict ) ) ) );
  i->EStack.pop(); // never forget me
}

/** @BeginDocumentation
   Name: cva_d - Convert dictionary to array

   Synopsis: dict cva_d -> array

   Description: cda converts a given dictionary to an array.
   The contents of the dictionay is mapped to the array in a
   form which is similar to the construction of an array, namely
   << key1 val1 ... keyn valn>> cva_d -> [key1 val1 ... keyn valn]

   Parameters:
   dict is a dictionary which may be empty

   Examples:
   << /a 1 /b 2>> cva_d -> [/a 1 /b 2]

   Diagnostics:
   no errors are issued.

   Remarks:
   The name follows the convention of PostScript and stands for
   ConVert to Array.

  Implementation:

   Class TokenMap provides an iterator which allows to step through the
   elements stored in the TokenMap. Member function begin() returns an
   iterator which points to the "first" element in the TokenMap and end()
   to the "last" element. operator ++ moves the iterator from one element
   to the next. Note, the iteration process itself does not define any
   order of the elements, e.g. in terms of the comparison operators. The
   only guaranteed function is that during the iteration from begin() to
   end() all elements in the TokenMap are accessed.
   Operator * applied to the iterator returns a pair. Member first of the
   pair is the key and member second is the value of the element the iterator
   points to.
   See
        @Book{Musser96,
          author =       "Musser, David R. and Saini, Atul",
          title =        "STL Tutorial and Reference Guide: {C++} Programming
                          with the Standard Template Library",
          publisher =    "Addison Wesley",
          year =         1996,
          isbn =         "0-201-63398-1",
          pages =        432,
          address =      {Reading Massachusetts},
          note =         "(hardcover)"
        }
  for a discussion of the definition and use of iterators in combination
  with containers.

   Author:
   Marc-oliver Gewaltig

   SeeAlso: keys, values, <<>>, cva, cvdict

*/
void
Cva_dFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );
  ArrayDatum* ad = new ArrayDatum();
  ad->reserve( ( *dict )->size() * 2 );

  for ( TokenMap::const_iterator t = ( *dict )->begin(); t != ( *dict )->end(); ++t )
  {
    Token nt( new LiteralDatum( ( *t ).first ) );
    ad->push_back_move( nt );
    ad->push_back( ( *t ).second );
  }
  i->OStack.pop();
  i->OStack.push( ad );
}

/** @BeginDocumentation
   Name: keys - Return array of keys in a dictionary

   Synopsis:
                            dict keys -> array
   <</key1 val1 ... /keyn valn>> keys -> [/key1 ... /keyn]

   Description:
   "keys" converts the keys in a given dictionary to an array.
   The order of elements is the same as for the "values" command.

   Parameters:
   dict is a dictionary which may be empty

   Examples:
   << /a 1 /b 2>> keys -> [/a /b]

   Diagnostics:
   no errors are issued.

   Remarks:
   The commands "keys" and "values" return the same order of
   elements, when applied to the same dictionary. Apart from that,
   there is no particular order of elements in the returned array.

   Author: Ruediger Kupper

   First Version: 27-jun-2008

   SeeAlso: keys, values, <<>>, cva, cvdict
*/
void
KeysFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );
  ArrayDatum* ad = new ArrayDatum();

  for ( TokenMap::const_iterator t = ( *dict )->begin(); t != ( *dict )->end(); ++t )
  {
    Token nt( new LiteralDatum( ( *t ).first ) );
    assert( not nt.empty() );
    ad->push_back_move( nt );
  }
  i->OStack.pop();
  i->OStack.push( ad );
}

/** @BeginDocumentation
   Name: values - Return array of values in a dictionary

   Synopsis:
                            dict values -> array
   <</key1 val1 ... /keyn valn>> values -> [val1 ... valn]

   Description:
   "values" converts the values in a given dictionary to an array.
   The order of elements is the same as for the "keys" command.

   Parameters:
   dict is a dictionary which may be empty

   Examples:
   << /a 1 /b 2>> values -> [1 2]

   Diagnostics:
   no errors are issued.

   Remarks:
   The commands "keys" and "values" return the same order of
   elements, when applied to the same dictionary. Apart from that,
   there is no particular order of elements in the returned array.

   Author: Ruediger Kupper

   First Version: 27-jun-2008

   SeeAlso: keys, values, <<>>, cva, cvdict
*/
void
ValuesFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );
  DictionaryDatum* dict = dynamic_cast< DictionaryDatum* >( i->OStack.top().datum() );
  assert( dict != NULL );
  ArrayDatum* ad = new ArrayDatum();

  for ( TokenMap::const_iterator t = ( *dict )->begin(); t != ( *dict )->end(); ++t )
  {
    ad->push_back( ( *t ).second );
  }
  i->OStack.pop();
  i->OStack.push( ad );
}

void
RestoredstackFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( ad != NULL );
  TokenArray ta = *ad;
  DictionaryStack* olddstack = i->DStack; // copy current dstack
  i->DStack = new DictionaryStack;
  for ( size_t j = 0; j < ta.size(); ++j )
  {
    try
    {
      DictionaryDatum dict = getValue< DictionaryDatum >( ta[ j ] );
      i->DStack->push( ta[ j ] );
    }
    catch ( ... )
    {
      delete i->DStack;
      i->DStack = olddstack;
      i->raiseerror( i->ArgumentTypeError );
      return;
    }
  }
  i->OStack.pop();
  i->EStack.pop();
}

const DictFunction dictfunction;
const DictputFunction dictputfunction;
const DictgetFunction dictgetfunction;
const DictinfoFunction dictinfofunction;
const DicttopinfoFunction dicttopinfofunction;
const WhoFunction whofunction;
const DictbeginFunction dictbeginfunction;
const DictendFunction dictendfunction;
const DictconstructFunction dictconstructfunction;
const Length_dFunction length_dfunction;
const Empty_DFunction empty_Dfunction;
const DictstackFunction dictstackfunction;
const CountdictstackFunction countdictstackfunction;
const CleardictstackFunction cleardictstackfunction;
const CurrentdictFunction currentdictfunction;
const KnownFunction knownfunction;
const UndefFunction undeffunction;
const CleardictFunction cleardictfunction;
const ClonedictFunction clonedictfunction;
const Cva_dFunction cva_dfunction;
const KeysFunction keysfunction;
const ValuesFunction valuesfunction;
const RestoredstackFunction restoredstackfunction;

void
init_slidict( SLIInterpreter* i )
{
  i->createcommand( "dict", &dictfunction );
  i->createcommand( "put_d", &dictputfunction );
  i->createcommand( "get_d", &dictgetfunction );
  i->createcommand( "info_d", &dictinfofunction );
  i->createcommand( "length_d", &length_dfunction );
  i->createcommand( "empty_D", &empty_Dfunction );
  i->createcommand( "topinfo_d", &dicttopinfofunction );
  i->createcommand( "info_ds", &whofunction );
  i->createcommand( "begin", &dictbeginfunction );
  i->createcommand( i->end_name, &dictendfunction );
  i->createcommand( "undef", &undeffunction );
  i->createcommand( ">>", &dictconstructfunction );
  i->createcommand( "dictstack", &dictstackfunction );
  i->createcommand( "countdictstack", &countdictstackfunction );
  i->createcommand( "cleardictstack", &cleardictstackfunction );
  i->createcommand( "currentdict", &currentdictfunction );
  i->createcommand( "known", &knownfunction );
  i->createcommand( "cleardict", &cleardictfunction );
  i->createcommand( "clonedict", &clonedictfunction );
  i->createcommand( "cva_d", &cva_dfunction );
  i->createcommand( "keys", &keysfunction );
  i->createcommand( "values", &valuesfunction );
  i->createcommand( "restoredstack", &restoredstackfunction );
}
