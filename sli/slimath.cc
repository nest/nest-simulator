/*
 *  slimath.cc
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
    slimath.cc
*/

#include "slimath.h"

// C++ includes:
#include <cmath>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "booldatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "namedatum.h"
#include "stringdatum.h"


void
IntegerFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  DoubleDatum* op = dynamic_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  if ( op != NULL )
  {
    Token res( new IntegerDatum( op->get() ) );
    i->OStack.top().swap( res );
  }
}

void
DoubleFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  IntegerDatum* op = dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  if ( op != NULL )
  {

    Token res( new DoubleDatum( op->get() ) );
    i->OStack.top().swap( res );
  }
}

void
Add_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() += ( op2->get() );
  i->OStack.pop();
}

void
Add_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() += ( op2->get() );
  i->OStack.pop();
}

void
Add_diFunction::execute( SLIInterpreter* i ) const
{
  // double int add -> double
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );

  op2->get() += ( op1->get() );
  i->OStack.pop();
}

void
Add_idFunction::execute( SLIInterpreter* i ) const
{
  // ind double add -> double
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );

  op1->get() += ( op2->get() );
  i->OStack.swap();
  i->OStack.pop();
}
//-----------------------------------------------------
void
Sub_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() -= ( op2->get() );
  i->OStack.pop();
}

void
Sub_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() -= ( op2->get() );
  i->OStack.pop();
}

void
Sub_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() -= ( op2->get() );
  i->OStack.pop();
}

void
Sub_idFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  op2->get() = op1->get() - op2->get();
  i->OStack.swap();
  i->OStack.pop();
}
//-----------------------------------------------------
void
Mul_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() *= ( op2->get() );
  i->OStack.pop();
}

void
Mul_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() *= ( op2->get() );
  i->OStack.pop();
}

void
Mul_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() *= ( op2->get() );
  i->OStack.pop();
}

void
Mul_idFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );

  op1->get() *= ( op2->get() );
  i->OStack.swap();
  i->OStack.pop();
}
//-----------------------------------------------------
void
Div_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op2->get() != 0 )
  {
    op1->get() /= ( op2->get() );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->DivisionByZeroError );
  }
}

//-----------------------------------------------------
/** @BeginDocumentation
Name: mod - compute the modulo of two integer numbers.
Synopsis: int int mod -> int
Examples: 7 4 mod -> 3
SeeAlso: E, sin, cos, exp, log
*/
void
Mod_iiFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() < 2 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1 == NULL || op2 == NULL )
  {
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  if ( op2->get() != 0 )
  {
    *op1 = op1->get() % op2->get();
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->DivisionByZeroError );
  }
}

void
Div_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op2->get() != 0 )
  {
    op1->get() /= ( op2->get() );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->DivisionByZeroError );
  }
}

void
Div_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op2->get() != 0 )
  {
    op1->get() /= ( op2->get() );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->DivisionByZeroError );
  }
}

void
Div_idFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op2->get() != 0 )
  {
    op2->get() = ( op1->get() ) / ( op2->get() );
    i->OStack.swap();
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->DivisionByZeroError );
  }
}

/** @BeginDocumentation
 Name: sin - Calculate the sine of double number.
 Synopsis:  double sin -> double

 Description: Alternatives: Function sin_d (undocumented)
 -> behaviour and synopsis are the same.

 Examples:  1.0 sin -> 0.841471

 Author: Hehl
 FirstVersion: 8.6.1999
 SeeAlso: cos
*/

void
Sin_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );


  *op = std::sin( op->get() );
  i->EStack.pop();
}

/** @BeginDocumentation
 Name: asin - Calculate the arc sine of double number.
 Synopsis:  double asin -> double

 Description: Alternatives: Function asin_d (undocumented)
 -> behaviour and synopsis are the same.

 Examples:  1.0 asin -> 1.570796

 Author: Diesmann
 FirstVersion: 090225
 SeeAlso: sin, acos
*/

void
Asin_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = std::asin( op->get() );
  i->EStack.pop();
}


/** @BeginDocumentation
 Name: cos - Calculate the cosine of double number.
 Synopsis:  double cos -> double

 Description: Alternatives: Function cos_d (undocumented)
 -> behaviour and synopsis are the same.

 Examples: 1.0 cos -> 0.540302

 Author: Hehl
 FirstVersion: 8.6.1999
 SeeAlso: sin
*/

void
Cos_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );


  *op = std::cos( op->get() );
  i->EStack.pop();
}

/** @BeginDocumentation
 Name: acos - Calculate the arc cosine of double number.
 Synopsis:  double acos -> double

 Description: Alternatives: Function acos_d (undocumented)
 -> behaviour and synopsis are the same.

 Examples: 1.0 acos -> 0.0

 Author: Diesmann
 FirstVersion: 090225
 SeeAlso: cos, asin
*/

void
Acos_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = std::acos( op->get() );
  i->EStack.pop();
}


/** @BeginDocumentation
 Name: exp - Calculate the exponential of double number
 Synopsis:  double exp -> double
 Examples: 1.0 exp -> 2.71828
 Author: Hehl
 FirstVersion: 10.6.1999
 SeeAlso: E, sin, cos
*/


void
Exp_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = std::exp( op->get() );
  i->EStack.pop();
}

/** @BeginDocumentation
 Name: log - Calculate decadic logarithm of double number.
 Synopsis:  double exp -> double
 Examples: 10.0 log -> 1.0
 Author: Gewaltig
 FirstVersion: 7.6.2000
 SeeAlso: E, sin, cos, exp
*/

void
Log_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op->get() > 0.0 )
  {
    *op = std::log10( op->get() );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
 Name: ln - Calculate natural logarithm of double number.
 Synopsis:  double ln -> double
 Examples: E ln -> 1.0
 Author: Gewaltig
 FirstVersion: 7.6.2000
 SeeAlso: E, sin, cos, exp, log
*/

void
Ln_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  if ( op->get() > 0.0 )
  {
    *op = std::log( op->get() );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
Name: sqr - Compute the square of a number.
Examples: 2.0 sqr -> 4.0
Synopsis: number sqr -> double
SeeAlso: sqrt
*/
void
Sqr_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = op->get() * op->get();
  i->EStack.pop();
}

/** @BeginDocumentation
Name: sqrt - compute the square root of a non-negative number
Synopsis: number sqrt -> double
Description: sqrt computes the the square root of a number.
If the value is negative, a RangeCheck error is raised.
Examples: 4 sqrt -> 2.0
SeeAlso: sqr
*/
void
Sqrt_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op->get() >= 0.0 )
  {
    *op = std::sqrt( op->get() );
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
Name: pow - raise a number to a power
Synopsis: x y pow -> number
Description: pow computes x raised to the y-th power (x^y).
Remarks: Raises a RangeCheck error if x is negative, unless y is positive
integer.
Author: Plesser
FirstVersion: 17.05.2004
SeeAlso: exp, log
*/
void
Pow_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  if ( op1->get() >= 0.0 )
  {
    *op1 = std::pow( op1->get(), op2->get() );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }

  return;
}

void
Pow_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  // can raise anything to an integer power, except zero to neg power
  if ( not( op1->get() == 0.0 && op2->get() < 0 ) )
  {
    // cast explicitly to double to avoid overloading ambiguity
    *op1 = std::pow( op1->get(), static_cast< double >( op2->get() ) );
    i->OStack.pop();
    i->EStack.pop();
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }

  return;
}


/** @BeginDocumentation
 Name: modf - Decomposes its argument into fractional and integral part
 Synopsis: double modf -> double double
 Description:
 This is an interface to the C++ function
 double std::modf(double, double*)
 Examples: 2.5 modf -> 0.5 2
 Author: Diesmann
 FirstVersion: 17.5.2005
 References: Stroustrup 3rd ed p 661
 SeeAlso: frexp
*/
void
Modf_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  double y;

  *op1 = std::modf( op1->get(), &y );
  i->OStack.push_by_pointer( new DoubleDatum( y ) );

  i->EStack.pop();
}


/** @BeginDocumentation
 Name: frexp - Decomposes its argument into an exponent of 2 and a factor
 Synopsis: double frexp -> double integer
 Description:
 This is an interface to the C++ function
 double std::frexp(double,int*)
 In accordance with the normalized representation of the mantissa
 in IEEE doubles, the factor is in the interval [0.5,1).
 Examples: -5 dexp frexp -> 0.5 -4
 Author: Diesmann
 FirstVersion: 17.5.2005
 References: Stroustrup 3rd ed p 661
 SeeAlso: ldexp, dexp, modf
*/
void
Frexp_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  int y;

  *op1 = std::frexp( op1->get(), &y );
  i->OStack.push( y );

  i->EStack.pop();
}


/** @BeginDocumentation
 Name: ldexp - computes the product of integer power of 2 and a factor
 Synopsis: double integer ldexp -> double
 Description:
 This is an interface to the C++ function
 double std::ldexp(double,int)
 Examples:
  1.5 3 ldexp -> 12
  12.0 frexp -> 0.75 4
  12.0 frexp ldexp -> 12.0
  1.0 -5 ldexp frexp -> 0.5 -4
 Author: Diesmann
 FirstVersion: 17.5.2005
 References: Stroustrup 3rd ed p 661
 SeeAlso: frexp, dexp, modf
*/
void
Ldexp_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  *op1 = std::ldexp( op1->get(), op2->get() );

  i->OStack.pop();
  i->EStack.pop();
}

/** @BeginDocumentation
 Name: dexp - computes an integer power of 2 and returns the result as double
 Synopsis: integer dexp -> double
 Description:
 This is an interface to the C++ expression
 double std::ldexp(1.0,int)
 Examples: -5 dexp frexp -> 0.5 -4
 Author: Diesmann
 FirstVersion: 17.5.2005
 References: Stroustrup 3rd ed p 661
 SeeAlso: frexp, ldexp, modf
*/
void
Dexp_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  double d = std::ldexp( 1.0, op1->get() );

  i->OStack.top() = d;
  i->EStack.pop();
}


//----------------------------------

/** @BeginDocumentation
 Name: abs_i - absolute value of integer
 Synopsis:  integer abs -> integer

 Description:
    implemented by C/C++
      long   labs(long) and

 Examples: -3 abs_i -> 3

 Remarks: If you are not sure, if the value is of type double or integer, use
 abs. If e.g. abs_d gets an integer as argument, NEST will exit throwing an
 assertion.
 Author: Diesmann
 FirstVersion: 27.4.1999
 References: Stroustrup 3rd ed p 661
 SeeAlso: abs
 */
void
Abs_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  IntegerDatum* op = static_cast< IntegerDatum* >( i->OStack.top().datum() );

  *op = std::labs( op->get() );
}

/** @BeginDocumentation
 Name: abs_d - absolute value of double
 Synopsis:  double abs -> double

 Description:
    implemented by C/C++
      double fabs(double)

 Examples: -3.456 abs_d -> 3.456

 Remarks: If you are not sure, if the value is of type double or integer, use
 abs. If e.g. abs_d gets an integer as argument, NEST will exit throwing an
 assertion.

 Author: Diesmann
 FirstVersion: 27.4.1999
 References: Stroustrup 3rd ed p 660
 SeeAlso: abs
*/
void
Abs_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.top().datum() );

  *op = std::fabs( op->get() );
}


/** @BeginDocumentation
 Name: neg_i - reverse sign of integer value
 Synopsis:  integer neg -> integer
 Author: Diesmann
 FirstVersion: 29.7.1999
 Remarks:
    implemented by C/C++
      - operator
  This function is called CHS in HP48S and
  related dialects.

 SeeAlso:neg
*/
void
Neg_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  IntegerDatum* op = static_cast< IntegerDatum* >( i->OStack.top().datum() );

  *op = -op->get();
}

/** @BeginDocumentation
 Name: neg_d - reverse sign of double value
 Synopsis:   double neg -> double
 Author: Diesmann
 FirstVersion: 29.7.1999
 Remarks:
    implemented by C/C++
      - operator
  This function is called CHS in HP48S and
  related dialects.

 SeeAlso: neg
*/
void
Neg_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.top().datum() );
  *op = -op->get();
}

/** @BeginDocumentation
   Name: inv - compute 1/x
   Synopsis:   double inv -> double
   Examples: 2.0 inv -> 0.5
   Author: Gewaltig
*/
void
Inv_dFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }


  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( op == NULL )
  {
    i->raiseerror( i->ArgumentTypeError );
    return;
  }


  *op = 1.0 / op->get();
  i->EStack.pop();
}


/** @BeginDocumentation:
Name: eq - Test two objects for equality
Synopsis: any1 any2 eq -> bool

Description: eq returns true if the two arguments are equal and false
otherwise.

eq can also be applied to container objecs like arrays, procedures, strings,
and dictionaries:
* two arrays or strings are equal, if all their components are equal.
* two dictionaries are equal, if they represent the same object.
SeeAlso: neq, gt, lt, leq, geq
*/

void
EqFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  Datum* op1 = i->OStack.pick( 1 ).datum();
  Datum* op2 = i->OStack.pick( 0 ).datum();

  bool result = op1->equals( op2 );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

/** @BeginDocumentation:
Name: neq - Test two objects for inequality
Synopsis: any1 any2 neq -> bool

Description: neq returns true if the two arguments are not equal and false
otherwise.

neq can also be applied to container objecs like arrays, procedures, strings,
and dictionaries:
* two arrays or strings are equal, if all their components are equal.
* two dictionaries are equal, if they represent the same object.
SeeAlso: eq, gt, lt, leq, geq
*/
void
NeqFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  Datum* op1 = i->OStack.pick( 1 ).datum();
  Datum* op2 = i->OStack.pick( 0 ).datum();

  bool result = not op1->equals( op2 );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

/** @BeginDocumentation:
Name: geq - Test if one object is greater or equal than another object
Synopsis: any1 any2 geq -> bool

Description: geq returns true if any1 >= any2 and false
otherwise.

geq can only be applied to numbers and strings.
SeeAlso: eq, neq, gt, lt, leq
*/

void
Geq_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() >= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Geq_idFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() >= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Geq_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() >= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Geq_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() >= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

/** @BeginDocumentation:
Name: leq - Test if one object is less or equal than another object
Synopsis: any1 any2 leq -> bool

Description: geq returns true if any1 <= any2 and false
otherwise.

geq can only be applied to numbers and strings.
SeeAlso: eq, neq, gt, lt, geq
*/

void
Leq_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() <= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Leq_idFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() <= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Leq_diFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() <= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Leq_ddFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = ( op1->get() <= op2->get() );
  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

/** @BeginDocumentation
Name: not - logical not operator.
Synopsis: bool not -> bool
          int  not -> int
Description: For booleans, not turns true info false and vice versa.
             For integer arguments, not performs a bit-wise not where
             1 is replaced by 0 and vice versa.
Examples: 1 2 eq not -> true
               0 not -> -1
SeeAlso: and, or, xor
*/

void
Not_bFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  BoolDatum* op = static_cast< BoolDatum* >( i->OStack.top().datum() );
  op->get() = not op->get();
}


void
Not_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );
  i->EStack.pop();

  IntegerDatum* op = static_cast< IntegerDatum* >( i->OStack.top().datum() );
  op->get() = ~op->get();
}

/** @BeginDocumentation
Name: or - logical or operator.
Synopsis: bool1 bool2 or -> bool
          int1  int2  or -> int

Description: For booleans, or returns true, if either,
             the arguments are true and false
             otherwise.
             For integers, or performs a bitwise or between the
             two arguments.

Examples: true  false or -> true
          true  true  or -> true
          false false or -> false
          2     8     or -> 10
          1     0     or -> 1
SeeAlso: and, or, not
*/
void
OrFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  BoolDatum* op1 = static_cast< BoolDatum* >( i->OStack.pick( 1 ).datum() );
  BoolDatum* op2 = static_cast< BoolDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  op1->get() = ( op1->get() == true || op2->get() == true );

  i->OStack.pop();
}

/** @BeginDocumentation
Name: xor - logical xor operator.
Synopsis: bool1 bool2 xor -> bool

Description: For booleans, xor returns true, if either,
             but not both of the arguments are true and false
             otherwise.

Examples: true  false xor -> true
          true  true  xor -> false
          false false xor -> false
SeeAlso: and, or, not
*/

void
XorFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  BoolDatum* op1 = static_cast< BoolDatum* >( i->OStack.pick( 1 ).datum() );
  BoolDatum* op2 = static_cast< BoolDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() = ( ( *op1 || *op2 ) && not( *op1 && *op2 ) );

  i->OStack.pop();
}

/** @BeginDocumentation
Name: and - logical and operator.
Synopsis: bool1 bool2 and -> bool
          int1  int2  and -> int
Description: For booleans, and returns true if both arguments are true
             For integer arguments, and performs a bit-wise and between
             the two integers.

Examples: true true and -> true
          10   24   and -> 8
SeeAlso: or, xor, not
*/


void
AndFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  BoolDatum* op1 = static_cast< BoolDatum* >( i->OStack.pick( 1 ).datum() );
  BoolDatum* op2 = static_cast< BoolDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() = ( *op1 && *op2 );

  i->OStack.pop();
}

void
And_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() = op1->get() & op2->get();
  i->OStack.pop();
}

void
Or_iiFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  op1->get() = op1->get() | op2->get();
  i->OStack.pop();
}

//---------------------------------------------------
/** @BeginDocumentation:
Name: gt - Test if one object is greater than another object
Synopsis: any1 any2 gt -> bool

Description: gt returns true if any1 > any2 and false
otherwise.

gt can only be applied to numbers and strings.
SeeAlso: eq, neq, gt, lt, leq
*/
void
Gt_idFunction::execute( SLIInterpreter* i ) const
{
  // call: integer double gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = *op1 > *op2;

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Gt_diFunction::execute( SLIInterpreter* i ) const
{
  // call: double integer gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );

  bool result = *op1 > *op2;

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}


void
Gt_iiFunction::execute( SLIInterpreter* i ) const
{
  // call: integer integer gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = op1->get() > op2->get();

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Gt_ddFunction::execute( SLIInterpreter* i ) const
{
  // call: double double gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = op1->get() > op2->get();

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}


void
Gt_ssFunction::execute( SLIInterpreter* i ) const
{
  // call: string string gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  StringDatum* op1 = static_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* op2 = static_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = *op1 > *op2;

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}
//----
/** @BeginDocumentation:
Name: lt - Test if one object is less than another object
Synopsis: any1 any2 lt -> bool

Description: lt returns true if any1 < any2 and false
otherwise.

lt can only be applied to numbers and strings.
SeeAlso: eq, neq, gt, lt, leq
*/
void
Lt_idFunction::execute( SLIInterpreter* i ) const
{
  // call: integer double gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = op1->get() < op2->get();

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Lt_diFunction::execute( SLIInterpreter* i ) const
{
  // call: double integer gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = op1->get() < op2->get();

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}


void
Lt_iiFunction::execute( SLIInterpreter* i ) const
{
  // call: integer integer gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = op1->get() < op2->get();

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}

void
Lt_ddFunction::execute( SLIInterpreter* i ) const
{
  // call: double double gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = op1->get() < op2->get();

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}


void
Lt_ssFunction::execute( SLIInterpreter* i ) const
{
  // call: string string gt bool
  assert( i->OStack.load() > 1 );
  i->EStack.pop();

  StringDatum* op1 = static_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* op2 = static_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );
  assert( op1 != NULL && op2 != NULL );

  bool result = *op1 < *op2;

  i->OStack.pop( 2 );
  i->OStack.push_by_pointer( new BoolDatum( result ) );
}


// Documentation can be found in file synod2/lib/sli/mathematica.sli
void
UnitStep_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* x = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  bool result = x->get() >= 0.0;


  i->EStack.pop();
  i->OStack.pop();
  if ( result )
  {
    i->OStack.push_by_pointer( new DoubleDatum( 1.0 ) );
  }
  else
  {
    i->OStack.push_by_pointer( new DoubleDatum( 0.0 ) );
  }
}

// Documentation can be found in file synod2/lib/sli/mathematica.sli
void
UnitStep_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  IntegerDatum* x = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  assert( x != NULL );

  bool result = x->get() >= 0;


  i->EStack.pop();
  i->OStack.pop();
  if ( result )
  {
    i->OStack.push_by_pointer( new IntegerDatum( 1.0 ) );
  }
  else
  {
    i->OStack.push_by_pointer( new IntegerDatum( 0.0 ) );
  }
}

// Documentation can be found in file synod2/lib/sli/mathematica.sli
void
UnitStep_daFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  TokenArray* a = dynamic_cast< TokenArray* >( i->OStack.pick( 0 ).datum() );
  assert( a != NULL );

  bool result = true;

  for ( size_t j = 0; j < a->size(); ++j )
  {
    DoubleDatum* x = static_cast< DoubleDatum* >( ( *a )[ j ].datum() );
    assert( x != NULL );
    if ( x->get() < 0.0 )
    {
      result = false;
      break;
    }
  }

  i->EStack.pop();
  i->OStack.pop();
  if ( result )
  {
    i->OStack.push_by_pointer( new DoubleDatum( 1.0 ) );
  }
  else
  {
    i->OStack.push_by_pointer( new DoubleDatum( 0.0 ) );
  }
}

// Documentation can be found in file synod2/lib/sli/mathematica.sli
void
UnitStep_iaFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  TokenArray* a = dynamic_cast< TokenArray* >( i->OStack.pick( 0 ).datum() );
  assert( a != NULL );

  bool result = true;

  for ( size_t j = 0; j < a->size(); ++j )
  {
    IntegerDatum* x = static_cast< IntegerDatum* >( ( *a )[ j ].datum() );
    assert( x != NULL );
    if ( x->get() < 0 )
    {
      result = false;
      break;
    }
  }

  i->EStack.pop();
  i->OStack.pop();
  if ( result )
  {
    i->OStack.push_by_pointer( new IntegerDatum( 1.0 ) );
  }
  else
  {
    i->OStack.push_by_pointer( new IntegerDatum( 0.0 ) );
  }
}

// round to the nearest integer
void
Round_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = std::floor( op->get() + 0.5 );
  i->EStack.pop();
}

// Documentation can be found in file synod2/lib/sli/mathematica.sli
void
Floor_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = std::floor( op->get() );
  i->EStack.pop();
}

// Documentation can be found in file synod2/lib/sli/mathematica.sli
void
Ceil_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 1 );

  DoubleDatum* op = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  *op = std::ceil( op->get() );
  i->EStack.pop();
}


// Documentation can be found in file synod2/lib/sli/typeinit.sli
void
Max_i_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() < op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}
void
Max_i_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() < op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}
void
Max_d_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() < op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}
void
Max_d_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() < op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}


// Documentation can be found in file synod2/lib/sli/typeinit.sli
void
Min_i_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() > op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}
void
Min_i_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  IntegerDatum* op1 = static_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() > op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}
void
Min_d_iFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* op2 = static_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() > op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}
void
Min_d_dFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() >= 2 );
  i->EStack.pop();

  DoubleDatum* op1 = static_cast< DoubleDatum* >( i->OStack.pick( 1 ).datum() );
  DoubleDatum* op2 = static_cast< DoubleDatum* >( i->OStack.pick( 0 ).datum() );

  if ( op1->get() > op2->get() )
  {
    i->OStack.swap();
  }

  i->OStack.pop();
}


const IntegerFunction integerfunction;
const DoubleFunction doublefunction;
const Add_ddFunction add_ddfunction;
const Add_diFunction add_difunction;
const Add_idFunction add_idfunction;
const Add_iiFunction add_iifunction;
const Sub_ddFunction sub_ddfunction;
const Sub_diFunction sub_difunction;
const Sub_idFunction sub_idfunction;
const Sub_iiFunction sub_iifunction;

const Mul_ddFunction mul_ddfunction;
const Mul_diFunction mul_difunction;
const Mul_idFunction mul_idfunction;
const Mul_iiFunction mul_iifunction;
const Div_ddFunction div_ddfunction;
const Div_diFunction div_difunction;
const Div_idFunction div_idfunction;
const Div_iiFunction div_iifunction;
const Sin_dFunction sin_dfunction;
const Asin_dFunction asin_dfunction;
const Cos_dFunction cos_dfunction;
const Acos_dFunction acos_dfunction;
const Exp_dFunction exp_dfunction;
const Ln_dFunction ln_dfunction;
const Log_dFunction log_dfunction;
const Sqr_dFunction sqr_dfunction;
const Sqrt_dFunction sqrt_dfunction;
const Pow_ddFunction pow_ddfunction;
const Pow_diFunction pow_difunction;

const Modf_dFunction modf_dfunction;
const Frexp_dFunction frexp_dfunction;

const Ldexp_diFunction ldexp_difunction;
const Dexp_iFunction dexp_ifunction;

const Mod_iiFunction mod_iifunction;

const Abs_iFunction abs_ifunction;
const Abs_dFunction abs_dfunction;

const Neg_iFunction neg_ifunction;
const Neg_dFunction neg_dfunction;
const Inv_dFunction inv_dfunction;

const EqFunction eqfunction;
const OrFunction orfunction;
const XorFunction xorfunction;
const AndFunction andfunction;
const And_iiFunction and_iifunction;
const Or_iiFunction or_iifunction;

const Geq_iiFunction geq_iifunction;
const Geq_idFunction geq_idfunction;
const Geq_diFunction geq_difunction;
const Geq_ddFunction geq_ddfunction;

const Leq_iiFunction leq_iifunction;
const Leq_idFunction leq_idfunction;
const Leq_diFunction leq_difunction;
const Leq_ddFunction leq_ddfunction;


const NeqFunction neqfunction;
const Not_bFunction not_bfunction;
const Not_iFunction not_ifunction;

const Gt_iiFunction gt_iifunction;
const Gt_ddFunction gt_ddfunction;
const Gt_idFunction gt_idfunction;
const Gt_diFunction gt_difunction;
const Gt_ssFunction gt_ssfunction;

const Lt_iiFunction lt_iifunction;
const Lt_ddFunction lt_ddfunction;
const Lt_idFunction lt_idfunction;
const Lt_diFunction lt_difunction;
const Lt_ssFunction lt_ssfunction;

const UnitStep_iFunction unitstep_ifunction;
const UnitStep_dFunction unitstep_dfunction;
const UnitStep_iaFunction unitstep_iafunction;
const UnitStep_daFunction unitstep_dafunction;

const Round_dFunction round_dfunction;
const Floor_dFunction floor_dfunction;
const Ceil_dFunction ceil_dfunction;

const Max_d_dFunction max_d_dfunction;
const Max_d_iFunction max_d_ifunction;
const Max_i_dFunction max_i_dfunction;
const Max_i_iFunction max_i_ifunction;

const Min_d_dFunction min_d_dfunction;
const Min_d_iFunction min_d_ifunction;
const Min_i_dFunction min_i_dfunction;
const Min_i_iFunction min_i_ifunction;


void
init_slimath( SLIInterpreter* i )
{
  i->createcommand( "int_d", &integerfunction );
  i->createcommand( "double_i", &doublefunction );
  i->createcommand( "add_dd", &add_ddfunction );
  i->createcommand( "add_di", &add_difunction );
  i->createcommand( "add_id", &add_idfunction );
  i->createcommand( "add_ii", &add_iifunction );
  //
  i->createcommand( "sub_dd", &sub_ddfunction );
  i->createcommand( "sub_di", &sub_difunction );
  i->createcommand( "sub_id", &sub_idfunction );
  i->createcommand( "sub_ii", &sub_iifunction );
  //
  i->createcommand( "mul_dd", &mul_ddfunction );
  i->createcommand( "mul_di", &mul_difunction );
  i->createcommand( "mul_id", &mul_idfunction );
  i->createcommand( "mul_ii", &mul_iifunction );
  //
  i->createcommand( "div_dd", &div_ddfunction );
  i->createcommand( "div_di", &div_difunction );
  i->createcommand( "div_id", &div_idfunction );
  i->createcommand( "div_ii", &div_iifunction );
  i->createcommand( "mod", &mod_iifunction );
  //
  i->createcommand( "sin_d", &sin_dfunction );
  i->createcommand( "asin_d", &asin_dfunction );
  i->createcommand( "cos_d", &cos_dfunction );
  i->createcommand( "acos_d", &acos_dfunction );
  i->createcommand( "exp_d", &exp_dfunction );
  i->createcommand( "log_d", &log_dfunction );
  i->createcommand( "ln_d", &ln_dfunction );
  i->createcommand( "sqr_d", &sqr_dfunction );
  i->createcommand( "sqrt_d", &sqrt_dfunction );
  i->createcommand( "pow_dd", &pow_ddfunction );
  i->createcommand( "pow_di", &pow_difunction );
  //

  i->createcommand( "modf_d", &modf_dfunction );
  i->createcommand( "frexp_d", &frexp_dfunction );
  //
  i->createcommand( "ldexp_di", &ldexp_difunction );
  i->createcommand( "dexp_i", &dexp_ifunction );
  //
  i->createcommand( "abs_i", &abs_ifunction );
  i->createcommand( "abs_d", &abs_dfunction );
  //
  i->createcommand( "neg_i", &neg_ifunction );
  i->createcommand( "neg_d", &neg_dfunction );
  i->createcommand( "inv", &inv_dfunction );
  //
  i->createcommand( "eq", &eqfunction );
  i->createcommand( "and", &andfunction );
  i->createcommand( "and_ii", &and_iifunction );
  i->createcommand( "or_ii", &or_iifunction );
  i->createcommand( "or", &orfunction );
  i->createcommand( "xor", &xorfunction );

  i->createcommand( "leq_ii", &leq_iifunction );
  i->createcommand( "leq_id", &leq_idfunction );
  i->createcommand( "leq_di", &leq_difunction );
  i->createcommand( "leq_dd", &leq_ddfunction );

  i->createcommand( "geq_ii", &geq_iifunction );
  i->createcommand( "geq_id", &geq_idfunction );
  i->createcommand( "geq_di", &geq_difunction );
  i->createcommand( "geq_dd", &geq_ddfunction );

  i->createcommand( "neq", &neqfunction );
  i->createcommand( "not_b", &not_bfunction );
  i->createcommand( "not_i", &not_ifunction );
  //
  i->createcommand( "gt_ii", &gt_iifunction );
  i->createcommand( "gt_dd", &gt_ddfunction );
  i->createcommand( "gt_id", &gt_idfunction );
  i->createcommand( "gt_di", &gt_difunction );
  i->createcommand( "gt_ss", &gt_ssfunction );
  //
  i->createcommand( "lt_ii", &lt_iifunction );
  i->createcommand( "lt_dd", &lt_ddfunction );
  i->createcommand( "lt_id", &lt_idfunction );
  i->createcommand( "lt_di", &lt_difunction );
  i->createcommand( "lt_ss", &lt_ssfunction );
  //
  i->createcommand( "UnitStep_i", &unitstep_ifunction );
  i->createcommand( "UnitStep_d", &unitstep_dfunction );
  i->createcommand( "UnitStep_ia", &unitstep_iafunction );
  i->createcommand( "UnitStep_da", &unitstep_dafunction );
  //
  i->createcommand( "round_d", &round_dfunction );
  i->createcommand( "floor_d", &floor_dfunction );
  i->createcommand( "ceil_d", &ceil_dfunction );
  //
  i->createcommand( "max_d_d", &max_d_dfunction );
  i->createcommand( "max_d_i", &max_d_ifunction );
  i->createcommand( "max_i_d", &max_i_dfunction );
  i->createcommand( "max_i_i", &max_i_ifunction );
  //
  i->createcommand( "min_d_d", &min_d_dfunction );
  i->createcommand( "min_d_i", &min_d_ifunction );
  i->createcommand( "min_i_d", &min_i_dfunction );
  i->createcommand( "min_i_i", &min_i_ifunction );
}
