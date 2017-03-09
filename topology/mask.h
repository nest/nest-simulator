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

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

// Includes from topology:
#include "position.h"
#include "topology_names.h"
#include "topologymodule.h"

namespace nest
{
class TopologyModule;

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

typedef lockPTRDatum< AbstractMask, &TopologyModule::MaskType > MaskDatum;

/**
 * Abstract base class for masks with given dimension.
 */
template < int D >
class Mask : public AbstractMask
{
public:
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
   * lower_left  - Position of lower left corner (array of doubles)
   * upper_right - Position of upper right corner (array of doubles)
   */
  BoxMask( const DictionaryDatum& );

  BoxMask( const Position< D >& lower_left, const Position< D >& upper_right );

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
  Position< D > lower_left_;
  Position< D > upper_right_;
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
 * Mask combining two masks with a Boolean AND, the intersection.
 */
template < int D >
class IntersectionMask : public Mask< D >
{
public:
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
      "topology::BoxMask<D>: "
      "Upper right must be strictly to the right and above lower left." );
  }
}

template < int D >
inline BoxMask< D >::BoxMask( const Position< D >& lower_left,
  const Position< D >& upper_right )
  : lower_left_( lower_left )
  , upper_right_( upper_right )
{
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
      "topology::BallMask<D>: "
      "radius > 0 required." );
  }

  if ( d->known( names::anchor ) )
  {
    center_ = getValue< std::vector< double > >( d, names::anchor );
  }
}

} // namespace nest

#endif
