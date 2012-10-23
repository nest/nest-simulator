/*
 *  binomial_randomdev.cpp
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

#include "binomial_randomdev.h"
#include "dictutils.h"
#include <cmath>

librandom::BinomialRandomDev::BinomialRandomDev(RngPtr r_s, 
						double p_s, 
						unsigned int n_s)
 : RandomDev(r_s), gamma_dev_(r_s), p_(p_s), n_(n_s)
{
  check_params_();
}

librandom::BinomialRandomDev::BinomialRandomDev(double p_s,
						unsigned int n_s)
 : RandomDev(), gamma_dev_(), p_(p_s), n_(n_s)
{
  check_params_();
}

unsigned long librandom::BinomialRandomDev::uldev(RngPtr r_s)
{
  assert(r_s.valid());
  
  int ntmp = n_;
  double ptmp = p_;
  unsigned long X = 0;
  int S = 1;
  
  // avoid problems for pathological case p_ == 1
  if ( p_ == 1 )
    return n_;
    
  // recursion first, see Devroye, Ch X.4, p. 537  
  while ( ntmp * ptmp > 5.0 )
  {
    const int i = (ntmp + 1) * ptmp;
    
    // generate Y as beta(i, n+1-i) deviate, see Devroye, Ch IX.4, p. 432
    const double y1 = gamma_dev_(r_s, static_cast<double>(i));
    const double y2 = gamma_dev_(r_s, static_cast<double>(ntmp+1-i));
    const double Y = y1 / (y1 + y2);
    
    X += S * i;
    
    if ( Y <= ptmp )
    {
      ntmp -= i;
      ptmp = ( ptmp - Y ) / ( 1 - Y );
    }
    else
    { 
      S *= -1;
      ntmp = i - 1;
      ptmp = 1 - ptmp / Y;
    }
  }
  
  // generate directly for rest, see Devroye, Ch X.4. p. 524
  while ( ntmp-- )
    if ( r_s->drand() < ptmp )
      X += S;
      
  return X;
}

void librandom::BinomialRandomDev::set_p_n(double p_s, unsigned int n_s)
{
  p_ = p_s;
  n_ = n_s;
  check_params_();
}

void librandom::BinomialRandomDev::set_p(double p_s)
{
  p_ = p_s;
  check_params_();
}

void librandom::BinomialRandomDev::set_n(unsigned int n_s)
{
  n_ = n_s;
  check_params_();
} 

void librandom::BinomialRandomDev::check_params_()
{
  assert( 0.0 <= p_ && p_ <= 1.0 );
}

void librandom::BinomialRandomDev::set_status(const DictionaryDatum &d)
{
  double p_tmp;
  if (  updateValue<double>(d, "p", p_tmp) )
    set_p(p_tmp);

  long n_tmp;
  if (  updateValue<long>(d, "n", n_tmp) )
    set_n(n_tmp);
} 

void librandom::BinomialRandomDev::get_status(DictionaryDatum &d) const 
{
  def<double>(d, "p", p_);
  def<long>(d, "n", n_);
}
