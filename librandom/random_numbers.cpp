/*
 *  random_numbers.cpp
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

#include "random_numbers.h"

// Generated includes:
#include "config.h"

// Includes from librandom:
#include "binomial_randomdev.h"
#include "clipped_randomdev.h"
#include "exp_randomdev.h"
#include "gamma_randomdev.h"
#include "gslrandomgen.h"
#include "knuthlfg.h"
#include "lognormal_randomdev.h"
#include "mt19937.h"
#include "normal_randomdev.h"
#include "poisson_randomdev.h"
#include "random.h"
#include "random_datums.h"
#include "uniform_randomdev.h"
#include "uniformint_randomdev.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictdatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "sharedptrdatum.h"
#include "sliexceptions.h"
#include "tokenutils.h"

#ifdef HAVE_GSL
#include "gsl_binomial_randomdev.h"
#endif

SLIType RandomNumbers::RngType;
SLIType RandomNumbers::RngFactoryType;
SLIType RandomNumbers::RdvType;
SLIType RandomNumbers::RdvFactoryType;


template class sharedPtrDatum< librandom::RandomGen, &RandomNumbers::RngType >;
template class sharedPtrDatum< librandom::RandomDev, &RandomNumbers::RdvType >;
template class sharedPtrDatum< librandom::GenericRandomDevFactory, &RandomNumbers::RdvFactoryType >;

Dictionary* RandomNumbers::rngdict_ = 0;
Dictionary* RandomNumbers::rdvdict_ = 0;

RandomNumbers::~RandomNumbers()
{
  RngType.deletetypename();
  RngFactoryType.deletetypename();

  RdvType.deletetypename();
  RdvFactoryType.deletetypename();
}

template < typename NumberGenerator >
void
RandomNumbers::register_rng_( const std::string& name, Dictionary& dict )
{
  Token rngfactory = new librandom::RngFactoryDatum( new librandom::BuiltinRNGFactory< NumberGenerator > );
  dict[ Name( name ) ] = rngfactory;
}

template < typename DeviateGenerator >
void
RandomNumbers::register_rdv_( const std::string& name, Dictionary& dict )
{
  Token rdevfactory = new librandom::RdvFactoryDatum( new librandom::RandomDevFactory< DeviateGenerator > );
  dict.insert_move( Name( name ), rdevfactory );
}

void
RandomNumbers::init( SLIInterpreter* i )
{
  RngType.settypename( "rngtype" );
  RngType.setdefaultaction( SLIInterpreter::datatypefunction );

  RngFactoryType.settypename( "rngfactorytype" );
  RngFactoryType.setdefaultaction( SLIInterpreter::datatypefunction );

  RdvType.settypename( "rdvtype" );
  RdvType.setdefaultaction( SLIInterpreter::datatypefunction );
  RdvFactoryType.settypename( "rdvfactorytype" );
  RdvFactoryType.setdefaultaction( SLIInterpreter::datatypefunction );
  if ( rngdict_ || rdvdict_ )
  {
    throw DynamicModuleManagementError( "RandomNumbers module has been initialized previously." );
  }

  // create random number generator type dictionary
  rngdict_ = new Dictionary();
  assert( rngdict_ );
  i->def( "rngdict", DictionaryDatum( rngdict_ ) );

  // add built-in rngs
  register_rng_< librandom::KnuthLFG >( "knuthlfg", *rngdict_ );
  register_rng_< librandom::MT19937 >( "MT19937", *rngdict_ );

  // let GslRandomGen add all of the GSL rngs
  librandom::GslRandomGen::add_gsl_rngs( *rngdict_ );

  // create random deviate generator dictionary
  rdvdict_ = new Dictionary();
  assert( rdvdict_ );
  i->def( "rdevdict", DictionaryDatum( rdvdict_ ) );

  register_rdv_< librandom::BinomialRandomDev >( "binomial", *rdvdict_ );
  register_rdv_< librandom::ClippedRedrawDiscreteRandomDev< librandom::BinomialRandomDev > >(
    "binomial_clipped", *rdvdict_ );
  register_rdv_< librandom::ClippedToBoundaryDiscreteRandomDev< librandom::BinomialRandomDev > >(
    "binomial_clipped_to_boundary", *rdvdict_ );
  register_rdv_< librandom::PoissonRandomDev >( "poisson", *rdvdict_ );
  register_rdv_< librandom::ClippedRedrawDiscreteRandomDev< librandom::PoissonRandomDev > >(
    "poisson_clipped", *rdvdict_ );
  register_rdv_< librandom::ClippedToBoundaryDiscreteRandomDev< librandom::PoissonRandomDev > >(
    "poisson_clipped_to_boundary", *rdvdict_ );
  register_rdv_< librandom::UniformRandomDev >( "uniform", *rdvdict_ );
  register_rdv_< librandom::UniformIntRandomDev >( "uniform_int", *rdvdict_ );

  register_rdv_< librandom::NormalRandomDev >( "normal", *rdvdict_ );
  register_rdv_< librandom::ClippedRedrawContinuousRandomDev< librandom::NormalRandomDev > >(
    "normal_clipped", *rdvdict_ );
  register_rdv_< librandom::ClippedToBoundaryContinuousRandomDev< librandom::NormalRandomDev > >(
    "normal_clipped_to_boundary", *rdvdict_ );
  register_rdv_< librandom::LognormalRandomDev >( "lognormal", *rdvdict_ );
  register_rdv_< librandom::ClippedRedrawContinuousRandomDev< librandom::LognormalRandomDev > >(
    "lognormal_clipped", *rdvdict_ );
  register_rdv_< librandom::ClippedToBoundaryContinuousRandomDev< librandom::LognormalRandomDev > >(
    "lognormal_clipped_to_boundary", *rdvdict_ );

  register_rdv_< librandom::ExpRandomDev >( "exponential", *rdvdict_ );
  register_rdv_< librandom::ClippedRedrawContinuousRandomDev< librandom::ExpRandomDev > >(
    "exponential_clipped", *rdvdict_ );
  register_rdv_< librandom::ClippedToBoundaryContinuousRandomDev< librandom::ExpRandomDev > >(
    "exponential_clipped_to_boundary", *rdvdict_ );
  register_rdv_< librandom::GammaRandomDev >( "gamma", *rdvdict_ );
  register_rdv_< librandom::ClippedRedrawContinuousRandomDev< librandom::GammaRandomDev > >(
    "gamma_clipped", *rdvdict_ );
  register_rdv_< librandom::ClippedToBoundaryContinuousRandomDev< librandom::GammaRandomDev > >(
    "gamma_clipped_to_boundary", *rdvdict_ );

#ifdef HAVE_GSL
  register_rdv_< librandom::GSL_BinomialRandomDev >( "gsl_binomial", *rdvdict_ );
#endif

  // create function
  i->createcommand( "CreateRNG_gt_i", &createrngfunction );
  i->createcommand( "CreateRDV_g_vf", &createrdvfunction );

  i->createcommand( "SetStatus_v", &setstatus_vdfunction );
  i->createcommand( "GetStatus_v", &getstatus_vfunction );

  // access functions
  i->createcommand( "seed_g_i", &seedfunction );
  i->createcommand( "irand_g_i", &irandfunction );
  i->createcommand( "drand_g", &drandfunction );

  i->createcommand( "RandomArray_v_i", &randomarrayfunction );
  i->createcommand( "Random_i", &randomfunction );
}

// see librandom.sli for SLI documentation
void
RandomNumbers::CreateRNGFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const long seed = getValue< long >( i->OStack.top() );
  librandom::RngFactoryDatum factory = getValue< librandom::RngFactoryDatum >( i->OStack.pick( 1 ) );

  librandom::RngDatum rng = librandom::create_rng( seed, factory );

  i->OStack.pop( 2 );
  i->OStack.push( rng );
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void
RandomNumbers::CreateRDVFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  librandom::RdvFactoryDatum factory = getValue< librandom::RdvFactoryDatum >( i->OStack.top() );
  librandom::RngDatum rng = getValue< librandom::RngDatum >( i->OStack.pick( 1 ) );

  librandom::RdvDatum rdv = librandom::create_rdv( factory, rng );

  i->OStack.pop( 2 );
  i->OStack.push( rdv );
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void
RandomNumbers::SetStatus_vdFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum dict = getValue< DictionaryDatum >( i->OStack.top() );
  librandom::RdvDatum rdv = getValue< librandom::RdvDatum >( i->OStack.pick( 1 ) );

  librandom::set_status( dict, rdv );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void
RandomNumbers::GetStatus_vFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  librandom::RdvDatum rdv = getValue< librandom::RdvDatum >( i->OStack.top() );

  DictionaryDatum dict = librandom::get_status( rdv );

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void
RandomNumbers::SeedFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const long seed = getValue< long >( i->OStack.top() );
  librandom::RngDatum rng = getValue< librandom::RngDatum >( i->OStack.pick( 1 ) );

  librandom::seed( seed, rng );

  i->OStack.pop( 2 );
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void
RandomNumbers::IrandFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const long N = getValue< long >( i->OStack.top() );
  librandom::RngDatum rng = getValue< librandom::RngDatum >( i->OStack.pick( 1 ) );

  const unsigned long r = librandom::irand( N, rng );

  i->OStack.pop( 2 );
  i->OStack.push( r );
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void
RandomNumbers::DrandFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  librandom::RngDatum rng = getValue< librandom::RngDatum >( i->OStack.top() );

  const double r = librandom::drand( rng );

  i->OStack.pop();
  i->OStack.push( r );
  i->EStack.pop();
}


/* see librandom.sli for SLI documentation */
void
RandomNumbers::RandomArrayFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  librandom::RdvDatum rdv = getValue< librandom::RdvDatum >( i->OStack.pick( 1 ) );
  const long n = getValue< long >( i->OStack.pick( 0 ) );

  ArrayDatum result = librandom::random_array( rdv, n );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}


void
RandomNumbers::RandomFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  librandom::RdvDatum rdv = getValue< librandom::RdvDatum >( i->OStack.top() );

  i->OStack.pop();
  Token result = librandom::random( rdv );

  i->OStack.push( result );
  i->EStack.pop();
}
