/*
 *  randomtest.cpp
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

// C++ includes:
#include <cmath>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>

// Generated includes:
#include "config.h"

// Includes from librandom:
#include "binomial_randomdev.h"
#include "binomial_randomdev.h"
#include "exp_randomdev.h"
#include "gamma_randomdev.h"
#include "gslrandomgen.h"
#include "knuthlfg.h"
#include "mt19937.h"
#include "normal_randomdev.h"
#include "poisson_randomdev.h"
#include "random_datums.h"
#include "randomdev.h"
#include "randomgen.h"

// Includes from sli:
#include "dict.h"
#include "dictdatum.h"
#include "token.h"
#include "tokenutils.h"

/* Run all available random generators and deviates
   Mean and std dev are computed as a simple test   */

// how many numbers/deviates to create per generator
const unsigned long Ngen = 1000000UL;
const unsigned long Ndev = 1000000UL;
const unsigned long seed = 1234567890UL;

void
printres( double mean, double sdev, double dt )
{
  std::cout << std::setprecision( 4 ) << std::fixed;
  std::cout << "<X> = " << std::setw( 6 ) << std::showpos << mean << std::noshowpos << std::setw( 4 ) << " +- "
            << std::setw( 6 ) << sdev;
  if ( dt >= 0 )
  {
    std::cout << ", dt = " << std::setw( 4 ) << std::setprecision( 0 ) << dt << " ms";
  }

  std::cout << std::endl;
}

// routine running RNG
void
rungen( librandom::RngPtr rng, const unsigned long N )
{
  double sum = 0;
  double sum2 = 0;
  double x;
  std::clock_t t1, t2;

  t1 = std::clock();
  for ( unsigned long k = 0; k < N; k++ )
  {
    x = ( *rng )();
    sum += x;
    sum2 += std::pow( x, 2 );
  }
  t2 = std::clock();
  double dt = double( t2 - t1 ) / CLOCKS_PER_SEC * 1000; // ms

  double mean = sum / N;
  double sdev = std::sqrt( sum2 / N - std::pow( mean, 2 ) );
  printres( mean, sdev, dt );
}

// routine running RND
void
rundev( librandom::RandomDev* rnd, const unsigned long N )
{
  double sum = 0;
  double sum2 = 0;
  double x;
  std::clock_t t1, t2;

  t1 = std::clock();
  for ( unsigned long k = 0; k < N; k++ )
  {
    x = ( *rnd )();
    // std::cout << x << std::endl;
    sum += x;
    sum2 += std::pow( x, 2 );
  }
  t2 = std::clock();
  double dt = double( t2 - t1 ) / CLOCKS_PER_SEC * 1000; // ms

  double mean = sum / N;
  double sdev = std::sqrt( sum2 / N - std::pow( mean, 2 ) );
  printres( mean, sdev, dt );
}

template < typename NumberGenerator >
void
register_rng( const std::string& name, DictionaryDatum& dict )
{
  Token rngfactory = new librandom::RngFactoryDatum( new librandom::BuiltinRNGFactory< NumberGenerator > );
  dict->insert_move( Name( name ), rngfactory );
}


int
main( void )
{
  // create random number generator type dictionary
  Dictionary rngdict;
  DictionaryDatum rngdictd( &rngdict );

  // add non-GSL rngs
  register_rng< librandom::KnuthLFG >( "KnuthLFG", rngdictd );
  register_rng< librandom::MT19937 >( "MT19937", rngdictd );

  // let GslRandomGen add all of the GSL rngs
  librandom::GslRandomGen::add_gsl_rngs( rngdict );

  // run all available RNG
  std::cout << std::endl
            << "===========================================================" << std::endl
            << std::endl;
  std::cout << "Available random generators---Generating " << Ngen << " numbers" << std::endl;
  std::cout << "-----------------------------------------------------------" << std::endl;
  // check all implementations
  for ( Dictionary::const_iterator it = rngdict.begin(); it != rngdict.end(); ++it )
  {
    std::cout << std::left << std::setw( 25 ) << it->first << ": ";

    librandom::RngFactoryDatum fd = getValue< librandom::RngFactoryDatum >( it->second );
    librandom::RngPtr rp = fd->create( librandom::RandomGen::DefaultSeed );
    rungen( rp, Ngen );
  }

  std::cout << std::left << std::setw( 25 ) << "Expected"
            << ": ";
  printres( 0.5, 1.0 / std::sqrt( 12.0 ), -1 );
  std::cout << std::endl
            << "===========================================================" << std::endl;

  // random deviates
  std::cout << std::endl
            << "Available random deviates---Generating " << Ndev << " numbers" << std::endl
            << "-----------------------------------------------------------" << std::endl
            << std::endl;


  // create default generator for deviate generation
  librandom::RngFactoryDatum rngfact = getValue< librandom::RngFactoryDatum >( rngdict.begin()->second );

  librandom::RngPtr lockrng = rngfact->create( librandom::RandomGen::DefaultSeed );

  librandom::RandomDev* rnd;

  // Poisson
  {
    std::cout << std::left << std::setw( 25 ) << "Poisson (lam=1)"
              << " : ";
    lockrng->seed( seed );
    rnd = new librandom::PoissonRandomDev( lockrng, 1 );
    rundev( rnd, Ndev );
    std::cout << std::left << std::setw( 25 ) << "Expected"
              << " : ";
    printres( 1.0, 1.0, -1 );
    std::cout << std::endl;
  }

  // Normal
  {
    std::cout << std::left << std::setw( 25 ) << "Normal"
              << " : ";
    lockrng->seed( seed );
    rnd = new librandom::NormalRandomDev( lockrng );
    rundev( rnd, Ndev );
    std::cout << std::left << std::setw( 25 ) << "Expected"
              << " : ";
    printres( 0.0, 1.0, -1 );
    std::cout << std::endl;
  }

  // Exponential
  {
    std::cout << std::left << std::setw( 25 ) << "Exponential"
              << " : ";
    lockrng->seed( seed );
    rnd = new librandom::ExpRandomDev( lockrng );
    rundev( rnd, Ndev );
    std::cout << std::left << std::setw( 25 ) << "Expected"
              << " : ";
    printres( 1.0, 1.0, -1 );
    std::cout << std::endl;
  }

  // Gamma
  {
    std::cout << std::left << std::setw( 25 ) << "Gamma (Order 4)"
              << " : ";
    lockrng->seed( seed );
    rnd = new librandom::GammaRandomDev( lockrng, 4 );
    rundev( rnd, Ndev );
    std::cout << std::left << std::setw( 25 ) << "Expected"
              << " : ";
    printres( 4.0, 2.0, -1 );
    std::cout << std::endl;
  }

  // Binomial
  {
    std::cout << std::left << std::setw( 25 ) << "Binom (0.25, 8)"
              << " : ";
    lockrng->seed( seed );
    rnd = new librandom::BinomialRandomDev( lockrng, 0.25, 8 );
    rundev( rnd, Ndev );
    std::cout << std::left << std::setw( 25 ) << "Expected"
              << " : ";
    printres( 2.0, 1.2247, -1 );
    std::cout << std::endl;
  }

  std::cout << std::endl
            << "===========================================================" << std::endl;

  return 0;
}
