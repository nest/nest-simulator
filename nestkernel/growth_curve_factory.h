/*
 *  growth_curve_factory.h
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

#ifndef GROWTH_CURVE_FACTORY_H
#define GROWTH_CURVE_FACTORY_H

// Includes from nestkernel:
#include "growth_curve.h"

namespace nest
{

class GrowthCurveGaussian;
class GrowthCurveLinear;
class GrowthCurveSigmoid;

/**
 * Generic factory class for GrowthCurve objects.
 *
 * This factory allows for flexible registration
 * of GrowthCurve subclasses and object creation.
 *
 */
class GenericGrowthCurveFactory
{
public:
  virtual ~GenericGrowthCurveFactory()
  {
  }
  virtual GrowthCurve* create() const = 0;
};

/**
 * Factory class for generating objects of type GrowthCurve
 */

template < typename GrowthCurveType >
class GrowthCurveFactory : public GenericGrowthCurveFactory
{

public:
  GrowthCurve*
  create() const
  {
    return new GrowthCurveType();
  }
};

} // namespace nest

#endif
