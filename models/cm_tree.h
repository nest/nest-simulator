/*
 *  cm_tree.h
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

#ifndef CM_TREE_H
#define CM_TREE_H

#include <stdlib.h>

#include "nest_time.h"
#include "ring_buffer.h"

// compartmental model
#include "cm_compartmentcurrents.h"

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"


namespace nest
{

class Compartment
{
private:
  //! aggragators for numerical integration
  double xx_;
  double yy_;

public:
  //! compartment index
  long comp_index;
  //! parent compartment index
  long p_index;
  //! tree structure indices
  Compartment* parent;
  std::vector< Compartment > children;

  //! buffer for currents
  RingBuffer currents;
  //! electrical parameters
  double ca; // compartment capacitance [uF]
  double gc; // coupling conductance with parent (meaningless if root) [uS]
  double gl; // leak conductance of compartment [uS]
  double el; // leak current reversal potential [mV]

  //! voltage variable
  double v_comp;

  //! auxiliary variables for efficienchy
  double gg0;
  double ca__div__dt;
  double gl__div__2;
  double gc__div__2;
  double gl__times__el;
  //! for numerical integration
  double ff;
  double gg;
  double hh;
  //! passage counter for recursion
  int n_passed;

  //! vector for synapses
  CompartmentCurrents compartment_currents;

  Compartment( const long compartment_index, const long parent_index );
  Compartment( const long compartment_index, const long parent_index, const DictionaryDatum& compartment_params );
  ~Compartment() {};

  // initialization
  void pre_run_hook();
  std::map< Name, double* > get_recordables();

  //! matrix construction
  void construct_matrix_element( const long lag );

  //! maxtrix inversion
  inline void gather_input( const std::pair< double, double >& in );
  inline std::pair< double, double > io();
  inline double calc_v( const double v_in );
}; // Compartment


/*
Short helper functions for solving the matrix equation. Can hopefully be inlined
*/
inline void
nest::Compartment::gather_input( const std::pair< double, double >& in )
{
  xx_ += in.first;
  yy_ += in.second;
}

inline std::pair< double, double >
nest::Compartment::io()
{
  // include inputs from child compartments
  gg -= xx_;
  ff -= yy_;

  // output values
  double g_val( hh * hh / gg );
  double f_val( ff * hh / gg );

  return std::make_pair( g_val, f_val );
}

inline double
nest::Compartment::calc_v( const double v_in )
{
  // reset recursion variables
  xx_ = 0.0;
  yy_ = 0.0;

  // compute voltage
  v_comp = ( ff - v_in * hh ) / gg;

  return v_comp;
}


class CompTree
{
private:
  /**
  structural data containers for the compartment model
  */
  mutable Compartment root_;
  std::vector< long > compartment_indices_;
  std::vector< Compartment* > compartments_;
  std::vector< Compartment* > leafs_;

  long size_ = 0;

  //! recursion functions for matrix inversion
  void solve_matrix_downsweep( Compartment* compartment_ptr, std::vector< Compartment* >::iterator leaf_it );
  void solve_matrix_upsweep( Compartment* compartment, double vv );

  //! functions for pointer initialization
  void set_parents();
  void set_compartments();
  void set_leafs();

public:
  CompTree();
  ~CompTree() {};

  //! add a compartment to the tree structure
  void add_compartment( const long parent_index );
  void add_compartment( const long parent_index, const DictionaryDatum& compartment_params );
  void add_compartment( Compartment* compartment, const long parent_index );

  //! initialize the tree for simulation
  void pre_run_hook();

  //! fix all pointers in the tree, the tree structure should not be modified between calling
  //! this function and starting the simulation
  void init_pointers();

  //! associate each receptor with a spike buffer
  void set_syn_buffers( std::vector< RingBuffer >& syn_buffers );

  //! make all state variables accessible for recording
  std::map< Name, double* > get_recordables();

  //! get a compartment pointer from the tree
  Compartment* get_compartment( const long compartment_index ) const;
  Compartment* get_compartment( const long compartment_index, Compartment* compartment, const long raise_flag ) const;
  Compartment* get_compartment_opt( const long compartment_indx ) const;
  Compartment*
  get_root() const
  {
    return &root_;
  };

  //! get tree size (number of compartments)
  long
  get_size() const
  {
    return size_;
  };

  //! get voltage values
  std::vector< double > get_voltage() const;
  double get_compartment_voltage( const long compartment_index );

  //! construct the numerical integration matrix and vector
  void construct_matrix( const long lag );
  //! solve the matrix equation for next timestep voltage
  void solve_matrix();

  //! print function
  void print_tree() const;
}; // CompTree

} // namespace

#endif /* #ifndef CM_TREE_H */
