/*
 *  exp_randomdev.h
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

#ifndef EXP_RANDOMDEV_H
#define EXP_RANDOMDEV_H

#include <cmath>
#include "randomgen.h"
#include "randomdev.h"
#include "lockptr.h"

namespace librandom {

/*BeginDocumentation
Name: rdevdict::exponential - exponential random deviate generator
Description: Generates exponentially distributed random numbers.

  p(x) = exp(-x), x >= 0.     
    
Parameters: No parameters.

SeeAlso: CreateRDV, RandomArray, rdevdict
Author: Hans Ekkehard Plesser
*/

/**
 * Class ExpRandomDev Create exponential random numbers
 *
 * @ingroup RandomDeviateGenerators
 */

  class ExpRandomDev : public RandomDev
  {

  public:

    // accept only lockPTRs for initialization,
    // otherwise creation of a lock ptr would 
    // occur as side effect---might be unhealthy
    ExpRandomDev(RngPtr r_in) : RandomDev(r_in) {} ;
    ExpRandomDev() : RandomDev() {} ;                // threaded

    double operator()(void);           // non-threaded
    double operator()(RngPtr rthrd) const;   // threaded

    //! set distribution parameters from SLI dict
    void set_status(const DictionaryDatum&) {} 

    //! get distribution parameters from SLI dict
    void get_status(DictionaryDatum&) const {} 

  };

  inline
  double ExpRandomDev::operator()(void)
  {
    return -std::log(rng_->drandpos());
  }

  inline
  double ExpRandomDev::operator()(RngPtr rthrd) const
  {
    return -std::log(rthrd->drandpos());
  }

}

# endif
