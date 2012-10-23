/*
 *  parameters.cpp
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
  This file is part of the NEST topology module.
  Author: Kittel Austvoll
*/

#include "parameters.h"

#include "dictutils.h"
#include "randomgen.h"
#include "nestmodule.h"

#include <limits>
#include <cmath>

namespace nest
{

  /***********************************************************************/
  /*                           BASE PARAMETER                            */
  /***********************************************************************/

  Parameters::Parameters():
    min_(std::numeric_limits<double_t>::max()*-1),
    max_(std::numeric_limits<double_t>::max()),
    cutoff_(std::numeric_limits<double_t>::max()*-1),
    cutoff_distance_(std::numeric_limits<double_t>::max()),
    anchor_(),
    k_(1.0)
  {}
  
  Parameters::Parameters(double_t k):
    min_(std::numeric_limits<double_t>::max()*-1),
    max_(std::numeric_limits<double_t>::max()),
    cutoff_(std::numeric_limits<double_t>::max()*-1),
    cutoff_distance_(std::numeric_limits<double_t>::max()),
    anchor_(),
    k_(k)
  {}

  Parameters::Parameters(const DictionaryDatum& settings):
    min_(std::numeric_limits<double_t>::max()*-1),
    max_(std::numeric_limits<double_t>::max()),
    cutoff_(std::numeric_limits<double_t>::max()*-1),
    cutoff_distance_(std::numeric_limits<double_t>::max()),
    anchor_(),
    k_(1.0)
  {
    updateValue<double_t>(settings, "min", min_);
    updateValue<double_t>(settings, "max", max_);

    if(max_ < min_)
      {
	throw EntryTypeMismatch("min <= max", "max < min");
      }

    updateValue<double_t>(settings, "cutoff", cutoff_);
    updateValue<double_t>(settings, "cutoff_distance", 
				cutoff_distance_);

    if(cutoff_distance_ < 0)
      {
	throw EntryTypeMismatch("cutoff_distance >= 0",
				"cutoff_distance < 0");
      }
    
    updateValue<std::vector<double_t> >(settings, "anchor", anchor_);
  }

  double_t Parameters::get_value(const Position<double_t>&) const
  {
    return k_;
  }

  double_t 
  Parameters::get_value(const Position<double_t>& driver,
			const Position<double_t>& pool,
                        std::vector<double_t>* extent) const
  {
    Position<double_t> displacement = driver - pool;
    if (anchor_.size()>0)
    {
      displacement += Position<double_t>(anchor_);
    }

    if (extent)
    {
      displacement.wrap_displacement_max_half(*extent);
    }

    if(displacement.length() > cutoff_distance_)
    {
      return 0.0;
    }

    return bound(get_value(displacement));
  }

  double_t Parameters::bound(double_t value) const
  {
    if(value < cutoff_)
      {
	return 0.0;
      }
    else if(value < min_)
      {
	return min_;
      }
    else if(value > max_)
      {
	return max_;
      }
    
    return value;
  }

  void Parameters::force_positive()
  {
    if(min_ < 0.0)
      {
	min_ = 0.0;
      }
  }

  Parameters* 
  Parameters::create_parameter(const DictionaryDatum& settings)
  {
    if(settings->known("gaussian"))
      {
	return new Gaussian(getValue<DictionaryDatum>(settings, 
						      "gaussian"));
      }
    else if(settings->known("gaussian2D"))
      {
	return new Gaussian2D(getValue<DictionaryDatum>(settings, 
							"gaussian2D"));
      }
    else if(settings->known("linear"))
      {
	return new Linear(getValue<DictionaryDatum>(settings, 
						    "linear"));
      }
    else if(settings->known("exponential"))
      {
	return new Exponential(getValue<DictionaryDatum>(settings, 
							 "exponential"));
      }
    else if(settings->known("uniform"))
      {
	return new Uniform(getValue<DictionaryDatum>(settings, 
						     "uniform"));
      }
    else if(settings->known("combination"))
      {
	return new 
	  Combination(getValue<TokenArray>(settings, 
					   "combination"));
      }
    else
      {
	throw TypeMismatch("parameter class",
			   "something else");
      }
  }

  /***********************************************************************/
  /*                            GAUSSIAN                                 */
  /***********************************************************************/

  Gaussian::Gaussian():
    c_(0.0),
    p_center_(1.0),
    mean_(0.0),
    sigma_(1.0)
  {}
  
  Gaussian::Gaussian(const DictionaryDatum& settings):
    Parameters(settings),
    c_(0.0),
    p_center_(1.0),
    mean_(0.0),
    sigma_(1.0)
  {
    updateValue<double_t>(settings, "c", c_);
    updateValue<double_t>(settings, "p_center", p_center_);
    updateValue<double_t>(settings, "mean", mean_);
    updateValue<double_t>(settings, "sigma", sigma_);

//     double_t std_devitation = 0.0;
//     updateValue<double_t>(settings, "cutoff_distance_std",
// 				std_deviation);
//     if(std_deviation != 0.0)
//       {
// 	cutoff_distance_ = //??
//       }
  }

  double_t Gaussian::get_value(const Position<double_t>& pos) const
  {
    return c_ + p_center_*
      std::exp(-std::pow(pos.length() - mean_,2)/(2*std::pow(sigma_,2)));
  }

  /***********************************************************************/
  /*                          BIVARIATE GAUSSIAN                         */
  /***********************************************************************/

  Gaussian2D::Gaussian2D():
    c_(0.0),
    p_center_(1.0),
    mean_x_(0.0),
    sigma_x_(1.0),
    mean_y_(0.0),
    sigma_y_(1.0),
    rho_(0.0)

  {}

  Gaussian2D::Gaussian2D(const DictionaryDatum& settings):
    Parameters(settings),
    c_(0.0),
    p_center_(1.0),
    mean_x_(0.0),
    sigma_x_(1.0),
    mean_y_(0.0),
    sigma_y_(1.0),
    rho_(0.0)
  {
    updateValue<double_t>(settings, "c", c_);
    updateValue<double_t>(settings, "p_center", p_center_);
    updateValue<double_t>(settings, "mean_x", mean_x_);
    updateValue<double_t>(settings, "sigma_x", sigma_x_);
    updateValue<double_t>(settings, "mean_y", mean_y_);
    updateValue<double_t>(settings, "sigma_y", sigma_y_);
    updateValue<double_t>(settings, "rho", rho_);

    if(rho_ > 1.0 || rho_ < -1.0)
      {
	throw TypeMismatch("rho between -1.0 and 1.0",
			   "something else");
      }
    if(sigma_x_ < 0.0 || sigma_y_ < 0.0)
      {
	throw TypeMismatch("sigma above 0","sigma below 0");
      }
  }

  double_t Gaussian2D::get_value(const Position<double_t>& pos) const
  {
    return c_ + 
      p_center_*std::exp(- (  (pos.get_x()-mean_x_)*(pos.get_x()-mean_x_)/(sigma_x_*sigma_x_)
                            + (pos.get_y()-mean_y_)*(pos.get_y()-mean_y_)/(sigma_y_*sigma_y_)
                            - 2.*rho_*(pos.get_x()-mean_x_)*(pos.get_y()-mean_y_)/(sigma_x_*sigma_y_) )
                          /(2.*(1.-rho_*rho_)) );

//     Position<double_t> temp = (pos - mean)/sigma;

//     return c_ + p_center_*std::exp( - ((temp*temp).get_x()+(temp*temp).get_y()-2*rho_*temp.get_x()*temp.get_y())/(2*(1-rho_*rho_)));
  }

  /***********************************************************************/
  /*                              LINEAR                                 */
  /***********************************************************************/

  Linear::Linear():
    a_(1.0),
    c_(0.0)
  {}
  
  Linear::Linear(const DictionaryDatum& settings):
    Parameters(settings),
    a_(1.0),
    c_(0.0)
  {
    updateValue<double_t>(settings, "a", a_);
    updateValue<double_t>(settings, "c", c_);
  }

  double_t Linear::get_value(const Position<double_t>& pos) const
  {
    return a_*pos.length() + c_;
  }

  /***********************************************************************/
  /*                           EXPONENTIAL                               */
  /***********************************************************************/

  Exponential::Exponential():
    a_(1.0),
    c_(0.0),
    tau_(1.0)
  {}
  
  Exponential::Exponential(const DictionaryDatum& settings):
    Parameters(settings),
    a_(1.0),
    c_(0.0),
    tau_(1.0)
  {
    updateValue<double_t>(settings, "a", a_);
    updateValue<double_t>(settings, "c", c_);
    updateValue<double_t>(settings, "tau", tau_);
  }
  
  double_t Exponential::get_value(const Position<double_t>& pos) const
  {
    return c_ + a_*std::exp(-pos.length()/tau_);
  }
  
  /***********************************************************************/
  /*                            UNIFORM                                  */
  /***********************************************************************/

  Uniform::Uniform():
    range_(0.0),
    lower_(0.0)
  {
    rng_ = NestModule::get_network().get_grng();
  }
  
  Uniform::Uniform(const DictionaryDatum& settings):
    Parameters(settings),
    range_(0.0),
    lower_(0.0)    
  {
    lower_ = getValue<double_t>(settings, "min");
	
    range_ = 
      getValue<double_t>(settings, "max") - lower_;

    rng_ = NestModule::get_network().get_grng();
  }
  
  double_t Uniform::get_value(const Position<double_t>&) const
  {
    return lower_ + rng_->drand()*range_;
  }
  
  /***********************************************************************/
  /*                             DISCRETE                                */
  /***********************************************************************/

  Discrete::Discrete():
    values_()
  {}
  
  Discrete::Discrete(std::vector<double_t> values):
    values_(values)
  {}

  double_t Discrete::get_value(const Position<double_t>& lid) const
  {
    if(lid.get_x() < values_.size())
      {
	return values_.at(lid.get_x());
      }

    return 1.0;
  }

  double_t 
  Discrete::get_value(const Position<double_t>&,
		      const Position<double_t>& lid,
		      std::vector<double_t>* extent) const
  {
    return bound(get_value(lid.get_x()));
  }
  
  /***********************************************************************/
  /*                            COMBINATION                              */
  /***********************************************************************/

  Combination::Combination():
    parameters_list_()
  {}
  
  Combination::Combination(const TokenArray& settings):
    Parameters(),
    parameters_list_()
  {
    // Add parameter objects to Combination object.
    for(uint_t i = 0; i != settings.size(); ++i)
      {
	parameters_list_.
	  push_back(create_parameter(getValue<DictionaryDatum>(settings[i])));
      }
  }

  void Combination::push_back(Parameters* par)
  {
    parameters_list_.push_back(par);
  }

  double_t Combination::get_value(const Position<double_t>& driver,
				  const Position<double_t>& pool,
				  std::vector<double_t>* extent) const
  {
    double_t result = 0.0;
    int_t n = 0;

    // Add the parameter value for all Parameter objects included
    // in the parameter list.
    for(std::vector<Parameters*>::const_iterator it = parameters_list_.begin();
	it != parameters_list_.end(); ++it)
      {
	double_t v = (*it)->get_value(driver, pool, extent);
	if(v != 0)
	  {
	    ++n;
	    result += v;
	  }
      }

    //    result /= n;

    return result;
  }

}
