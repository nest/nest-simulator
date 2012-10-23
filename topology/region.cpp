/*
 *  region.cpp
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

#include "region.h"

#include <vector>

#include "dictdatum.h"
#include "nest.h"
#include "tokenarray.h"
#include "dictutils.h"
#include "exceptions.h"
#include "doubledatum.h"
#include "arraydatum.h"

#include "position.h"
#include "topology_names.h"

namespace nest
{

  /***********************************************************************/
  /*                         ABSTRACT REGION                             */
  /***********************************************************************/

  AbstractRegion::~AbstractRegion()
  {}

  /***********************************************************************/
  /*                           BASE REGION                               */
  /***********************************************************************/

  Region::Region():
    lower_left_(0,0),
    upper_right_(0,0)
  {}

  Region::Region(const Region& r):
    AbstractRegion(r),
    lower_left_(r.lower_left_),
    upper_right_(r.upper_right_)
  {}

  Region::Region(const Position<double_t>& lower_left,
		 const Position<double_t>& upper_right):
    lower_left_(lower_left),
    upper_right_(upper_right)
  {}

  Region::Region(const DictionaryDatum& mask_dict)
  {
    DictionaryDatum rectangular_dict = 
      getValue<DictionaryDatum>(mask_dict, Name("rectangular"));
    
    lower_left_ = Position<double_t>
      (getValue<std::vector<double_t> >(rectangular_dict, 
					      "lower_left"));
    
    upper_right_ = Position<double_t>
      (getValue<std::vector<double_t> >(rectangular_dict, 
					      "upper_right"));

    std::vector<double_t> anchor;
    if ( updateValue<std::vector<double_t> >(mask_dict, names::anchor, anchor) )
    	set_anchor(anchor);
  }

  Region* Region::copy() const
  {
    return new Region(*this);
  }

  Position<double_t> Region::get_lower_left() const
  {
    return lower_left_;
  }

  Position<double_t> Region::get_upper_right() const
  {
    return upper_right_;
  }

  void Region::set_anchor(const Position<double_t>& pos)
  {
    lower_left_ += pos;
    upper_right_ += pos;
  }

  bool Region::within_range(const Position<double_t>& target) const
  {
    return target.within_range(lower_left_, upper_right_);
  }

  bool Region::
  within_range(const Region& reg) const
  {
    return within_range(reg.get_lower_left()) && 
      within_range(reg.get_upper_right());
  }

  bool Region::
  outside(const Region& reg) const
  {
    // Input region is outside region if lower left corner of
    // input region is above or right of region, or if 
    // upper right corner of input region is below or 
    // to left of region.
    const Position<double_t> ll = reg.get_lower_left();
    const Position<double_t> ur = reg.get_upper_right();

    if(ll.get_x() > upper_right_.get_x() ||
       ll.get_y() > upper_right_.get_y() ||
       ur.get_x() < lower_left_.get_x() || 
       ur.get_y() < lower_left_.get_y())
      {
	return true;
      }
    return false;
  }

  AbstractRegion* Region::create_region(const DictionaryDatum& mask_dict)
  {
    // Set up region according to input dictionary
    if(mask_dict->known(Name("rectangular")))
      {
	return new Region(mask_dict);
      }
    else if(mask_dict->known(Name("circular")))
      {
	return new Circular(mask_dict);
      }
    else if(mask_dict->known(Name("doughnut")))
      {
	return new Doughnut(mask_dict);
      }
    else if(mask_dict->known(Name("volume")))
      {
	return new Volume(mask_dict);
      }
    else if(mask_dict->known(Name("grid")))
      {
	// Fixed grid region.
	return new DiscreteRegion(mask_dict);
      }
    else
      {
	throw TypeMismatch("region",  "nothing");
      }
  }

  /***********************************************************************/
  /*                         CIRCULAR REGION                             */
  /***********************************************************************/

  Circular::Circular():
    Region(),
    radius_(0.0),
    center_(0,0)
  {}

  Circular::Circular(const Circular& c):
    Region(c),
    radius_(c.radius_),
    center_(c.center_)
  {}
  
  Circular::Circular(const double_t radius):
    Region(),
    radius_(radius),
    center_(0,0)
  {
    lower_left_ = Position<double_t>(-radius_, -radius_);
    upper_right_ = Position<double_t>(radius_, radius_);
  }
  
  Circular::Circular(const DictionaryDatum& mask_dict):
    Region(),
    center_(0,0)
  {
    radius_ = 
      getValue<double_t>(getValue<DictionaryDatum>(mask_dict, 
							 Name("circular")), 
			       "radius");

    lower_left_ = Position<double_t>(-radius_, -radius_);
    upper_right_ = Position<double_t>(radius_, radius_);

    std::vector<double_t> anchor;
    if ( updateValue<std::vector<double_t> >(mask_dict, names::anchor, anchor) )
    	set_anchor(anchor);
  }

  Region* Circular::copy() const
  {
    return new Circular(*this);
  }

  void Circular::set_anchor(const Position<double_t>& pos)
  {
    Region::set_anchor(pos);
    center_ += pos;
  }

  bool Circular::
  within_range(const Position<double_t>& target) const
  {
    return (target - center_).length() <= radius_;
  }

  bool Circular::
  within_range(const Region& reg) const
  {
    // Input region is fully within circular region if all 
    // four corner points of input region are within circular 
    // region.
    return within_range(reg.get_lower_left()) && 
      within_range(reg.get_upper_right()) &&
      within_range(Position<double_t>(reg.get_lower_left().get_x(),
					    reg.get_upper_right().get_y())) &&
      within_range(Position<double_t>(reg.get_upper_right().get_x(),
					    reg.get_lower_left().get_y()));
  }

  bool Circular::
  outside(const Region& reg) const
  {
    // We only check if input region is outside minimum bounding box
    // of circular region. Add a complete check for a circular 
    // region if you find this worthwhile. Please don't forget to 
    // account for all situations if you do this.
    return Region::outside(reg);
  }

  /***********************************************************************/
  /*                        DOUGHNUT REGION                              */
  /***********************************************************************/

  Doughnut::Doughnut():
    Circular(),
    inner_circle_()
  {}
  
  Doughnut::Doughnut(const Doughnut& d):
    Circular(d),
    inner_circle_(d.inner_circle_)
  {}

  Doughnut::Doughnut(const double_t inner_radius, 
		     const double_t outer_radius):
    Circular(outer_radius),
    inner_circle_(inner_radius)
  {
  }
  
  Doughnut::Doughnut(const DictionaryDatum& mask_dict)
  {
    DictionaryDatum dictionary = 
      getValue<DictionaryDatum>(mask_dict, Name("doughnut"));

    inner_circle_ = 
      Circular(getValue<double_t>(dictionary, "inner_radius"));

    radius_ = getValue<double_t>(dictionary, "outer_radius");    

    lower_left_ = Position<double_t>(-radius_, -radius_);
    upper_right_ = Position<double_t>(radius_, radius_);

    std::vector<double_t> anchor;
    if ( updateValue<std::vector<double_t> >(mask_dict, names::anchor, anchor) )
    	set_anchor(anchor);
  }

  Region* Doughnut::copy() const
  {
    return new Doughnut(*this);
  }

  void Doughnut::set_anchor(const Position<double_t>& pos)
  {
    Circular::set_anchor(pos);
    inner_circle_.set_anchor(pos);
  }

  bool Doughnut::
  within_range(const Position<double_t>& target) const
  {
    return Circular::within_range(target) && 
      !(inner_circle_.within_range(target));
  }

  bool Doughnut::
  within_range(const Region& reg) const
  {
    // Input region is within the doughnut region if it is 
    // inside the outer circle and outside the inner circle.
    return Circular::within_range(reg) && 
      inner_circle_.outside(reg);
  }

  bool Doughnut::
  outside(const Region& reg) const
  {
    // Input region is outside the doughnut region if it is
    // outside the outer circle or inside the inner circle.
    return Circular::outside(reg) || 
      inner_circle_.within_range(reg);
  }

  /***********************************************************************/
  /*                    SPECIAL SHIFT REGION                             */
  /***********************************************************************/

  bool Shift::EDGE_WRAP = false;

  Shift::Shift(const Position<double_t>& lower_left,
	       const Position<double_t>& upper_right,
	       const Position<double_t>& shift)
  {
    lower_left_ = lower_left;
    upper_right_ = upper_right;
    shift_ = shift;    
  }

  Shift::Shift(const Region& s):
    Region(s),
    shift_()
  {}
  
  Position<double_t> Shift::get_shift()
  {
    return shift_;
  }

  void Shift::
  split_box(std::vector<Shift>& boxes,
	    const double_t a_x,
	    const double_t a_y,
	    const double_t b_x,
	    const double_t b_y,
	    double_t shift_x,
	    double_t shift_y) const
  {
    // Temporarily store the Shift region dimensions in 
    // some other variables.
    double_t lower_left_x = lower_left_.get_x();
    double_t lower_left_y = lower_left_.get_y();
    double_t upper_right_x = upper_right_.get_x();
    double_t upper_right_y = upper_right_.get_y();

    // Truncate the region at the edges if edge wrap 
    // isn't used
    if(!EDGE_WRAP)
      {
	// Truncate edges
	if(lower_left_x < a_x)
	  {
	    lower_left_x = a_x;
	  }
	if(lower_left_y < a_y)
	  {
	    lower_left_y = a_y;
	  }
	if(upper_right_x > b_x)
	  {
	    upper_right_x = b_x;
	  }
	if(upper_right_y > b_y)
	  {
	    upper_right_y = b_y;
	  }
	
	// Region is skipped if it is completely outside the edges.
	if(lower_left_x > upper_right_x || lower_left_y > upper_right_y)
	  {
	    return;
	  } 

	// Return truncated region with no displacement.
	boxes.push_back(Shift(Position<double_t>(lower_left_x, 
						       lower_left_y),
			      Position<double_t>(upper_right_x,
						       upper_right_y),
 			      Position<double_t>(0, 0)));
	return;
      }

    // Region doesn't have any size. This can happen 
    // on rare occasions. For example on some occasions
    // when the mask region is almost exactly overlapping
    // the complete layer.
    if(lower_left_x == upper_right_x ||
       lower_left_y == upper_right_y)
      {
	return;
      }

    // Region is completely outside of edge bounds.
    if(lower_left_x >= b_x ||
       lower_left_y >= b_y ||
       upper_right_x <= a_x ||
       upper_right_y <= a_y)
      {
	//Shift entire region within edge bounds.
	move_box(boxes, a_x, a_y, b_x, b_y, 
		 lower_left_x, lower_left_y,
		 upper_right_x, upper_right_y, 
		 shift_x, shift_y);
	return;
      }

    // Region is left of the leftmost x-bound
    if(lower_left_x < a_x)
      {
	Shift(Position<double_t>(lower_left_x, lower_left_y), 
	      Position<double_t>(a_x, upper_right_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
	
	Shift(Position<double_t>(a_x, lower_left_y), 
	      Position<double_t>(upper_right_x, upper_right_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
	return;
      }
    // Region is right of the rightmost x-bound
    else if(upper_right_x > b_x)
      {
	Shift(Position<double_t>(lower_left_x, lower_left_y),
	      Position<double_t>(b_x, upper_right_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);

	Shift(Position<double_t>(b_x, lower_left_y),
	      Position<double_t>(upper_right_x, upper_right_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
	
	return;
      }
    
    // Region is above the topmost y-bound
    if(upper_right_y > b_y)
      {
	Shift(Position<double_t>(lower_left_x, lower_left_y),
	      Position<double_t>(upper_right_x, b_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);

	Shift(Position<double_t>(lower_left_x, b_y),
	      Position<double_t>(upper_right_x, upper_right_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
	
	return;
      }
    // Region is below the lower y-bound
    else if(lower_left_y < a_y)
      {

	Shift(Position<double_t>(lower_left_x, lower_left_y),
	      Position<double_t>(upper_right_x, a_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
	

	Shift(Position<double_t>(lower_left_x, a_y),
	      Position<double_t>(upper_right_x, upper_right_y)).
	  split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
	
	return;
      }
    
    // Entire region is within edge bounds and is stored
    // in the output region list. The shift variables indicate
    // the original position of the region.

    boxes.push_back(Shift(lower_left_, upper_right_,
			  Position<double_t>(shift_x,
						   shift_y)));
    return;
  } 

  void Shift::
  move_box(std::vector<Shift>& boxes,
	   const double_t a_x,
	   const double_t a_y,
	   const double_t b_x,
	   const double_t b_y,
	   double_t lower_left_x,
	   double_t lower_left_y,
	   double_t upper_right_x,
	   double_t upper_right_y,
	   double_t shift_x,
	   double_t shift_y) const
  {
    // Distance that the region will be shifted with
    // to come within the edge bounds.
    const double_t width = b_x - a_x;
    const double_t height = b_y - a_y;

    // Please make sure that two reverse operations
    // are not listed consecutively below
    // (i.e. right-left, top-bottom).
    
    // Move region to the right.
    if(lower_left_x < a_x)
      {
	lower_left_x += width;
	upper_right_x += width;
	shift_x += width;
      }

    // Move region downwards.
    else if(upper_right_y > b_y)
      {
	lower_left_y -= height;
	upper_right_y -= height;
	shift_y -= height;
      }
    
    // Move region to the left.
    else if(upper_right_x > b_x)
      {
	upper_right_x -= width;
	lower_left_x -= width;
	shift_x -= width;
      }

    // Move region upwards.
    else if(lower_left_y < a_y)
      {
	upper_right_y += height;
	lower_left_y += height;
	shift_y += height;
      }

    // Check if region now is within the edge bounds.
    Shift(Position<double_t>(lower_left_x, lower_left_y), 
	  Position<double_t>(upper_right_x, upper_right_y)).split_box(boxes, a_x, a_y, b_x, b_y, shift_x, shift_y);
    return;
  }

  //Test function, can be removed or modified
  void Shift::print()
  {
//     std::cout << "UL: " << upper_left_.get_x() << "," 
// 	      << upper_left_.get_y() << "\tLR: " 
// 	      << lower_right_.get_x() << "," << lower_right_.get_y() 
//               << "S: " << shift_.get_x() << "," << shift_.get_y()
// 	      << "\n";
  }

  /***********************************************************************/
  /*                        DISCRETE REGION                              */
  /***********************************************************************/

  DiscreteRegion::DiscreteRegion():
    rows_(),
    columns_(),
    anchor_(0,0)
  {}

  DiscreteRegion::DiscreteRegion(const DiscreteRegion& d):
    AbstractRegion(d),
    rows_(d.rows_),
    columns_(d.columns_),
    anchor_(d.anchor_)
  {}

  DiscreteRegion::DiscreteRegion(const DictionaryDatum& mask_dict):
    anchor_(0,0)
  {
    DictionaryDatum grid_dict = 
      getValue<DictionaryDatum>(mask_dict, Name("grid"));
    
    rows_ = getValue<long_t>(grid_dict, "rows");
    columns_ = getValue<long_t>(grid_dict, "columns");
    
    DictionaryDatum anchor = new Dictionary();
    if(updateValue<DictionaryDatum>(mask_dict, "anchor", anchor))
      {
	anchor_ = 
	  Position<int_t>(getValue<long_t>(anchor, "column"), 
				getValue<long_t>(anchor, "row"));
      }
    else
      {
	anchor_ = Position<int_t>(0,0);
      }
  }
  
  index DiscreteRegion::get_rows() const
  {
    return rows_;
  }

  index DiscreteRegion::get_columns() const
  {
    return columns_;
  }

  Position<int_t> DiscreteRegion::get_anchor() const
  {
    return anchor_;
  }

  index DiscreteRegion::size() const
  {
    return rows_*columns_;
  }
  
  Position<int_t> DiscreteRegion::get_position(const int_t lid) const
  {
    //Converts lid position to 2-dimensional layerspace position.
    return anchor_ - Position<int_t>(lid / (int_t)rows_, 
					   lid % (int_t)rows_); 
  }

  /***********************************************************************/
  /*                       3D VOLUME REGION                              */
  /***********************************************************************/

  Volume::Volume():
    Region()
  {}

  Volume::Volume(const Volume& v):
    Region(v)
  {}

  Volume::Volume(const Position<double_t>& lower_left,
		 const Position<double_t>& upper_right):
    Region(lower_left, upper_right)
  {}

  Volume::Volume(const DictionaryDatum& mask_dict)
  {
    DictionaryDatum rectangular_dict = 
      getValue<DictionaryDatum>(mask_dict, Name("volume"));
   
    lower_left_ = Position<double_t>
      (getValue<std::vector<double_t> >(rectangular_dict, 
					      "lower_left"));
    
    upper_right_ = Position<double_t>
      (getValue<std::vector<double_t> >(rectangular_dict, 
					      "upper_right"));
    
  }

  Volume* Volume::copy() const
  {
    return new Volume(*this);
  }

  bool Volume::
  outside(const Volume& reg) const
  {
    const Position<double_t> ur = reg.get_upper_right();
    const Position<double_t> ll = reg.get_lower_left();

    if(ll.get_x() > upper_right_.get_x() ||
       ll.get_y() > upper_right_.get_y() ||
       ll.get_z() > upper_right_.get_z() ||
       ur.get_x() < lower_left_.get_x() || 
       ur.get_y() < lower_left_.get_y() ||
       ur.get_z() < lower_left_.get_z())
      {
	return true;
      }
    return false;
  }

  /***********************************************************************/
  /*                        COMPLEX REGION                               */
  /***********************************************************************/

//   Complex::Complex():
//     Region(),
//     regions_()
//   {}

//   Complex::Complex(const TokenArray& settings):
//     Region(),
//     regions_()
//   {
//     Position<double_t> ll(std::numeric_limits<double_t>::max(),
// 				std::numeric_limits<double_t>::max());
//     Position<double_t> ur(std::numeric_limits<double_t>::max()*-1,
// 				std::numeric_limits<double_t>::max()*-1);

//     // Check dictionary for all parameter objects.
//     for(int_t i = 0; i != settings.size(); ++i)
//       {
// 	regions_.
// 	  push_back(create_region(getValue<DictionaryDatum>(settings[i])));
//       }

//     for(std::vector<Region*>::iterator it = regions_.begin();
// 	it != regions_.end(); ++it)
//       {
// 	if((*it)->get_lower_left().get_x() < ll.get_x())
// 	  {
// 	    ll.set_x((*it)->get_lower_left().get_x());
// 	  }

// 	if((*it)->get_lower_left().get_y() < ll.get_y())
// 	  {
// 	    ll.set_y((*it)->get_lower_left().get_y());
// 	  }

// 	if((*it)->get_upper_right().get_x() > ur.get_x())
// 	  {
// 	    ur.set_x((*it)->get_upper_right().get_x());
// 	  }

// 	if((*it)->get_upper_right().get_y() > ur.get_y())
// 	  {
// 	    ur.set_y((*it)->get_upper_right().get_y());
// 	  }
//       }
//   }

//   Region* Complex::copy() const
//   {
//     return new Complex(*this);
//   }

//   void Complex::set_anchor(const Position<double_t>& pos)
//   {
//     Region::set_anchor(pos);
//     center_ = pos;
//   }

//   bool Complex::
//   within_range(const Position<double_t>& target) const
//   {
//     return (target - center_).length() <= radius_;
//   }

//   bool Complex::
//   outside(const Region& reg) const
//   {
//     const Position<double_t> ll = reg.get_lower_left();
//     const Position<double_t> ur = reg.get_upper_right();

//     // Circle is fully outside region if lower left corner of 
//     // region is above circle top or beyond right circle edge,
//     // or if upper right corner of region is below circle 
//     // bottom or beyond left circle edge.
//     if(ll.get_y() > (center_.get_y() + radius_) ||
//        ur.get_x() < (center_.get_x() - radius_) ||
//        ur.get_y() < (center_.get_y() - radius_) ||
//        ll.get_x() > (center_.get_x() + radius_))
//       {
// 	return true;
//       }	   

//     return false;
//   }

}
