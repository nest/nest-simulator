/*
 * Copyright (C) 2012 The NEST Initiative
 * This file is part of NEST.
 */

#include "gsl_binomial_randomdev.h"

#ifdef HAVE_GSL

#include "dictutils.h"

librandom::GSL_BinomialRandomDev::GSL_BinomialRandomDev(RngPtr r_s, double p_s, unsigned int n_s)
        : RandomDev(r_s), p_(p_s), n_(n_s)
{
  GslRandomGen* gsr_rng = dynamic_cast<GslRandomGen*>(&(*r_s));
  assert (gsr_rng && "r_s needs to be a GSL RNG");
  rng_ = gsr_rng->rng_;
}

librandom::GSL_BinomialRandomDev::GSL_BinomialRandomDev(double p_s, unsigned int n_s)
        : RandomDev(), p_(p_s), n_(n_s)
{}

unsigned long librandom::GSL_BinomialRandomDev::uldev()
{
  return gsl_ran_binomial(rng_, p_, n_);
}

unsigned long librandom::GSL_BinomialRandomDev::uldev(RngPtr rng) const
{
  GslRandomGen* gsr_rng = dynamic_cast<GslRandomGen*>(&(*rng));
  assert (gsr_rng && "rng needs to be a GSL RNG");
  return gsl_ran_binomial(gsr_rng->rng_, p_, n_);
}

void librandom::GSL_BinomialRandomDev::set_p(double p_s)
{
  assert( 0.0 <= p_ && p_ <= 1.0 );
  p_ = p_s;
}

void librandom::GSL_BinomialRandomDev::set_n(unsigned int n_s)
{
  n_ = n_s;
}

void librandom::GSL_BinomialRandomDev::set_status(const DictionaryDatum &d)
{
  double p_tmp;
  if (  updateValue<double>(d, "p", p_tmp) )
    set_p(p_tmp);

  long n_tmp;
  if (  updateValue<long>(d, "n", n_tmp) )
    set_n(n_tmp);
}

void librandom::GSL_BinomialRandomDev::get_status(DictionaryDatum &d) const
{
  def<double>(d, "p", p_);
  def<long>(d, "n", n_);
}

#endif
