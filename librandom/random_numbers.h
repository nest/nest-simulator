/*
 *  random_numbers.h
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

#ifndef RANDOM_NUMBERS_H
#define RANDOM_NUMBERS_H

// C++ includes:
#include <string>

// Includes from sli:
#include "slimodule.h"
#include "slitype.h"

class Dictionary;

/**
 *  Provides random numbers and deviates to SLI.
 */
class RandomNumbers : public SLIModule
{
public:
  static SLIType RngType;        // actual RNG
  static SLIType RngFactoryType; // random generator factory

  static SLIType RdvType;        // random deviate generator
  static SLIType RdvFactoryType; // random deviate generator factory


  RandomNumbers(){};
  ~RandomNumbers();

  const std::string
  name( void ) const
  {
    return "RandomNumbers";
  }

  const std::string
  commandstring( void ) const
  {
    return std::string( "(librandom) run" );
  }

  /**
   * Initializes the random number module.
   * The random number generator
   * and the random deviate generator dictionaries are set up.
   */
  void init( SLIInterpreter* );

  //! Returns global random number generator dictionary
  static const Dictionary& get_rngdict();

  //! Returns global random deviate generator dictionary
  static const Dictionary& get_rdvdict();

  // RNG creation function
  class CreateRNGFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  // RNG creation function
  class CreateRDVFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class SetStatus_vdFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class GetStatus_vFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  // RNG access functions
  class IrandFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class DrandFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class SeedFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class RandomArrayFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class RandomFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };


  // create function
  CreateRNGFunction createrngfunction;
  CreateRDVFunction createrdvfunction;

  // set/get functions
  SetStatus_vdFunction setstatus_vdfunction;
  GetStatus_vFunction getstatus_vfunction;

  // access functions
  SeedFunction seedfunction;
  IrandFunction irandfunction;
  DrandFunction drandfunction;

  RandomArrayFunction randomarrayfunction;
  RandomFunction randomfunction;

private:
  //! Utility function for registering number generators
  template < typename NumberGenerator >
  void register_rng_( const std::string& name, Dictionary& dict );

  //! Utility function for registering deviate generators
  template < typename DeviateGenerator >
  void register_rdv_( const std::string& name, Dictionary& dict );

  static Dictionary* rngdict_; //!< manages random number generators
  static Dictionary* rdvdict_; //!< manages random deviate generators
};

inline const Dictionary&
RandomNumbers::get_rngdict()
{
  assert( rngdict_ );
  return *rngdict_;
}

inline const Dictionary&
RandomNumbers::get_rdvdict()
{
  assert( rdvdict_ );
  return *rdvdict_;
}

#endif
