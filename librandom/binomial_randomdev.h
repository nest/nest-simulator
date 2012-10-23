/*
 *  binomial_randomdev.h
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

#ifndef BINOMIAL_RANDOMDEV_H
#define  BINOMIAL_RANDOMDEV_H

#include <cmath>
#include <vector>
#include "randomgen.h"
#include "randomdev.h"
#include "gamma_randomdev.h"
#include "lockptr.h"
#include "dictdatum.h"



/*BeginDocumentation
Name: rdevdict::binomial - binomial random deviate generator
Description:
   Generates binomially distributed random numbers.

   p(k) = (n! / k!(n-k)!) p^k (1-p)^(n-k)  , 0<=k<=n, n>0   
    
Parameters:
   p - probability of success in a single trial (double)
   n - number of trials (positive integer)

SeeAlso: CreateRDV, RandomArray, rdevdict
Author: Hans Ekkehard Plesser
*/


namespace librandom {

/**
 Class BinomialRNG                                        
                                                          
 Generates an RNG which returns Binomial(k;p;n)           
 distributed random numbers out of an RNG which returns   
 binomially distributed random numbers:                   
                                                          
    p(k) = (n! / k!(n-k)!) p^k (1-p)^(n-k)  , 0<=k<=n, n<0   
                                                          
 Arguments:                                               
  - pointer to an RNG                                     
  - parameter p (optional, default = 1)                   
  - parameter n (optional, defautl = 1)                                       
                                 
 @see Devroye, Non-Uniform Random ..., Ch X.4., p 537                                 
 @ingroup RandomDeviateGenerators
*/

class BinomialRandomDev : public RandomDev
  {
  public:
    // accept only lockPTRs for initialization,
    // otherwise creation of a lock ptr would
    // occur as side effect---might be unhealthy
    BinomialRandomDev(RngPtr, double p_s = 0.5, unsigned int n_s=1);
    BinomialRandomDev(double p_s = 0.5, unsigned int n_s=1);
   
    /** 
     * set parameters for p and n 
     * @parameters
     * p - success probability for single trial
     * n - number of trials
     */
    void   set_p_n   (double, unsigned int);   
    void   set_p     (double);                 //!<set p
    void   set_n     (unsigned int);           //!<set n  

    /**
     * Import sets of overloaded virtual functions.
     * We need to explicitly include sets of overloaded
     * virtual functions into the current scope.
     * According to the SUN C++ FAQ, this is the correct
     * way of doing things, although all other compilers
     * happily live without.
     */
    using RandomDev::uldev;
    
    unsigned long uldev(void);     //!< draw integer
    unsigned long uldev(RngPtr);   //!< draw integer, threaded
    bool has_uldev() const { return true; }

    double operator()(void);       //!< return as double
    double operator()(RngPtr);     //!< return as double, threaded

    //! set distribution parameters from SLI dict
    void set_status(const DictionaryDatum&); 

    //! get distribution parameters from SLI dict
    void get_status(DictionaryDatum&) const; 


  private:
    GammaRandomDev gamma_dev_;  //!< source of gamma distributed numbers
    double p_;                  //!<probability p of binomial distribution
    unsigned int n_;            //!<parameter n in binomial distribution

    void check_params_();       //!< check internal parameters
   
  };

  inline
  double BinomialRandomDev::operator()(void)
  { 
    return static_cast<double>(uldev()); 
  }

  inline
  double BinomialRandomDev::operator()(RngPtr rthrd)
  { 
    return static_cast<double>(uldev(rthrd)); 
  }

  inline
  unsigned long BinomialRandomDev::uldev(void)
  {
    return uldev(rng_);
  }

}

#endif

