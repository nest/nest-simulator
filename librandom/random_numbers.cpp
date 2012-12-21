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

#include "config.h"
#include "dict.h"
#include "dictdatum.h"
#include "random_numbers.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "arraydatum.h"
#include "lockptrdatum_impl.h"
#include "tokenutils.h"
#include "sliexceptions.h"

#include "random_datums.h"
#include "knuthlfg.h"
#include "mt19937.h"
#include "gslrandomgen.h"

#include "binomial_randomdev.h"
#include "poisson_randomdev.h"
#include "normal_randomdev.h"
#include "exp_randomdev.h"
#include "gamma_randomdev.h"
#include "uniformint_randomdev.h"

#ifdef HAVE_GSL
#include "gsl_binomial_randomdev.h"
#endif

SLIType RandomNumbers::RngType;
SLIType RandomNumbers::RngFactoryType;
SLIType RandomNumbers::RdvType;
SLIType RandomNumbers::RdvFactoryType;


template class lockPTRDatum<librandom::RandomGen, &RandomNumbers::RngType>;
template class lockPTRDatum<librandom::RandomDev, &RandomNumbers::RdvType>;
template class lockPTRDatum<librandom::GenericRandomDevFactory, &RandomNumbers::RdvFactoryType>;


RandomNumbers::~RandomNumbers()
{
  RngType.deletetypename();
  RngFactoryType.deletetypename();

  RdvType.deletetypename();
  RdvFactoryType.deletetypename();
}

template <typename NumberGenerator>
void RandomNumbers::register_rng_(const std::string& name, DictionaryDatum& dict)
{
  Token rngfactory = new librandom::RngFactoryDatum(
                         new librandom::BuiltinRNGFactory<NumberGenerator>);
  dict->insert_move(Name(name), rngfactory);
}

template <typename DeviateGenerator>
void RandomNumbers::register_rdv_(const std::string& name, DictionaryDatum& dict)
{
  Token rdevfactory = new librandom::RdvFactoryDatum(
                          new librandom::RandomDevFactory<DeviateGenerator>);
  dict->insert_move(Name(name), rdevfactory);
}

void RandomNumbers::init(SLIInterpreter *i)
{
  RngType.settypename("rngtype");
  RngType.setdefaultaction(SLIInterpreter::datatypefunction);

  RngFactoryType.settypename("rngfactorytype");
  RngFactoryType.setdefaultaction(SLIInterpreter::datatypefunction);

  RdvType.settypename("rdvtype");
  RdvType.setdefaultaction(SLIInterpreter::datatypefunction);
  RdvFactoryType.settypename("rdvfactorytype");
  RdvFactoryType.setdefaultaction(SLIInterpreter::datatypefunction);

  // create random number generator type dictionary
  DictionaryDatum rngdict(new Dictionary());
  i->def("rngdict", rngdict);
  
  // add built-in rngs
  register_rng_<librandom::KnuthLFG>("knuthlfg", rngdict);
  register_rng_<librandom::MT19937>("MT19937", rngdict);

  // let GslRandomGen add all of the GSL rngs
  librandom::GslRandomGen::add_gsl_rngs(rngdict);

  // create random deviate generator dictionary
  DictionaryDatum rdvdict(new Dictionary());
  i->def("rdevdict", rdvdict);

  register_rdv_<librandom::BinomialRandomDev>("binomial", rdvdict);
  register_rdv_<librandom::PoissonRandomDev>("poisson", rdvdict);
  register_rdv_<librandom::NormalRandomDev>("normal", rdvdict);
  register_rdv_<librandom::ExpRandomDev>("exponential", rdvdict);
  register_rdv_<librandom::GammaRandomDev>("gamma", rdvdict);
  register_rdv_<librandom::UniformIntRandomDev>("uniformint", rdvdict);

#ifdef HAVE_GSL
  register_rdv_<librandom::GSL_BinomialRandomDev>("gsl_binomial", rdvdict);
#endif

  // create function
  i->createcommand("CreateRNG_gt_i", &createrngfunction);
  i->createcommand("CreateRDV_g_vf", &createrdvfunction);

  i->createcommand("SetStatus_v", &setstatus_vdfunction);
  i->createcommand("GetStatus_v",  &getstatus_vfunction);
  
  // access functions
  i->createcommand("seed_g_i",&seedfunction);
  i->createcommand("irand_g_i",&irandfunction);
  i->createcommand("drand_g",&drandfunction);

  i->createcommand("RandomArray_v_i", &randomarrayfunction);
  i->createcommand("Random_i", &randomfunction);
}

// see librandom.sli for SLI documentation
void RandomNumbers::CreateRNGFunction::execute(SLIInterpreter *i) const
{
  assert(i->OStack.load()>1);

  const long seed = getValue<long>(i->OStack.top());
  librandom::RngFactoryDatum factory 
    = getValue<librandom::RngFactoryDatum>(i->OStack.pick(1));  

  librandom::RngDatum rng( factory->create(seed) );

  i->OStack.pop(2);
  i->OStack.push(rng);
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void RandomNumbers::CreateRDVFunction::execute(SLIInterpreter *i) const
{
  assert(i->OStack.load()>1);

  librandom::RdvFactoryDatum factory 
    = getValue<librandom::RdvFactoryDatum>(i->OStack.top());
  librandom::RngDatum rng = getValue<librandom::RngDatum>(i->OStack.pick(1));

  librandom::RdvDatum rdv( factory->create(rng) );
			
  i->OStack.pop(2);
  i->OStack.push(rdv);
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void RandomNumbers::SetStatus_vdFunction::execute(SLIInterpreter *i) const
{
  assert(i->OStack.load() > 1);

  DictionaryDatum dict;
  librandom::RdvDatum rdv;

  try 
    {
      dict = getValue<DictionaryDatum>(i->OStack.top());
      rdv = getValue<librandom::RdvDatum>(i->OStack.pick(1));
    }
  catch ( TypeMismatch &e )
    {
      i->raiseerror("TypeMismatch");
      return;
    }

  try
    {
      rdv->set_status(dict);
      i->OStack.pop(2);
      i->EStack.pop();
    }
  catch(...)
    {
      /* This is not nice.  Before we improve on this,
	 we should unify the exception handling in SLI an the kernel,
	 so that the above set_status can throw a nest::KernelException,
	 which can be queried here with e.what().
	 HEP, 2004-08-05
      */
      i->raiseerror(i->BadErrorHandler);
      return;
    }
}

// see librandom.sli for SLI documentation
void RandomNumbers::GetStatus_vFunction::execute(SLIInterpreter *i) const
{
  
  assert(i->OStack.load() > 0);

  librandom::RdvDatum rdv;
  try
    {
      rdv = getValue<librandom::RdvDatum>(i->OStack.top());
    }
  catch ( TypeMismatch &e )
    {
      i->raiseerror("TypeMismatch");
      return;
    }
  
  try
    {
      DictionaryDatum dict(new Dictionary);
      assert(dict.valid());

      rdv->get_status(dict);

      i->OStack.pop();
      i->OStack.push(dict);
      i->EStack.pop();
    }
  catch(...)
    {
      /* This is not nice.  Before we improve on this,
	 we should unify the exception handling in SLI an the kernel,
	 so that the above set_status can throw a nest::KernelException,
	 which can be queried here with e.what().
	 HEP, 2004-08-05
      */
      i->raiseerror(i->BadErrorHandler);
      return;
    }
}

// see librandom.sli for SLI documentation
void RandomNumbers::SeedFunction::execute(SLIInterpreter *i) const 
{
  assert(i->OStack.load()>1);

  const long seed = getValue<long>(i->OStack.top());
  librandom::RngDatum   rng  = getValue<librandom::RngDatum>(i->OStack.pick(1));

  rng->seed(seed);

  i->OStack.pop(2);
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void RandomNumbers::IrandFunction::execute(SLIInterpreter *i) const 
{
  assert(i->OStack.load()>1);

  const long N = getValue<long>(i->OStack.top());
  librandom::RngDatum rng = getValue<librandom::RngDatum>(i->OStack.pick(1));

  const unsigned long r = rng->ulrand(N);

  i->OStack.pop(2);
  i->OStack.push(r);
  i->EStack.pop();
}

// see librandom.sli for SLI documentation
void RandomNumbers::DrandFunction::execute(SLIInterpreter *i) const 
{
  assert(i->OStack.load()>0);

  librandom::RngDatum rng = getValue<librandom::RngDatum>(i->OStack.top());

  const double r = rng->drand();

  i->OStack.pop();
  i->OStack.push(r);
  i->EStack.pop();
}


/* see librandom.sli for SLI documentation */
void RandomNumbers::RandomArrayFunction::execute(SLIInterpreter *i) const
{
  if( i->OStack.load() < 2 )
  {
    i->message(SLIInterpreter::M_ERROR, "RandomArray","Too few parameters supplied.");
    i->message(SLIInterpreter::M_ERROR, "RandomArray","Usage: rdv n RandomArray.");
    i->raiseerror(i->StackUnderflowError);
    return;
  }

  librandom::RdvDatum rdv = getValue<librandom::RdvDatum>(i->OStack.pick(1));
  const long n = getValue<long>(i->OStack.pick(0));

  TokenArray result;
  result.reserve(n);

  if ( rdv->has_uldev() )
    for( long j = 0; j < n ; ++j)
      result.push_back(rdv->uldev());
  else
    for( long j=0; j<n ; ++j)
      result.push_back( (*rdv)() );
  
  i->OStack.pop(2);
  i->OStack.push(ArrayDatum(result));
  i->EStack.pop();
}


void RandomNumbers::RandomFunction::execute(SLIInterpreter *i) const
{
  if( i->OStack.load()<1 )
  {
    i->raiseerror(i->StackUnderflowError);
    return;
  }

  librandom::RdvDatum rdv = getValue<librandom::RdvDatum>(i->OStack.top());
 
  i->OStack.pop(); 

  if ( rdv->has_uldev() )
    i->OStack.push( rdv->uldev() );
  else
    i->OStack.push( (*rdv)() );
  
  i->EStack.pop();
}




