/*
 *  datumconverter.h
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

#ifndef DATUMCONVERTER_H
#define DATUMCONVERTER_H
 
// These class definitions are better forward declarations
// because otherwise the interpreter will know about nest
// destroying the reciprocal isolation and causing problems
// on the PETA fx system. 2010-09-28.
// These forward declarations are only used, if the
// respective header file was not included before.
// In the implementation file of a specific type,
// say arraydatum.cc, arraydatum.h must be included before
// aggregratedatum_impl.h

#ifndef DATUM_H
class Datum;
#endif

#ifndef DOUBLEDATUM_H
class DoubleDatum;
#endif

#ifndef INTEGERDATUM_H
class IntegerDatum;
#endif

#ifndef BOOLDATUM_H
class BoolDatum;
#endif

#ifndef STRINGDATUM_H
class StringDatum;
#endif

#ifndef ARRAYDATUM_H
class ArrayDatum;
class IntVectorDatum;
class DoubleVectorDatum;
#endif

#ifndef DICTDATUM_H
class DictionaryDatum;
#endif

#ifndef LITERALDATUM_H
class LiteralDatum;
#endif

#ifndef CONNECTIONDATUM_H
class ConnectionDatum;
#endif


/**
 * This is the base class of a DatumConverter.
 *
 * It may be used to create a type conversion from a SLI Datum to some
 * arbitrary type not known to NEST. A concrete DatumConverter has to
 * override the convert_me method. This base class' implementation of
 * convert_me will issue an error. This may be used to indicate, that
 * a type conversion has failed. It follows the visitor design
 * pattern.
 */

class DatumConverter {

 public:

  virtual ~DatumConverter() {};

  /**
   * Base class' implementation of convert_me will produce an
   * exception, since we cannot visit an abstract Datum object.
   * Each derived class which is to be visited by a datum of a
   * derived type must override the respective method.
   */
  virtual void convert_me(Datum &)=0;
  virtual void convert_me(DoubleDatum &d)=0;
  virtual void convert_me(IntegerDatum &i)=0;
  virtual void convert_me(BoolDatum &i)=0;
  virtual void convert_me(StringDatum &s)=0;
  virtual void convert_me(DoubleVectorDatum &dvd)=0;
  virtual void convert_me(IntVectorDatum &dvd)=0;
  virtual void convert_me(ArrayDatum &ad)=0; 
  virtual void convert_me(DictionaryDatum &dd)=0;
  virtual void convert_me(LiteralDatum &ld)=0;
  virtual void convert_me(ConnectionDatum &cd)=0;

};

#endif
