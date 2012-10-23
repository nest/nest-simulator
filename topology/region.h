#ifndef REGION_H
#define REGION_H

/*
 *  region.h
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

#include "position.h"

#include <vector>

#include "dictdatum.h"
#include "nest.h"

namespace nest{

  class Region;

  /**
   * Abstract region class used to gather the fixed grid and the
   * unrestricted layer regions.
   */
  class AbstractRegion
  {
  public:
    // Pure virtual destructor must be defined (see region.cpp).
    virtual ~AbstractRegion() = 0;

    // Unrestricted layer functions
    virtual Position<double_t> get_lower_left() const = 0;
    virtual Position<double_t> get_upper_right() const = 0;
    virtual bool within_range(const Position<double_t>&) const = 0;
    virtual void set_anchor(const Position<double_t>&) = 0;
    virtual bool outside(const Region&) const = 0;
  };

  /**
   * The Region class defines a base region in a 2D space. The Region
   * class is inherited by other region classes. The class contain a 
   * description of the minimum bounding box (i.e. a rectangular region
   * that covers any sub-region). The minimum bounding
   * box is known by its upper left and lower right corner.
   */  
  class Region: public AbstractRegion
  {
  public:

    /**
     * Default constructor.
     */
    Region();

    /**
     * Copy constructor.
     */
    Region(const Region& r);

    /**
     * Constructor that creates a rectangular region.
     * @param lower_left position of rectangular region.
     * @param upper_right position of rectangular region.
     */
    Region(const Position<double_t>&,
	   const Position<double_t>&);
    
    /**
     * Constructs a rectangular region.
     * @param dict Dictionary that should contain the 
     *             input dictionary "rectangular".
     *             The "rectangular" dictionary should
     *             contain the dictionary parameters 
     *             lower_left and upper_right.
     */
    Region(const DictionaryDatum& dict);

    virtual ~Region(){}

    /**
     * @returns a copy of the calling object.
     */
    virtual Region* copy() const;

    /**
     * @returns lower left position of region
     */
    Position<double_t> get_lower_left() const;

    /**
     * @returns upper right position of region
     */
    Position<double_t> get_upper_right() const;

    /**
     * Checks if a position is within the region defined by the
     * class. Also used to check if a position is within the 
     * bounding box of another region type that inherits from 
     * this class.
     * @param target Input position
     * @returns true if input position is within region
     */
    virtual bool within_range(const Position<double_t>& target) const;

    /**
     * @returns true if input region is completely inside region.
     */
    virtual bool within_range(const Region& reg) const;

    /**
     * Shift region according to an input position.
     * @param pos Position that region should be displaced with.
     */
    virtual void set_anchor(const Position<double_t>& pos);

    /**
     * @input region Region object.
     * @returns true if minimum bounding box of input region is outside 
     *               of calling region.
     */
    virtual bool outside(const Region& reg) const;

    /**
     * Set up region variable.
     * @param mask_dict dictionary describing the layout of the region scope
     */
    static AbstractRegion* create_region(const DictionaryDatum& mask_dict);

    virtual Position<double_t> get_center() const
    {return Position<double_t>();}

  protected:

    // Class variables. Position of minimum bounding box.
    Position<double_t> lower_left_;
    Position<double_t> upper_right_;

  private:

  };

  /**
   * The Circular class defines a circular region in a 2D space. The
   * class inherits from the Region class, and the minimum bounding
   * box of the circular region can be retrieved from this base class.
   * The circular region is identified by the circle center and radius.
   */  
  class Circular: public Region
  {
  public:
    /**
     * Default constructor.
     */
    Circular();

    /**
     * @param radius of the circular region.
     */
    Circular(const double_t);

    /**
     * Constructor accepting an input dictionary. The input dictionary
     * should contain the sub-dictionary "circular". The "circular"
     * dictionary should contain the element "radius".
     * @param dict dictionary.
     */
    Circular(const DictionaryDatum& dict);

    Circular(const Circular&);

    virtual ~Circular(){}

    /**
     * @returns a copy of the calling object.
     */
    virtual Region* copy() const;

    /**
     * See Region::set_anchor(..).
     */
    virtual void set_anchor(const Position<double_t>& pos);

    /**
     * @param target input position
     * @returns true if input position is within circular region.
     */
    virtual bool within_range(const Position<double_t>& target) const;

    /**
     * @returns true if input region is completely inside region.
     */
    virtual bool within_range(const Region& reg) const;

    /**
     * See Region::outside(..).
     */
    virtual bool outside(const Region& reg) const;

    virtual Position<double_t> get_center() const
      {return center_;}

  protected:
    // Class variables
    double_t radius_;

    // Derived value showing the center of the region.
    // Depends upon the lower left and upper right corner
    // points.
    Position<double_t> center_;
  };

  /**
   * The Doughnut class defines a circular region in a 2D space where
   * the center is missing. The class inherits from the Circular class
   * The doughnut region is identified by the center and the inner and 
   * outer radius of the region.
   */ 
  class Doughnut: public Circular
  {
  public:
    /**
     * Default constructor.
     */
    Doughnut();

    /**
     * Constructor that creates a doughnut region centered in origo.
     * @param inner_radius of doughnut region.
     * @param outer_radius of region.
     */
    Doughnut(const double_t, const double_t);

    /**
     * Constructor accepting an input dictionary. The input dictionary
     * should contain the sub-dictionary "doughnut". The "doughnut"
     * dictionary should contain the elements "inner_radius" and
     * "outer_radius".
     * @param dict dictionary.
     */
    Doughnut(const DictionaryDatum& dict);

    Doughnut(const Doughnut&);

    ~Doughnut(){}

    /**
     * @returns a copy of the calling object.
     */
    Region* copy() const;

    /**
     * See Circular::set_anchor(..).
     */
    void set_anchor(const Position<double_t>& pos);

    /**
     * @param target input position
     * @returns true if input position is between inner and outer
     *          radius of region.
     */
    bool within_range(const Position<double_t>& target) const;

    /**
     * @returns true if input region is completely inside region.
     */
    bool within_range(const Region& reg) const;

    /**
     * See Region::outside(..).
     */
    bool outside(const Region& reg) const;
    
  private:
    // Class variables
    // Note: other variables are inherited from Circular and 
    // Region class.
    Circular inner_circle_;
  };

  /**
   * Special region class that is used together with the 
   * unrestricted layer edge options. Based upon an input
   * framework the region can be truncated or wrapped if
   * it exceeds the bounds of this framework. A wrapped 
   * Shift region is split into a maximum of four sub-regions
   * where each region covers a specific area of the wrapped
   * region and where each region is within the bounds
   * specified by the framework. The class contains the 
   * special shift variable that indicates the original 
   * position of a wrapped region.
   *
   * Whether edge wrap or edge truncate should be used
   * can be set with the static variable Shift::EDGE_WRAP.
   */
  class Shift: public Region
  {
  public:
    /**
     * Default constructor.
     */ 
    Shift();

    /**
     * Construct a Shift region based upon a rectangular 
     * region.
     */
    Shift(const Region& s);

    /**
     * Constructs a Shift region based upon a rectangular
     * region and a shift value. The shift value indicates
     * the displacement of the Shift region relevant to its
     * original position.
     */
    Shift(const Position<double_t>& lower_left,
	  const Position<double_t>& upper_right,
	  const Position<double_t>& shift = 
	  Position<double_t>());

    ~Shift(){}

    /**
     * @returns the displacement of the region. Used to 
     *          find the wrapped position of the nodes
     *          overlapping this region.
     */
    Position<double_t> get_shift();

    /**
     * Prints a summary of the class variables. Used for 
     * testing.
     */
    void print();

    /**
     * Function that implements the edge algorithm. When 
     * edge truncate is used any shift region that is outside 
     * the input bounds are cut off at the edge bounds. When
     * edge wrap is used any shift region that is outside the
     * input bounds is split into up to four sub-regions where
     * each region is within the input bounds. In this process
     * some of the sub-regions may be moved in the global 
     * space this is indicated by the Shift region shift 
     * variable.
     * @param boxes vector where result regions is stored
     * @param a_x edge upper left x coordinate
     * @param a_y edge upper left y coordinate
     * @param b_x edge lower right x coordinate
     * @param b_y edge lower right y coordinate
     * @param shift_x region displacement along the x-axis
     * @param shift_y region displacement along the y-axis
     */
    void split_box(std::vector<Shift>& boxes,
		   const double_t a_x,
		   const double_t a_y,
		   const double_t b_x,
		   const double_t b_y,
		   double_t shift_x,
		   double_t shift_y) const;

    //EDGE_WRAP should be set to true if edge wrapping is used
    static bool EDGE_WRAP;    
   
  private:

    /**
     * Function used by split_box(..). Moves(shifts) an input 
     * region so that it will come within the input edge bounds.
     *
     * @param boxes vector where result regions is stored
     * @param a_x edge lower left x coordinate
     * @param a_y edge lower left y coordinate
     * @param b_x edge upper right x coordinate
     * @param b_y edge upper right y coordinate
     * @param lower_left_x corner of new region
     * @param lower_left_y corner of new region
     * @param upper_right_x corner of new region
     * @param upper_right_y corner of new region
     * @param shift_x region displacement along the x-axis
     * @param shift_y region displacement along the y-axis
     */
    void move_box(std::vector<Shift>& boxes,
		  const double_t a_x,
		  const double_t a_y,
		  const double_t b_x,
		  const double_t b_y,
		  double_t lower_left_x,
		  double_t lower_left_y,
		  double_t upper_right_x,
		  double_t upper_right_y,
		  double_t shift_x,
		  double_t shift_y) const;

    // Displacement of region.
    Position<double_t> shift_;

  };

  /**
   * Discrete region used to select a group of nodes in from a 
   * fixed grid layer. In contrast to the other region classes
   * who's continuous the DiscreteRegion operate on a discrete
   * node level. The size of the region is given as number of
   * node rows and columns. The region is centered with the 
   * anchor parameter.
   */  
  class DiscreteRegion: public AbstractRegion
  {
  public:
    /**
     * Default constructor.
     */
    DiscreteRegion();

    DiscreteRegion(const DiscreteRegion& d);

    /**
     */
    DiscreteRegion(const double_t);

    /**
     */
    DiscreteRegion(const DictionaryDatum& dict);

    ~DiscreteRegion(){}

    /**
     * @returns number of rows
     */
    index get_rows() const;

    /**
     * @returns number of columns
     */
    index get_columns() const;

    /**
     * @returns anchor (center) of mask
     */
    Position<int_t> get_anchor() const;

    /**
     * @returns size (number of nodes) of region
     */
    index size() const;

    /**
     * @returns discrete (layer (node) space) 2D position of local
     *          id relevant to mask center.
     */
    Position<int_t> get_position(const int_t lid) const;

    Position<double_t> get_lower_left() const {assert(false);}
    Position<double_t> get_upper_right() const {assert(false);}
    bool within_range(const Position<double_t>&) const {assert(false);}
    void set_anchor(const Position<double_t>&) {assert(false);}
    bool outside(const Region&) const {assert(false);}

  private:
    // Class variables
    index rows_;
    index columns_;
    Position<int_t> anchor_;
  };

  /**
   * Region class used with 3D layers.
   */  
  class Volume: public Region
  {
  public:

    /**
     * Default constructor.
     */
    Volume();

    /**
     * Copy constructor.
     */
    Volume(const Volume& v);

    /**
     * Constructor that creates a rectangular volume.
     * @param lower_left position of rectangular volume.
     * @param upper_right position of rectangular volume.
     */
    Volume(const Position<double_t>&,
	   const Position<double_t>&);
    
    /**
     * Constructs a rectangular volume.
     * @param dict Dictionary that should contain the 
     *             input dictionary "rectangular".
     *             The "rectangular" dictionary should
     *             contain the dictionary parameters 
     *             upper_left and lower_right.
     */
    Volume(const DictionaryDatum& dict);

    ~Volume(){}

    /**
     * @returns a copy of the calling object.
     */
    Volume* copy() const;

    /**
     * @input volume Volume object.
     * @returns true if minimum bounding box of input volume is outside 
     *               of calling volume.
     */
    bool outside(const Volume& reg) const;

  protected:

  private:

  };

  // Experimental combination region:
/*   class Complex: public Region */
/*   { */
/*   public: */
/*     Complex(); */
/*     Complex(const TokenArray& dict); */

/*   private: */
/*     std::vector<Region*> regions_; */
/*   }; */

}

#endif 
