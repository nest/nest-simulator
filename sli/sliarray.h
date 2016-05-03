/*
 *  sliarray.h
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

#ifndef SLIARRAY_H
#define SLIARRAY_H
/*
    SLI's array access functions
*/

// Includes from sli:
#include "slifunction.h"
#include "slimodule.h"

/**
 * SLI module defining array functions.
 * This class implements the SLI functions which operate on SLI arrays.
 */
class SLIArrayModule : public SLIModule
{
  /**
   * @defgroup sliarray SLI array functions
   * SLI functions that operate on SLI arrays.
   * @{
   */
  class MapFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class IMapFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };
  class IMap_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };
  class IMap_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };
  class MapThreadFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class IMapThreadFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };

  class MapIndexedFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class IMapIndexedFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };

  class RangeFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class ArraystoreFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class ArrayloadFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class ArraycreateFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class ReverseFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class RotateFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class FlattenFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class SortFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class UniqueFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class TransposeFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };


  class PartitionFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class ValidFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Put_a_a_tFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  /**
   * Return array of indices defining a 2d subarea of a 2d array.
   *
   * Given a -- hypothetical -- twodimensional array,
   * "area" tells you, what indices you need to
   * subscript a contiguous, twodimensional subarea.
   * Returns 1-d indices.
   *
   * For further information refer to the SLI documentation.
   *
   * @par Synopsis:
   *                    source_width source_anchor_y source_anchor_x
   *        area_height   area_width   area_anchor_y   area_anchor_x
   *                                                   area -> *[1d-indices]
   *
   * @param source_width    width  of the (hypothetical) source
   *                        array to be subscribed into
   * @param source_anchor_y y position of the anchor point relative
   *                        to ORIGIN OF THE SOURCE ARRAY
   * @param source_anchor_x x position of the anchor point relative
   *                        to ORIGIN OF THE SOURCE ARRAY
   *
   * @param area_height     height of the subarea to be subscribed
   * @param area_width      width  of the subarea to be subscribed
   * @param area_anchor_y   y position of the anchor point relative
   *                        to ORIGIN OF THE SUBAREA
   * @param area_anchor_x   x position of the anchor point relative
   *                        to ORIGIN OF THE SUBAREA
   *
   * @return \a [1d-indices] flat integer array containing the indices
   *                         that can be used to subscript the
   *                         (hypothetical) source array in order to
   *                         access the desired subarea.
   */
  class AreaFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  /**
   * Return array of indices defining a 2d subarea of a 2d array.
   *
   * Given a -- hypothetical -- twodimensional array,
   * "area" tells you, what indices you need to
   * subscript a contiguous, twodimensional subarea.
   * Returns 2-d indices.
   *
   * For further information refer to the SLI documentation.
   *
   * @par Synopsis:
   *                                 source_anchor_y source_anchor_x
   *        area_height   area_width   area_anchor_y   area_anchor_x
   *                                                   area -> *[1d-indices]
   *
   * @param source_anchor_y y position of the anchor point relative
   *                        to ORIGIN OF THE SOURCE ARRAY
   * @param source_anchor_x x position of the anchor point relative
   *                        to ORIGIN OF THE SOURCE ARRAY
   *
   * @param area_height     height of the subarea to be subscribed
   * @param area_width      width  of the subarea to be subscribed
   * @param area_anchor_y   y position of the anchor point relative
   *                        to ORIGIN OF THE SUBAREA
   * @param area_anchor_x   x position of the anchor point relative
   *                        to ORIGIN OF THE SUBAREA
   *
   * @return \a [2d-indices] flat integer array containing the indices
   *                         that can be used to subscript the
   *                         (hypothetical) source array in order to
   *                         access the desired subarea.
   */
  class Area2Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Cv1dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Cv2dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class GetMaxFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class GetMinFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };


  /**
   * Generate two-dimensional array with Gabor patch.
   */
  class GaborFunction : public SLIFunction
  {
  public:
    GaborFunction()
    {
    }

    void execute( SLIInterpreter* ) const;
  };

  /**
   * Generate two-dimensional array with Gauss patch.
   */
  class Gauss2dFunction : public SLIFunction
  {
  public:
    Gauss2dFunction()
    {
    }

    void execute( SLIInterpreter* ) const;
  };


  /**
   * Convert SLI array to std::vector.
   */

  class Array2IntVectorFunction : public SLIFunction
  {
  public:
    Array2IntVectorFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  /**
   * Convert SLI array to std::vector.
   */

  class Array2DoubleVectorFunction : public SLIFunction
  {
  public:
    Array2DoubleVectorFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class DoubleVector2ArrayFunction : public SLIFunction
  {
  public:
    DoubleVector2ArrayFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class IntVector2ArrayFunction : public SLIFunction
  {
  public:
    IntVector2ArrayFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class Add_iv_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Add_i_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Neg_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Sub_iv_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Mul_iv_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Mul_i_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Mul_d_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Div_iv_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Add_dv_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Add_d_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Sub_dv_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Mul_dv_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Mul_d_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Div_dv_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Neg_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Inv_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Length_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Length_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Get_iv_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Get_iv_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Get_dv_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Get_dv_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Put_dv_i_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Put_iv_i_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Zeros_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Zeros_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Ones_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class Ones_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class ArangeFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  /**
   * Test single double for finiteness.
   * @todo This class does not really belong into sliarray, but is placed
   * here since it is a Mathematica-style Q function.
   */
  class FiniteQ_dFunction : public SLIFunction
  {
  public:
    FiniteQ_dFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class Forall_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Forall_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Iforall_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };

  class Iforall_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
    void backtrace( SLIInterpreter*, int ) const;
  };


  class Eq_dvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class Eq_ivFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  /** @} */

  RangeFunction rangefunction;
  ArraystoreFunction arraystorefunction;
  ArraycreateFunction arraycreatefunction;
  ArrayloadFunction arrayloadfunction;
  ReverseFunction reversefunction;
  RotateFunction rotatefunction;
  FlattenFunction flattenfunction;
  SortFunction sortfunction;
  TransposeFunction transposefunction;
  MapFunction mapfunction;
  IMapFunction imapfunction;
  IMap_ivFunction imap_ivfunction;
  IMap_dvFunction imap_dvfunction;
  MapIndexedFunction mapindexedfunction;
  IMapIndexedFunction imapindexedfunction;
  MapThreadFunction mapthreadfunction;
  IMapThreadFunction imapthreadfunction;
  PartitionFunction partitionfunction;
  ValidFunction validfunction;
  AreaFunction areafunction;
  Area2Function area2function;
  Cv1dFunction cv1dfunction;
  Cv2dFunction cv2dfunction;
  GetMaxFunction getmaxfunction;
  GetMinFunction getminfunction;
  GaborFunction gaborfunction;
  Gauss2dFunction gauss2dfunction;
  Put_a_a_tFunction put_a_a_tfunction;
  Array2IntVectorFunction array2intvectorfunction;
  Array2DoubleVectorFunction array2doublevectorfunction;
  IntVector2ArrayFunction intvector2arrayfunction;
  DoubleVector2ArrayFunction doublevector2arrayfunction;
  FiniteQ_dFunction finiteq_dfunction;
  Forall_ivFunction forall_ivfunction;
  Iforall_ivFunction iforall_ivfunction;
  Forall_dvFunction forall_dvfunction;
  Iforall_dvFunction iforall_dvfunction;


  Add_iv_ivFunction add_iv_ivfunction;
  Add_i_ivFunction add_i_ivfunction;
  Neg_ivFunction neg_ivfunction;
  Sub_iv_ivFunction sub_iv_ivfunction;
  Mul_iv_ivFunction mul_iv_ivfunction;
  Mul_i_ivFunction mul_i_ivfunction;
  Mul_d_ivFunction mul_d_ivfunction;
  Div_iv_ivFunction div_iv_ivfunction;
  Length_ivFunction length_ivfunction;

  Add_dv_dvFunction add_dv_dvfunction;
  Add_d_dvFunction add_d_dvfunction;
  Sub_dv_dvFunction sub_dv_dvfunction;
  Mul_dv_dvFunction mul_dv_dvfunction;
  Mul_d_dvFunction mul_d_dvfunction;
  Div_dv_dvFunction div_dv_dvfunction;
  Neg_dvFunction neg_dvfunction;
  Inv_dvFunction inv_dvfunction;
  Length_dvFunction length_dvfunction;
  Get_dv_iFunction get_dv_ifunction;
  Get_dv_ivFunction get_dv_ivfunction;
  Get_iv_iFunction get_iv_ifunction;
  Get_iv_ivFunction get_iv_ivfunction;
  Put_iv_i_iFunction put_iv_i_ifunction;
  Put_dv_i_dFunction put_dv_i_dfunction;
  Zeros_dvFunction zeros_dvfunction;
  Ones_dvFunction ones_dvfunction;
  Eq_dvFunction eq_dvfunction;
  Eq_ivFunction eq_ivfunction;
  Zeros_ivFunction zeros_ivfunction;
  Ones_ivFunction ones_ivfunction;
  ArangeFunction arangefunction;

public:
  SLIArrayModule()
  {
  }

  void init( SLIInterpreter* );
  const std::string commandstring( void ) const;
  const std::string name( void ) const;
};


#endif
