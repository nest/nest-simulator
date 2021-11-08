/*
 *  specialfunctionsmodule.cc
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

#include "specialfunctionsmodule.h"

// C++ includes:
#include <cmath>

// Generated includes:
#include "config.h" // has definition of HAVE_GSL

// Includes from sli:
#include "doubledatum.h" // Include the data-types we use!

#ifdef HAVE_GSL

// External includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_sf_erf.h>   // as more and more special functions get
#include <gsl/gsl_sf_gamma.h> // added, replace by <gsl/gsl_sf.h>
#include <gsl/gsl_sf_lambert.h>

#endif

const int SpecialFunctionsModule::GaussDiskConvFunction::MAX_QUAD_SIZE = 5000;
const double SpecialFunctionsModule::GaussDiskConvFunction::QUAD_ERR_LIM = 1e-12;
const double SpecialFunctionsModule::GaussDiskConvFunction::QUAD_ERR_SCALE = 200.0;


// We need this for some compiling reason... (ask Bjarne)
const SpecialFunctionsModule::GammaIncFunction gammaincfunction;
const SpecialFunctionsModule::ErfFunction erffunction;
const SpecialFunctionsModule::ErfcFunction erfcfunction;
const SpecialFunctionsModule::GaussDiskConvFunction gaussdiskconvfunction;
const SpecialFunctionsModule::LambertW0Function lambertw0function;
const SpecialFunctionsModule::LambertWm1Function lambertwm1function;

// Part 1: Methods pertaining to the entire module -----------------

// GSL independent code
const std::string
SpecialFunctionsModule::name( void ) const
{
  return std::string( "SpecialFunctionsModule" ); // Return name of the module
}

void
SpecialFunctionsModule::init( SLIInterpreter* i )
{
// Do whatever initialization is needed, then...

#ifdef HAVE_GSL
  // turn error handler off, so that errors in GSL functions
  // do not lead to a core dump
  gsl_set_error_handler_off();
#endif

  // ...don't forget to create the new SLI-commands!
  i->createcommand( "Gammainc", &gammaincfunction );
  i->createcommand( "LambertW0", &lambertw0function );
  i->createcommand( "LambertWm1", &lambertwm1function );
  i->createcommand( "Erf", &erffunction );
  i->createcommand( "Erfc", &erfcfunction );
  i->createcommand( "GaussDiskConv", &gaussdiskconvfunction );
}


// Part 2: Methods pertaining to the individual functions ----------

// NOTE: see below for dummy implementations in absence of GSL
#ifdef HAVE_GSL

void
SpecialFunctionsModule::GammaIncFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop(); // pop yourself

  if ( i->OStack.load() < 2 )
  { // expect two arguments on stack
    i->raiseerror( "Gammainc", "two arguments required" );
    return;
  }

  // get top argument
  DoubleDatum* da = dynamic_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( not da )
  {
    i->raiseerror( "Gammainc", "arguments must be doubles" );
    return;
  }
  i->OStack.pop(); // pop top argument

  // get second argument, leave datum on stack
  DoubleDatum* dx = dynamic_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( not dx )
  {
    i->raiseerror( "Gammainc", "arguments must be doubles" );
    return;
  }

  // computation via GSL
  gsl_sf_result result;

  int status = gsl_sf_gamma_inc_P_e( da->get(), dx->get(), &result );
  if ( status )
  {
    i->raiseerror( "Gammainc[GSL]", gsl_strerror( status ) );
    return;
  }

  // return result value through argument object still on stack
  ( *dx ) = result.val;
}

// ---------------------------------------------------------------


// see mathematica.sli for documentation
void
SpecialFunctionsModule::LambertW0Function::execute( SLIInterpreter* i ) const
{
  i->EStack.pop(); // pop yourself

  if ( i->OStack.load() < 1 )
  { // expect one arguments on stack
    i->raiseerror( "LambertW0", "one argument required" );
    return;
  }

  // get argument, leave datum on stack
  DoubleDatum* dx = dynamic_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( not dx )
  {
    i->raiseerror( "LambertW0", "argument must be doubles" );
    return;
  }

  // computation via GSL
  gsl_sf_result result;
  int status = gsl_sf_lambert_W0_e( dx->get(), &result );
  if ( status )
  {
    i->raiseerror( "LambertW0[GSL]", gsl_strerror( status ) );
    return;
  }

  // return result value through argument object still on stack
  ( *dx ) = result.val;
}

// see mathematica.sli for documentation
void
SpecialFunctionsModule::LambertWm1Function::execute( SLIInterpreter* i ) const
{
  i->EStack.pop(); // pop yourself

  if ( i->OStack.load() < 1 )
  { // expect one arguments on stack
    i->raiseerror( "LambertWm1", "one argument required" );
    return;
  }

  // get argument, leave datum on stack
  DoubleDatum* dx = dynamic_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( not dx )
  {
    i->raiseerror( "LambertWm1", "argument must be doubles" );
    return;
  }

  // computation via GSL
  gsl_sf_result result;
  int status = gsl_sf_lambert_Wm1_e( dx->get(), &result );
  if ( status )
  {
    i->raiseerror( "LambertWm1[GSL]", gsl_strerror( status ) );
    return;
  }

  // return result value through argument object still on stack
  ( *dx ) = result.val;
}


void
SpecialFunctionsModule::ErfFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop(); // pop yourself

  if ( i->OStack.load() < 1 )
  { // expect one arguments on stack
    i->raiseerror( "Erf", "one argument required" );
    return;
  }

  // get argument, leave datum on stack
  DoubleDatum* dx = dynamic_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( not dx )
  {
    i->raiseerror( "Erf", "arguments must be doubles" );
    return;
  }

  // computation via GSL
  gsl_sf_result result;
  int status = gsl_sf_erf_e( dx->get(), &result );
  if ( status )
  {
    i->raiseerror( "Erf[GSL]", gsl_strerror( status ) );
    return;
  }

  // return result value through argument object still on stack
  ( *dx ) = result.val;
}

// ---------------------------------------------------------------

void
SpecialFunctionsModule::ErfcFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop(); // pop yourself

  if ( i->OStack.load() < 1 )
  { // expect one arguments on stack
    i->raiseerror( "Erfc", "one argument required" );
    return;
  }

  // get argument, leave datum on stack
  DoubleDatum* dx = dynamic_cast< DoubleDatum* >( i->OStack.top().datum() );
  if ( not dx )
  {
    i->raiseerror( "Erfc", "arguments must be doubles" );
    return;
  }

  // computation via GSL
  gsl_sf_result result;
  int status = gsl_sf_erfc_e( dx->get(), &result );
  if ( status )
  {
    i->raiseerror( "Erfc[GSL]", gsl_strerror( status ) );
    return;
  }

  // return result value through argument object still on stack
  ( *dx ) = result.val;
}

// ---------------------------------------------------------------

gsl_function SpecialFunctionsModule::GaussDiskConvFunction::F_;


SpecialFunctionsModule::GaussDiskConvFunction::GaussDiskConvFunction( void )
{
  // allocate integration workspace
  w_ = gsl_integration_workspace_alloc( MAX_QUAD_SIZE );

  // set integrand function
  F_.function = SpecialFunctionsModule::GaussDiskConvFunction::f_;
}

SpecialFunctionsModule::GaussDiskConvFunction::~GaussDiskConvFunction( void )
{
  // free integration workspace
  gsl_integration_workspace_free( w_ );
}

void
SpecialFunctionsModule::GaussDiskConvFunction::execute( SLIInterpreter* i ) const
{

  i->EStack.pop(); // pop yourself
  i->assert_stack_load( 2 );

  double r0 = i->OStack.top();
  double R = i->OStack.pick( 1 );

  // copy arguments to doubles, square, as they are needed several times
  //  const double z = std::pow(r0, 2); // commented out, since unused. mog.
  const double y = std::pow( R, 2 );

  // check for simple cases first
  gsl_sf_result X;
  double result;
  if ( y < 2 * GSL_DBL_EPSILON )
  { /* disk has zero diameter */
    result = 0.0;
  }
  else if ( r0 < 2 * GSL_DBL_EPSILON )
  { /* Gaussian is concentric */
    int status = gsl_sf_expm1_e( -y, &X );
    if ( not status )
    {
      result = -X.val;
    }
    else
    {
      i->raiseerror( "GaussDiskConv[GSL]", gsl_strerror( status ) );
      return;
    }
  }
  else if ( std::fabs( R - r0 ) < 2 * GSL_DBL_EPSILON )
  { /* Gaussian on perimeter */
    int status = gsl_sf_bessel_I0_scaled_e( 2.0 * y, &X );
    if ( not status )
    {
      result = 0.5 * ( 1.0 - X.val );
    }
    else
    {
      i->raiseerror( "GaussDiskConv[GSL]", gsl_strerror( status ) );
      return;
    }
  }
  else if ( R > r0 + sqrt( -log( GSL_DBL_EPSILON ) ) )
  { /* Gaussian in disk */
    result = 1.0;
  }
  else if ( y > 1 && r0 > R + sqrt( -log( GSL_DBL_EPSILON / y ) ) )
  { /* tail */
    result = 0.25 * R / r0 * ( std::exp( -( r0 - R ) * ( r0 - R ) ) - std::exp( -( r0 + R ) * ( r0 + R ) ) );
  }
  else
  { /* in all other cases, integration */

    // parameter for integrand function
    F_.params = &r0;

    double C = 0.0;
    double Cerr = 0.0;
    int status = gsl_integration_qag( &F_, 0.0, R, 0.0, QUAD_ERR_LIM, MAX_QUAD_SIZE, GSL_INTEG_GAUSS61, w_, &C, &Cerr );

    if ( status )
    {
      i->raiseerror( "GaussDiskConv[GSL]", gsl_strerror( status ) );
      return;
    }

    if ( C <= 1.0 )
    {
      result = C;
    }
    else
    {
      result = 1.0;
    }
  }

  // return result value through argument object still on stack
  i->OStack.pop();
  i->OStack.top() = result;
}

// integrand function --- C linkage, so we can pass it to GSL
extern "C" inline double
SpecialFunctionsModule::GaussDiskConvFunction::f_( double r, void* params )
{
  double r0 = *( double* ) params;

  int status;
  gsl_sf_result X;

  status = gsl_sf_bessel_I0_scaled_e( 2.0 * r * r0, &X );
  if ( status )
  {
    return GSL_NAN;
  }
  else
  {
    return 2.0 * r * exp( -( r - r0 ) * ( r - r0 ) ) * X.val;
  }
}

// ---------------------------------------------------------------

#else

// dummy implementations when no GSL

// ---------------------------------------------------------------

void
SpecialFunctionsModule::GammaIncFunction::execute( SLIInterpreter* i ) const
{
  i->raiseerror( "Gammainc", "Not implemented (no GSL)" );
}

// ---------------------------------------------------------------

void
SpecialFunctionsModule::LambertW0Function::execute( SLIInterpreter* i ) const
{
  i->raiseerror( "LambertW0", "Not implemented (no GSL)" );
}

// ---------------------------------------------------------------

void
SpecialFunctionsModule::LambertWm1Function::execute( SLIInterpreter* i ) const
{
  i->raiseerror( "LambertWm1", "Not implemented (no GSL)" );
}

// ---------------------------------------------------------------

void
SpecialFunctionsModule::ErfFunction::execute( SLIInterpreter* i ) const
{
  i->raiseerror( "Erf", "Not implemented (no GSL)" );
}

// ---------------------------------------------------------------

void
SpecialFunctionsModule::ErfcFunction::execute( SLIInterpreter* i ) const
{
  i->raiseerror( "Erfc", "Not implemented (no GSL)" );
}

// ---------------------------------------------------------------

SpecialFunctionsModule::GaussDiskConvFunction::GaussDiskConvFunction( void )
{
}

SpecialFunctionsModule::GaussDiskConvFunction::~GaussDiskConvFunction( void )
{
}

void
SpecialFunctionsModule::GaussDiskConvFunction::execute( SLIInterpreter* i ) const
{
  i->raiseerror( "GaussDiskConv", "Not implemented (no GSL)" );
}

// ---------------------------------------------------------------
#endif
