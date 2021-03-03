/*
 *  mask.h
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

#ifndef MASK_H
#define MASK_H

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_names.h"
#include "nest_types.h"
#include "nestmodule.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

// Includes from spatial:
#include "position.h"

namespace nest
{
class AbstractMask;

typedef sharedPtrDatum< AbstractMask, &NestModule::MaskType > MaskDatum;


/**
 * Abstract base class for masks with unspecified dimension.
 */
class AbstractMask
{
public:
  /**
   * Virtual destructor
   */
  virtual ~AbstractMask()
  {
  }

  /**
   * @returns true if point is inside mask.
   */
  virtual bool inside( const std::vector< double >& ) const = 0;

  /**
   * @returns a dictionary with the definition for this mask.
   */
  virtual DictionaryDatum
  get_dict() const
  {
    throw KernelException( "Can not convert mask to dict" );
  }

  /**
   * Create the intersection of this mask with another. Masks must have
   * the same dimension
   * @returns a new dynamically allocated mask.
   */
  virtual AbstractMask* intersect_mask( const AbstractMask& other ) const = 0;

  /**
   * Create the union of this mask with another. Masks must have the same
   * dimension.
   * @returns a new dynamically allocated mask.
   */
  virtual AbstractMask* union_mask( const AbstractMask& other ) const = 0;

  /**
   * Create the difference of this mask and another. Masks must have the
   * same dimension.
   * @returns a new dynamically allocated mask.
   */
  virtual AbstractMask* minus_mask( const AbstractMask& other ) const = 0;
};

/**
 * Abstract base class for masks with given dimension.
 */
template < int D >
class Mask : public AbstractMask
{
public:
  using AbstractMask::inside;

  ~Mask()
  {
  }

  /**
   * @returns true if point is inside mask.
   */
  virtual bool inside( const Position< D >& ) const = 0;

  /**
   * @returns true if point is inside mask.
   */
  bool inside( const std::vector< double >& pt ) const;

  /**
   * @returns true if the whole box is inside the mask.
   * @note a return value of false is not a guarantee that the whole box
   * is not inside the mask.
   */
  virtual bool inside( const Box< D >& ) const = 0;

  /**
   * @returns true if the whole box is outside the mask.
   * @note a return value of false is not a guarantee that the whole box
   * is not outside the mask.
   */
  virtual bool outside( const Box< D >& b ) const;

  /**
   * The whole mask is inside (i.e., false everywhere outside) the bounding box.
   * @returns bounding box
   */
  virtual Box< D > get_bbox() const = 0;

  /**
   * Clone method.
   * @returns dynamically allocated copy of mask object
   */
  virtual Mask* clone() const = 0;

  AbstractMask* intersect_mask( const AbstractMask& other ) const;
  AbstractMask* union_mask( const AbstractMask& other ) const;
  AbstractMask* minus_mask( const AbstractMask& other ) const;
};

/**
 * Mask which covers all of space
 */
template < int D >
class AllMask : public Mask< D >
{
public:
  ~AllMask()
  {
  }

  using Mask< D >::inside;

  /**
   * @returns true always for this mask.
   */
  bool
  inside( const Position< D >& ) const
  {
    return true;
  }

  /**
   * @returns true always for this mask
   */
  bool
  inside( const Box< D >& ) const
  {
    return true;
  }

  /**
   * @returns false always for this mask
   */
  bool
  outside( const Box< D >& ) const
  {
    return false;
  }

  Box< D >
  get_bbox() const
  {
    const double inf = std::numeric_limits< double >::infinity();
    return Box< D >( Position< D >( -inf, -inf ), Position< D >( inf, inf ) );
  }

  Mask< D >*
  clone() const
  {
    return new AllMask();
  }
};

/**
 * Mask defining a box region.
 */
template < int D >
class BoxMask : public Mask< D >
{
public:
  /**
   * Parameters that should be in the dictionary:
   * lower_left    - Position of lower left corner (array of doubles)
   * upper_right   - Position of upper right corner (array of doubles)
   * azimuth_angle - Rotation angle in degrees from x-axis (double), optional
   * polar_angle   - Rotation angle in degrees from z-axis (double), the polar
   *                 angle does not apply in 2D, optional
   */
  BoxMask( const DictionaryDatum& );

  BoxMask( const Position< D >& lower_left,
    const Position< D >& upper_right,
    const double azimuth_angle = 0.0,
    const double polar_angle = 0.0 );

  ~BoxMask()
  {
  }

  using Mask< D >::inside;

  /**
   * @returns true if point is inside the box
   */
  bool inside( const Position< D >& p ) const;

  /**
   * @returns true if the whole given box is inside this box
   */
  bool inside( const Box< D >& b ) const;

  /**
   * @returns true if the whole given box is outside this box
   */
  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  DictionaryDatum get_dict() const;

  Mask< D >* clone() const;

  /**
   * @returns the name of this mask type.
   */
  static Name get_name();

protected:
  /**
   *  Calculate the min/max x, y, z values in case of a rotated box.
   */
  void calculate_min_max_values_();

  Position< D > lower_left_;
  Position< D > upper_right_;

  /*
   * The {min,max}_values_ correspond to the minimum and maximum x, y, z values
   * after the box has been rotated. That is, the lower_left and upper_right of
   * the bounding box of the rotated box. If the box is not rotated,
   * min_values_ = lower_left_ and max_values_ = upper_right_.
   */
  Position< D > min_values_;
  Position< D > max_values_;

  double azimuth_angle_;
  double polar_angle_;
  double azimuth_cos_;
  double azimuth_sin_;
  double polar_cos_;
  double polar_sin_;

  Position< D > cntr_;
  Position< D > eps_;
  double cntr_x_az_cos_;
  double cntr_x_az_sin_;
  double cntr_y_az_cos_;
  double cntr_y_az_sin_;
  double cntr_z_pol_cos_;
  double cntr_z_pol_sin_;
  double cntr_x_az_cos_pol_cos_;
  double cntr_x_az_cos_pol_sin_;
  double cntr_y_az_sin_pol_cos_;
  double cntr_y_az_sin_pol_sin_;
  double az_cos_pol_cos_;
  double az_cos_pol_sin_;
  double az_sin_pol_cos_;
  double az_sin_pol_sin_;

  bool is_rotated_;
};

/**
 * Mask defining a circular or spherical region.
 */
template < int D >
class BallMask : public Mask< D >
{
public:
  /**
   * @param center Center of sphere
   * @param radius Radius of sphere
   */
  BallMask( Position< D > center, double radius )
    : center_( center )
    , radius_( radius )
  {
  }

  /**
   * Creates a BallMask from a Dictionary which should contain the key
   * "radius" with a double value and optionally the key "anchor" (the
   * center position) with an array of doubles.
   */
  BallMask( const DictionaryDatum& );

  ~BallMask()
  {
  }

  using Mask< D >::inside;

  /**
   * @returns true if point is inside the circle
   */
  bool inside( const Position< D >& p ) const;

  /**
   * @returns true if the whole box is inside the circle
   */
  bool inside( const Box< D >& ) const;

  /**
   * @returns true if the whole box is outside the circle
   */
  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  DictionaryDatum get_dict() const;

  Mask< D >* clone() const;

  /**
   * @returns the name of this mask type.
   */
  static Name get_name();

protected:
  Position< D > center_;
  double radius_;
};

/**
 * Mask defining an elliptical or ellipsoidal region.
 */
template < int D >
class EllipseMask : public Mask< D >
{
public:
  /**
   * @param center Center of ellipse
   * @param major_axis Length of major axis of ellipse or ellipsoid
   * @param minor_axis Length of minor axis of ellipse or ellipsoid
   * @param polar_axis Length of polar axis of ellipsoid
   * @param azimuth_angle Angle in degrees between x-axis and major axis of the
   *        ellipse or ellipsoid
   * @param polar_angle Angle in degrees between z-axis and polar axis of the
   *        ellipsoid
   */
  EllipseMask( Position< D > center,
    double major_axis,
    double minor_axis,
    double polar_axis,
    double azimuth_angle,
    double polar_angle )
    : center_( center )
    , major_axis_( major_axis )
    , minor_axis_( minor_axis )
    , polar_axis_( polar_axis )
    , azimuth_angle_( azimuth_angle )
    , polar_angle_( polar_angle )
    , x_scale_( 4.0 / ( major_axis_ * major_axis_ ) )
    , y_scale_( 4.0 / ( minor_axis_ * minor_axis_ ) )
    , z_scale_( 4.0 / ( polar_axis_ * polar_axis_ ) )
    , azimuth_cos_( std::cos( azimuth_angle_ * numerics::pi / 180. ) )
    , azimuth_sin_( std::sin( azimuth_angle_ * numerics::pi / 180. ) )
    , polar_cos_( std::cos( polar_angle_ * numerics::pi / 180. ) )
    , polar_sin_( std::sin( polar_angle_ * numerics::pi / 180. ) )
  {
    if ( major_axis_ <= 0 or minor_axis_ <= 0 or polar_axis_ <= 0 )
    {
      throw BadProperty(
        "nest::EllipseMask<D>: "
        "All axis > 0 required." );
    }
    if ( major_axis_ < minor_axis_ )
    {
      throw BadProperty(
        "nest::EllipseMask<D>: "
        "major_axis greater than minor_axis required." );
    }
    if ( D == 2 and not( polar_angle_ == 0.0 ) )
    {
      throw BadProperty(
        "nest::EllipseMask<D>: "
        "polar_angle not defined in 2D." );
    }

    create_bbox_();
  }

  /**
   * Creates an EllipseMask from a Dictionary which should contain the keys
   * "major_axis" and "minor_axis" with double values, and optionally the keys
   * "polar_axis", "anchor" (the center position), "azimuth_angle" or
   * "polar_angle" with a double, an array of doubles, a double and a double,
   * respectively.
   */
  EllipseMask( const DictionaryDatum& );

  ~EllipseMask()
  {
  }

  using Mask< D >::inside;

  /**
   * @returns true if point is inside the ellipse
   */
  bool inside( const Position< D >& p ) const;

  /**
   * @returns true if the whole box is inside the ellipse
   */
  bool inside( const Box< D >& ) const;

  /**
   * @returns true if the whole box is outside the ellipse
   */
  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  DictionaryDatum get_dict() const;

  Mask< D >* clone() const;

  /**
   * @returns the name of this mask type.
   */
  static Name get_name();

private:
  void create_bbox_();

  Position< D > center_;
  double major_axis_;
  double minor_axis_;
  double polar_axis_;
  double azimuth_angle_;
  double polar_angle_;

  double x_scale_;
  double y_scale_;
  double z_scale_;

  double azimuth_cos_;
  double azimuth_sin_;
  double polar_cos_;
  double polar_sin_;

  Box< D > bbox_;
};

/**
 * Mask combining two masks with a Boolean AND, the intersection.
 */
template < int D >
class IntersectionMask : public Mask< D >
{
public:
  using Mask< D >::inside;

  /**
   * Construct the intersection of the two given masks. Copies are made
   * of the supplied Mask objects.
   */
  IntersectionMask( const Mask< D >& m1, const Mask< D >& m2 )
    : mask1_( m1.clone() )
    , mask2_( m2.clone() )
  {
  }

  /**
   * Copy constructor
   */
  IntersectionMask( const IntersectionMask& m )
    : Mask< D >( m )
    , mask1_( m.mask1_->clone() )
    , mask2_( m.mask2_->clone() )
  {
  }

  ~IntersectionMask()
  {
    delete mask1_;
    delete mask2_;
  }

  bool inside( const Position< D >& p ) const;

  bool inside( const Box< D >& b ) const;

  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  Mask< D >* clone() const;

protected:
  Mask< D >* mask1_, *mask2_;
};

/**
 * Mask combining two masks with a Boolean OR, the sum.
 */
template < int D >
class UnionMask : public Mask< D >
{
public:
  using Mask< D >::inside;

  /**
   * Construct the union of the two given masks. Copies are made
   * of the supplied Mask objects.
   */
  UnionMask( const Mask< D >& m1, const Mask< D >& m2 )
    : mask1_( m1.clone() )
    , mask2_( m2.clone() )
  {
  }

  /**
   * Copy constructor
   */
  UnionMask( const UnionMask& m )
    : Mask< D >( m )
    , mask1_( m.mask1_->clone() )
    , mask2_( m.mask2_->clone() )
  {
  }

  ~UnionMask()
  {
    delete mask1_;
    delete mask2_;
  }

  bool inside( const Position< D >& p ) const;

  bool inside( const Box< D >& b ) const;

  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  Mask< D >* clone() const;

protected:
  Mask< D >* mask1_, *mask2_;
};

/**
 * Mask combining two masks with a minus operation, the difference.
 */
template < int D >
class DifferenceMask : public Mask< D >
{
public:
  using Mask< D >::inside;

  /**
   * Construct the difference of the two given masks. Copies are made
   * of the supplied Mask objects.
   */
  DifferenceMask( const Mask< D >& m1, const Mask< D >& m2 )
    : mask1_( m1.clone() )
    , mask2_( m2.clone() )
  {
  }

  /**
   * Copy constructor
   */
  DifferenceMask( const DifferenceMask& m )
    : Mask< D >( m )
    , mask1_( m.mask1_->clone() )
    , mask2_( m.mask2_->clone() )
  {
  }

  ~DifferenceMask()
  {
    delete mask1_;
    delete mask2_;
  }

  bool inside( const Position< D >& p ) const;

  bool inside( const Box< D >& b ) const;

  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  Mask< D >* clone() const;

protected:
  Mask< D >* mask1_, *mask2_;
};


/**
 * Mask oriented in the opposite direction.
 */
template < int D >
class ConverseMask : public Mask< D >
{
public:
  using Mask< D >::inside;

  /**
   * Construct the converse of the two given mask. A copy is made of the
   * supplied Mask object.
   */
  ConverseMask( const Mask< D >& m )
    : m_( m.clone() )
  {
  }

  /**
   * Copy constructor
   */
  ConverseMask( const ConverseMask& m )
    : Mask< D >( m )
    , m_( m.m_->clone() )
  {
  }

  ~ConverseMask()
  {
    delete m_;
  }

  bool inside( const Position< D >& p ) const;

  bool inside( const Box< D >& b ) const;

  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  Mask< D >* clone() const;

protected:
  Mask< D >* m_;
};


/**
 * Mask shifted by an anchor
 */
template < int D >
class AnchoredMask : public Mask< D >
{
public:
  using Mask< D >::inside;

  /**
   * Construct the converse of the two given mask. A copy is made of the
   * supplied Mask object.
   */
  AnchoredMask( const Mask< D >& m, Position< D > anchor )
    : m_( m.clone() )
    , anchor_( anchor )
  {
  }

  /**
   * Copy constructor
   */
  AnchoredMask( const AnchoredMask& m )
    : Mask< D >( m )
    , m_( m.m_->clone() )
    , anchor_( m.anchor_ )
  {
  }

  ~AnchoredMask()
  {
    delete m_;
  }

  bool inside( const Position< D >& p ) const;

  bool inside( const Box< D >& b ) const;

  bool outside( const Box< D >& b ) const;

  Box< D > get_bbox() const;

  DictionaryDatum get_dict() const;

  Mask< D >* clone() const;

protected:
  Mask< D >* m_;
  Position< D > anchor_;
};

template <>
inline Name
BoxMask< 2 >::get_name()
{
  return names::rectangular;
}

template <>
inline Name
BoxMask< 3 >::get_name()
{
  return names::box;
}

template < int D >
BoxMask< D >::BoxMask( const DictionaryDatum& d )
{
  lower_left_ = getValue< std::vector< double > >( d, names::lower_left );
  upper_right_ = getValue< std::vector< double > >( d, names::upper_right );

  if ( not( lower_left_ < upper_right_ ) )
  {
    throw BadProperty(
      "nest::BoxMask<D>: "
      "Upper right must be strictly to the right and above lower left." );
  }

  if ( d->known( names::azimuth_angle ) )
  {
    azimuth_angle_ = getValue< double >( d, names::azimuth_angle );
  }
  else
  {
    azimuth_angle_ = 0.0;
  }

  if ( d->known( names::polar_angle ) )
  {
    if ( D == 2 )
    {
      throw BadProperty(
        "nest::BoxMask<D>: "
        "polar_angle not defined in 2D." );
    }
    polar_angle_ = getValue< double >( d, names::polar_angle );
  }
  else
  {
    polar_angle_ = 0.0;
  }

  azimuth_cos_ = std::cos( azimuth_angle_ * numerics::pi / 180. );
  azimuth_sin_ = std::sin( azimuth_angle_ * numerics::pi / 180. );
  polar_cos_ = std::cos( polar_angle_ * numerics::pi / 180. );
  polar_sin_ = std::sin( polar_angle_ * numerics::pi / 180. );

  cntr_ = ( upper_right_ + lower_left_ ) * 0.5;
  for ( int i = 0; i != D; ++i )
  {
    eps_[ i ] = 1e-12;
  }

  cntr_x_az_cos_ = cntr_[ 0 ] * azimuth_cos_;
  cntr_x_az_sin_ = cntr_[ 0 ] * azimuth_sin_;
  cntr_y_az_cos_ = cntr_[ 1 ] * azimuth_cos_;
  cntr_y_az_sin_ = cntr_[ 1 ] * azimuth_sin_;
  if ( D == 3 )
  {
    cntr_z_pol_cos_ = cntr_[ 2 ] * polar_cos_;
    cntr_z_pol_sin_ = cntr_[ 2 ] * polar_sin_;
    cntr_x_az_cos_pol_cos_ = cntr_x_az_cos_ * polar_cos_;
    cntr_x_az_cos_pol_sin_ = cntr_x_az_cos_ * polar_sin_;
    cntr_y_az_sin_pol_cos_ = cntr_y_az_sin_ * polar_cos_;
    cntr_y_az_sin_pol_sin_ = cntr_y_az_sin_ * polar_sin_;
    az_cos_pol_cos_ = azimuth_cos_ * polar_cos_;
    az_cos_pol_sin_ = azimuth_cos_ * polar_sin_;
    az_sin_pol_cos_ = azimuth_sin_ * polar_cos_;
    az_sin_pol_sin_ = azimuth_sin_ * polar_sin_;
  }
  else
  {
    cntr_z_pol_cos_ = 0.0;
    cntr_z_pol_sin_ = 0.0;
    cntr_x_az_cos_pol_cos_ = 0.0;
    cntr_x_az_cos_pol_sin_ = 0.0;
    cntr_y_az_sin_pol_cos_ = 0.0;
    cntr_y_az_sin_pol_sin_ = 0.0;
    az_cos_pol_cos_ = 0.0;
    az_cos_pol_sin_ = 0.0;
    az_sin_pol_cos_ = 0.0;
    az_sin_pol_sin_ = 0.0;
  }

  is_rotated_ = azimuth_angle_ != 0.0 or polar_angle_ != 0.0;

  calculate_min_max_values_();
}

template < int D >
inline BoxMask< D >::BoxMask( const Position< D >& lower_left,
  const Position< D >& upper_right,
  const double azimuth_angle,
  const double polar_angle )
  : lower_left_( lower_left )
  , upper_right_( upper_right )
  , azimuth_angle_( azimuth_angle )
  , polar_angle_( polar_angle )
  , azimuth_cos_( std::cos( azimuth_angle_ * numerics::pi / 180. ) )
  , azimuth_sin_( std::sin( azimuth_angle_ * numerics::pi / 180. ) )
  , polar_cos_( std::cos( polar_angle_ * numerics::pi / 180. ) )
  , polar_sin_( std::sin( polar_angle_ * numerics::pi / 180. ) )
  , cntr_( ( upper_right_ + lower_left_ ) * 0.5 )
  , cntr_x_az_cos_( cntr_[ 0 ] * azimuth_cos_ )
  , cntr_x_az_sin_( cntr_[ 0 ] * azimuth_sin_ )
  , cntr_y_az_cos_( cntr_[ 1 ] * azimuth_cos_ )
  , cntr_y_az_sin_( cntr_[ 1 ] * azimuth_sin_ )
{
  if ( D == 2 and not( polar_angle_ == 0.0 ) )
  {
    throw BadProperty(
      "nest::BoxMask<D>: "
      "polar_angle not defined in 2D." );
  }

  for ( int i = 0; i != D; ++i )
  {
    eps_[ i ] = 1e-12;
  }

  if ( D == 3 )
  {
    cntr_z_pol_cos_ = cntr_[ 2 ] * polar_cos_;
    cntr_z_pol_sin_ = cntr_[ 2 ] * polar_sin_;
    cntr_x_az_cos_pol_cos_ = cntr_x_az_cos_ * polar_cos_;
    cntr_x_az_cos_pol_sin_ = cntr_x_az_cos_ * polar_sin_;
    cntr_y_az_sin_pol_cos_ = cntr_y_az_sin_ * polar_cos_;
    cntr_y_az_sin_pol_sin_ = cntr_y_az_sin_ * polar_sin_;
    az_cos_pol_cos_ = azimuth_cos_ * polar_cos_;
    az_cos_pol_sin_ = azimuth_cos_ * polar_sin_;
    az_sin_pol_cos_ = azimuth_sin_ * polar_cos_;
    az_sin_pol_sin_ = azimuth_sin_ * polar_sin_;
  }
  else
  {
    cntr_z_pol_cos_ = 0.0;
    cntr_z_pol_sin_ = 0.0;
    cntr_x_az_cos_pol_cos_ = 0.0;
    cntr_x_az_cos_pol_sin_ = 0.0;
    cntr_y_az_sin_pol_cos_ = 0.0;
    cntr_y_az_sin_pol_sin_ = 0.0;
    az_cos_pol_cos_ = 0.0;
    az_cos_pol_sin_ = 0.0;
    az_sin_pol_cos_ = 0.0;
    az_sin_pol_sin_ = 0.0;
  }

  is_rotated_ = azimuth_angle_ != 0.0 or polar_angle_ != 0.0;

  calculate_min_max_values_();
}

template <>
inline Name
BallMask< 2 >::get_name()
{
  return names::circular;
}

template <>
inline Name
BallMask< 3 >::get_name()
{
  return names::spherical;
}

template < int D >
BallMask< D >::BallMask( const DictionaryDatum& d )
{
  radius_ = getValue< double >( d, names::radius );
  if ( radius_ <= 0 )
  {
    throw BadProperty(
      "nest::BallMask<D>: "
      "radius > 0 required." );
  }

  if ( d->known( names::anchor ) )
  {
    center_ = getValue< std::vector< double > >( d, names::anchor );
  }
}

template <>
inline Name
EllipseMask< 2 >::get_name()
{
  return names::elliptical;
}

template <>
inline Name
EllipseMask< 3 >::get_name()
{
  return names::ellipsoidal;
}

template < int D >
EllipseMask< D >::EllipseMask( const DictionaryDatum& d )
{
  major_axis_ = getValue< double >( d, names::major_axis );
  minor_axis_ = getValue< double >( d, names::minor_axis );
  if ( major_axis_ <= 0 or minor_axis_ <= 0 )
  {
    throw BadProperty(
      "nest::EllipseMask<D>: "
      "All axis > 0 required." );
  }
  if ( major_axis_ < minor_axis_ )
  {
    throw BadProperty(
      "nest::EllipseMask<D>: "
      "major_axis greater than minor_axis required." );
  }

  x_scale_ = 4.0 / ( major_axis_ * major_axis_ );
  y_scale_ = 4.0 / ( minor_axis_ * minor_axis_ );

  if ( d->known( names::polar_axis ) )
  {
    if ( D == 2 )
    {
      throw BadProperty(
        "nest::EllipseMask<D>: "
        "polar_axis not defined in 2D." );
    }
    polar_axis_ = getValue< double >( d, names::polar_axis );

    if ( polar_axis_ <= 0 )
    {
      throw BadProperty(
        "nest::EllipseMask<D>: "
        "All axis > 0 required." );
    }

    z_scale_ = 4.0 / ( polar_axis_ * polar_axis_ );
  }
  else
  {
    polar_axis_ = 0.0;
    z_scale_ = 0.0;
  }

  if ( d->known( names::anchor ) )
  {
    center_ = getValue< std::vector< double > >( d, names::anchor );
  }

  if ( d->known( names::azimuth_angle ) )
  {
    azimuth_angle_ = getValue< double >( d, names::azimuth_angle );
  }
  else
  {
    azimuth_angle_ = 0.0;
  }

  if ( d->known( names::polar_angle ) )
  {
    if ( D == 2 )
    {
      throw BadProperty(
        "nest::EllipseMask<D>: "
        "polar_angle not defined in 2D." );
    }
    polar_angle_ = getValue< double >( d, names::polar_angle );
  }
  else
  {
    polar_angle_ = 0.0;
  }

  azimuth_cos_ = std::cos( azimuth_angle_ * numerics::pi / 180. );
  azimuth_sin_ = std::sin( azimuth_angle_ * numerics::pi / 180. );
  polar_cos_ = std::cos( polar_angle_ * numerics::pi / 180. );
  polar_sin_ = std::sin( polar_angle_ * numerics::pi / 180. );

  create_bbox_();
}

} // namespace nest

#endif
