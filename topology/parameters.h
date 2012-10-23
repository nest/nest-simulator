#ifndef PARAMETERS_H
#define PARAMETERS_H

/*
 *  parameters.h
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

#include "nest.h"
#include "dictdatum.h"
#include "randomgen.h"

#include "position.h"

/** @file parameters.h
 *  Implements the Parameters structure.
 */

namespace nest{

  /**
   * The Parameters class is used to store information
   * about the weight, delay and probability of a mask
   * object. There should be a parameter object for
   * each of these attributes. The basic Parameters
   * object store the attribute value as a constant
   * number. But other classes that inherit from this
   * class can be built that can calculate a more 
   * sophisticated set of values. See: Gaussian.
   */    
  class Parameters
  {
  public:
    /**
     * Default constructor
     */
    Parameters();

    /**
     * Creates a Parameters object that returns a constant value.
     * @param constant double
     */
    Parameters(double_t);

    /**
     * Constructor that sets up the basic shared settings of the 
     * Parameters structure. Used by children of the Parameters class.
     */
    Parameters(const DictionaryDatum& dict);

    virtual ~Parameters(){}

    /**
     * @param pos  Input position.
     * @returns the value of the Parameters object at the selected
     *          position
     */
    virtual double_t 
      get_value(const Position<double_t>& pos) const;

    /**
     * Calculates the relative position between the driver and the
     * pool node and returns the Parameters value for this
     * position. 
     *
     * @param driver  Node position
     * @param pool    Node position
     * @param extent  layer extent for periodic boundary conditions, or 0 otherwise
     * 
     * @returns the value of the Parameters object at the selected
     *          position
      */
    virtual double_t 
      get_value(const Position<double_t>& driver,
		const Position<double_t>& pool,
	        std::vector<double_t>* extent=0) const;
      
    /**
     * Move the input value within the bounds of the parameters
     * object if it is outside the min or max values. And sets the
     * return value to 0 if the input value is smaller than the
     * cutoff value.
     * @param value  input value
     * @returns new value
     */
    virtual double_t bound(double_t value) const;

    /**
     * Set minimum parameters value to 0.
     */
    void force_positive();

    /**
     * Create a Parameters object based upon an entry in the input
     * dictionary. 
     *
     * @param mask_dict Mask dictionary
     * @returns a Parameters object.
     */
    static Parameters* create_parameter(const DictionaryDatum& mask_dict);

  protected:
    // Minimum and maximum value allowed by calls to get_value(..)
    double_t min_;
    double_t max_;

    // Position at which get_value(..) returns a value. If we're outside
    // this bound 0 is returned.
    double_t cutoff_;
    double_t cutoff_distance_;

    // Global spatial anchor (center) of the Parameters object.
    std::vector<double_t> anchor_;

  private:
    //Member variables
    double_t k_;
  };

  /**
   * The Gaussian class is used to retrieve spatially
   * distributed values for a mask object. The spatially 
   * distributed values are based on the gaussian function
   * c + p_center * std::exp(-(distance-mean)^2/(sigma^2))
   * These values have to be set in the input dictionary 
   * given at the creation of an object of this class.
   */   
  class Gaussian: public Parameters
  {
  public:
    Gaussian();
    Gaussian(const DictionaryDatum& dict);

    ~Gaussian(){}
      
    using Parameters::get_value;

    double_t 
      get_value(const Position<double_t>& pos) const;

  private:
    //Member variables
    double_t c_;
    double_t p_center_;
    double_t mean_; 
    double_t sigma_;
  };

  /***************** test bivariate Gauss *************************/
  /**
   * The Gaussian2D class is used to retrieve spatially
   * distributed values for a mask object. The spatially 
   * distributed values are based on the bivariate gaussian function
   * c + p_center * std::exp(-((x-mean_x)^2/(sigma_x^2) + (y-mean_y)^2/(sigma_y^2) - 2*(x-mean_x)*(y-mean_y)*rho/(sigma_x*sigma_y))/(2*(1-rho^2)))
   * (cf. e.g. http://en.wikipedia.org/wiki/Multivariate_normal_distribution)
   * The parameter rho determines the correlation between the x and 
   * y coordinate and must be within ]-1,1[.
   * These values have to be set in the input dictionary 
   * given at the creation of an object of this class.
   *
   * Thanks to Birgit Kriener for adding this parameter class.
   */   
  class Gaussian2D: public Parameters
  {
  public:
    Gaussian2D();
    Gaussian2D(const DictionaryDatum& dict);
      
    ~Gaussian2D(){}

    using Parameters::get_value;

    double_t 
      get_value(const Position<double_t>& pos) const;

  private:
    //Member variables
    double_t c_;
    double_t p_center_;
    double_t mean_x_; 
    double_t sigma_x_;
    double_t mean_y_; 
    double_t sigma_y_;
    double_t rho_;
  };
  /***************************************************************/

  /**
   * The linear class generates values based upon the 
   * function  a*distance + c
   */  
  class Linear: public Parameters
  {
  public:
    Linear();
    Linear(const DictionaryDatum& dict);
      
    ~Linear(){}

    using Parameters::get_value;

    double_t 
      get_value(const Position<double_t>& pos) const;

  private:
    //Member variables
    double_t a_;
    double_t c_;
  };

  /**
   * Class generates values based on the function
   * c + a * std::exp( -distance/tau )
   */
  class Exponential: public Parameters
  {
  public:
    Exponential();
    Exponential(const DictionaryDatum& dict);
      
    ~Exponential(){}

    using Parameters::get_value;

    double_t 
      get_value(const Position<double_t>& pos) const;

  private:
    //Member variables
    double_t a_;
    double_t c_;
    double_t tau_;
  };

  /**
   * Class generates values placed randomly within the range
   * specified by the class variables.
   */
  class Uniform: public Parameters
  {
  public:
    Uniform();
    Uniform(const DictionaryDatum& dict);
      
    ~Uniform(){}

    using Parameters::get_value;

    double_t 
      get_value(const Position<double_t>&) const;

  private:
    //Member variables
    double_t range_;
    double_t lower_;

    librandom::RngPtr rng_;
  };

  /**
   * Parameters class used by fixed grid layers. In contrast to the
   * other Parameters classes (who's values are calculated on the
   * spot) the Discrete class values are calculated before hand.
   * Each position in the Discrete value_ vector corresponds to 
   * one position in the DiscreteRegion (region.h) class. In 
   * this class the positions are given as local ids instead of 
   * spatial positions. 
   */
  class Discrete: public Parameters
  {
  public:
    Discrete();
    Discrete(std::vector<double_t> values);
    Discrete(const DictionaryDatum& dict);
      
    ~Discrete(){}

    using Parameters::get_value;

    /**
     * @param pos  Input position (local id!).
     * @returns the value of the Discrete parameters object at the 
     *          selected position.
     */
    double_t 
      get_value(const Position<double_t>& lid) const;

    double_t 
      get_value(const Position<double_t>&,
		const Position<double_t>& lid,
	        std::vector<double_t>* extent=0) const;
    
  private:
    //Member variables
    std::vector<double_t> values_;
  };

  /**
   * Experimental class that combines many different parameters
   * objects. 
   */

  class Combination: public Parameters
  {
  public:
    Combination();
    Combination(const TokenArray& dict);
      
    ~Combination(){}

    void push_back(Parameters* par);

    using Parameters::get_value;

    double_t 
      get_value(const Position<double_t>& driver,
		const Position<double_t>& pool,
	        std::vector<double_t>* extent=0) const;
    
  private:
    //Member variables
    std::vector<Parameters*> parameters_list_;
  };

} //namespace ends

#endif 


