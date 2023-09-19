/*
 *  slimath.h
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

#ifndef SLIMATH_H
#define SLIMATH_H
/*
    SLI's math operators
*/

// Includes from sli:
#include "interpret.h"

void init_slimath( SLIInterpreter* );

class Add_diFunction : public SLIFunction
{
public:
  Add_diFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class Add_iiFunction : public SLIFunction
{
public:
  Add_iiFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class Add_idFunction : public SLIFunction
{
public:
  Add_idFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class Add_ddFunction : public SLIFunction
{
public:
  Add_ddFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};
//---------------------------------------

class Sub_diFunction : public SLIFunction
{
public:
  Sub_diFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class Sub_iiFunction : public SLIFunction
{
public:
  Sub_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Sub_idFunction : public SLIFunction
{
public:
  Sub_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Sub_ddFunction : public SLIFunction
{
public:
  Sub_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
//---------------------------------------

class Mul_diFunction : public SLIFunction
{
public:
  Mul_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Mul_iiFunction : public SLIFunction
{
public:
  Mul_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Mul_idFunction : public SLIFunction
{
public:
  Mul_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Mul_ddFunction : public SLIFunction
{
public:
  Mul_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
//---------------------------------------

class Div_diFunction : public SLIFunction
{
public:
  Div_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Div_iiFunction : public SLIFunction
{
public:
  Div_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Div_idFunction : public SLIFunction
{
public:
  Div_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Div_ddFunction : public SLIFunction
{
public:
  Div_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

//--------------------------------------

class Mod_iiFunction : public SLIFunction
{
public:
  Mod_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
//--------------------------------------


class Sin_dFunction : public SLIFunction
{
public:
  Sin_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Asin_dFunction : public SLIFunction
{
public:
  Asin_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Cos_dFunction : public SLIFunction
{
public:
  Cos_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Acos_dFunction : public SLIFunction
{
public:
  Acos_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Exp_dFunction : public SLIFunction
{
public:
  Exp_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Ln_dFunction : public SLIFunction
{
public:
  Ln_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Log_dFunction : public SLIFunction
{
public:
  Log_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Sqr_dFunction : public SLIFunction
{
public:
  Sqr_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Sqrt_dFunction : public SLIFunction
{
public:
  Sqrt_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Pow_ddFunction : public SLIFunction
{
public:
  Pow_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Pow_diFunction : public SLIFunction
{
public:
  Pow_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Modf_dFunction : public SLIFunction
{
public:
  Modf_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Frexp_dFunction : public SLIFunction
{
public:
  Frexp_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Ldexp_diFunction : public SLIFunction
{
public:
  Ldexp_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};


class Dexp_iFunction : public SLIFunction
{
public:
  Dexp_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};


//--------------------------------------
class Abs_iFunction : public SLIFunction
{
public:
  Abs_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Abs_dFunction : public SLIFunction
{
public:
  Abs_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

//--------------------------------------

class Neg_iFunction : public SLIFunction
{
public:
  Neg_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Neg_dFunction : public SLIFunction
{
public:
  Neg_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Inv_dFunction : public SLIFunction
{
public:
  Inv_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

//-------------------------

class EqFunction : public SLIFunction
{
public:
  EqFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class NeqFunction : public SLIFunction
{
public:
  NeqFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class AndFunction : public SLIFunction
{
public:
  AndFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class OrFunction : public SLIFunction
{
public:
  OrFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class And_iiFunction : public SLIFunction
{
public:
  And_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Or_iiFunction : public SLIFunction
{
public:
  Or_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class XorFunction : public SLIFunction
{
public:
  XorFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Not_bFunction : public SLIFunction
{
public:
  Not_bFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Not_iFunction : public SLIFunction
{
public:
  Not_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Geq_iiFunction : public SLIFunction
{
public:
  Geq_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Geq_idFunction : public SLIFunction
{
public:
  Geq_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Geq_diFunction : public SLIFunction
{
public:
  Geq_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Geq_ddFunction : public SLIFunction
{
public:
  Geq_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Leq_iiFunction : public SLIFunction
{
public:
  Leq_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Leq_idFunction : public SLIFunction
{
public:
  Leq_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Leq_diFunction : public SLIFunction
{
public:
  Leq_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Leq_ddFunction : public SLIFunction
{
public:
  Leq_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};


//-------------------------

class Gt_iiFunction : public SLIFunction
{
public:
  Gt_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Gt_ddFunction : public SLIFunction
{
public:
  Gt_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Gt_idFunction : public SLIFunction
{
public:
  Gt_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Gt_diFunction : public SLIFunction
{
public:
  Gt_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Gt_ssFunction : public SLIFunction
{
public:
  Gt_ssFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Lt_iiFunction : public SLIFunction
{
public:
  Lt_iiFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Lt_ddFunction : public SLIFunction
{
public:
  Lt_ddFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Lt_idFunction : public SLIFunction
{
public:
  Lt_idFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Lt_diFunction : public SLIFunction
{
public:
  Lt_diFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Lt_ssFunction : public SLIFunction
{
public:
  Lt_ssFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class IntegerFunction : public SLIFunction
{
public:
  IntegerFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class DoubleFunction : public SLIFunction
{
public:
  DoubleFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class UnitStep_iFunction : public SLIFunction
{
public:
  UnitStep_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class UnitStep_dFunction : public SLIFunction
{
public:
  UnitStep_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class UnitStep_iaFunction : public SLIFunction
{
public:
  UnitStep_iaFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class UnitStep_daFunction : public SLIFunction
{
public:
  UnitStep_daFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Round_dFunction : public SLIFunction
{
public:
  Round_dFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class Floor_dFunction : public SLIFunction
{
public:
  Floor_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

class Ceil_dFunction : public SLIFunction
{
public:
  Ceil_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};


class Max_i_iFunction : public SLIFunction
{
public:
  Max_i_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Max_i_dFunction : public SLIFunction
{
public:
  Max_i_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Max_d_iFunction : public SLIFunction
{
public:
  Max_d_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Max_d_dFunction : public SLIFunction
{
public:
  Max_d_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};


class Min_i_iFunction : public SLIFunction
{
public:
  Min_i_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Min_i_dFunction : public SLIFunction
{
public:
  Min_i_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Min_d_iFunction : public SLIFunction
{
public:
  Min_d_iFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};
class Min_d_dFunction : public SLIFunction
{
public:
  Min_d_dFunction()
  {
  }

  void execute( SLIInterpreter* ) const override;
};

#endif
