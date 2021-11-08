/*
 *  specialfunctionsmodule.h
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

#ifndef SPECIALFUNCTIONSMODULE_H
#define SPECIALFUNCTIONSMODULE_H
/*
    SLI Module implementing functions from the GNU Science Library.
    The GSL is available from sources.redhat.com/gsl.
*/

/*
    NOTE: Special functions are available only if the GSL is installed.
          If no GSL is available, calling special functions will result
          in a SLI error message.  HEP 2002-09-19.
*/

// Generated includes:
#include "config.h"

// Includes from sli:
#include "slifunction.h"
#include "slimodule.h"

#ifdef HAVE_GSL
// External include:
#include <gsl/gsl_integration.h>
#endif

// NOTE: all gsl headers are included in specialfunctionsmodule.cc

class SpecialFunctionsModule : public SLIModule
{

  // Part 1: Methods pertaining to the module ----------------------

public:
  SpecialFunctionsModule( void ){};
  // ~SpecialFunctionsModule(void);

  // The Module is registered by a call to this Function:
  void init( SLIInterpreter* );

  // This function will return the name of our module:
  const std::string name( void ) const;


  // Part 2: Classes for the implemented functions -----------------


public:
  /**
   * Classes which implement the GSL Funktions.
   * These must be public, since we want to export
   * objects of these.
   */
  class GammaIncFunction : public SLIFunction
  {
  public:
    GammaIncFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };
  class LambertW0Function : public SLIFunction
  {
  public:
    LambertW0Function()
    {
    }
    void execute( SLIInterpreter* ) const;
  };
  class LambertWm1Function : public SLIFunction
  {
  public:
    LambertWm1Function()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class ErfFunction : public SLIFunction
  {
  public:
    ErfFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class ErfcFunction : public SLIFunction
  {
  public:
    ErfcFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class GaussDiskConvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;

    // need constructor and destructor to set up integration workspace
    GaussDiskConvFunction( void );
    ~GaussDiskConvFunction( void );

  private:
    // quadrature parameters, see GSL Reference
    static const int MAX_QUAD_SIZE;
    static const double QUAD_ERR_LIM;
    static const double QUAD_ERR_SCALE;

// integration workspace
#ifdef HAVE_GSL
    gsl_integration_workspace* w_;

    /**
     * Integrand function.
     * @note This function must be static with C linkage so that it can
     *       be passed to the GSL. Alternatively, one could define it
     *       outside the class.
     */
    static double f_( double, void* );
    static gsl_function F_; // GSL wrapper struct for it
#endif
  };

  // Part 3: One instatiation of each new function class -----------

public:
  const GammaIncFunction gammaincfunction;
  const LambertW0Function lambertw0function;
  const LambertWm1Function lambertwm1function;
  const ErfFunction erffunction;
  const ErfcFunction erfcfunction;
  const GaussDiskConvFunction gaussdiskconvfunction;

  // Part 3b: Internal variables
private:
};


// Part 4: Documentation for all functions -------------------------

/** @BeginDocumentation

Name: Gammainc - incomplete gamma function

Synopsis: x a Gammainc -> result

Description: Computes the incomplete Gamma function
             int(t^(a-1)*exp(-t), t=0..x) / Gamma(a)

Parameters:  x      (double): upper limit of integration
             a      (double): order of Gamma function

Examples: 2.2 1.5 Gammainc -> 0.778615

Author: H E Plesser

FirstVersion: 2001-07-26

Remarks: This is the incomplete Gamma function P(a,x) defined as no. 6.5.1
         in Abramowitz&Stegun.  Requires the GSL.

References: http://sources.redhat.com/gsl/ref
*/

/** @BeginDocumentation

Name: Erf - error function

Synopsis: x Erf -> result

Description: Computes the error function
             erf(x) = 2/sqrt(pi) int_0^x dt exp(-t^2)

Parameters:  x (double): error function argument

Examples: 0.5 erf -> 0.5205

Author: H E Plesser

FirstVersion: 2001-07-30

Remarks: Requires the GSL.

References: http://sources.redhat.com/gsl/ref

SeeAlso: Erfc
*/

/** @BeginDocumentation

Name: Erfc - complementary error function

Synopsis: x Erfc -> result

Description: Computes the error function
             erfc(x) = 1 - erf(x) = 2/sqrt(pi) int_x^inf dt exp(-t^2)

Parameters:  x (double): error function argument

Examples: 0.5 erfc -> 0.4795

Author: H E Plesser

FirstVersion: 2001-07-30

Remarks: Requires the GSL.

References: http://sources.redhat.com/gsl/ref

SeeAlso: Erf
*/

/** @BeginDocumentation

Name:GaussDiskConv - Convolution of a Gaussian with an excentric disk

Synopsis:R r0 GaussDiskConv -> result

Description:Computes the convolution of an excentric normalized Gaussian
with a disk

       C[R, r0] = IInt[ disk(rvec; R) * Gauss(rvec - r_0vec) d^2rvec ]
                = 2 Int[ r Exp[-r0^2-r^2] I_0[2 r r_0] dr, r=0..R]

Parameters:R   radius of the disk, centered at origin
r0  distance of Gaussian center from origin

Examples:SLI ] 3.2 2.3 GaussDiskConv =
0.873191

Author:H E Plesser

FirstVersion: 2002-07-12

Remarks:This integral is needed to compute the response of a DOG model to
 excentric light spots, see [1].  For technicalities, see [2].  Requires GSL.

References: [1] G. T. Einevoll and P. Heggelund, Vis Neurosci 17:871-885 (2000).
 [2] Hans E. Plesser, Convolution of an Excentric Gaussian with a Disk,
     Technical Report, arken.nlh.no/~itfhep, 2002

*/

#endif
