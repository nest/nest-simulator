/*
 *  connection_generator.h
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

#ifndef CONNECTION_GENERATOR_H
#define CONNECTION_GENERATOR_H

#define CONNECTION_GENERATOR_DEBUG 1

#include <vector>

/**
 * Pure abstract base class for connection generators.
 */
class ConnectionGenerator {
 public:
  class ClosedInterval {
  public:
    ClosedInterval (int _first, int _last) : first (_first), last (_last) { }
    int first;
    int last;
  };

  class IntervalSet {
    std::vector<ClosedInterval> ivals;
    int _skip;
  public:
    IntervalSet (int skip = 1) : ivals (), _skip (skip) { }
    typedef std::vector<ClosedInterval>::iterator iterator;
    iterator begin () { return ivals.begin (); }
    iterator end () { return ivals.end (); }
    int skip () { return _skip; }
    void setSkip (int skip) { _skip = skip; }
    void insert (int first, int last) {
      ivals.push_back (ClosedInterval (first, last));
    }
  };

  class Mask {
  public:
    Mask (int sourceSkip = 1, int targetSkip = 1)
      : sources (sourceSkip), targets (targetSkip) { }
    IntervalSet sources;
    IntervalSet targets;
  };

  virtual ~ConnectionGenerator ();

  /**
   * Return the number of values associated with this generator
   */
  virtual int arity () = 0;

  /**
   * Inform the generator of which source and target indexes exist
   * (must always be called before any of the methods below)
   *
   * skip can be used in round-robin allocation schemes.
   */
  virtual void setMask (Mask& mask);

  /**
   * For a parallel simulator, we want to know the masks for all ranks
   */
  virtual void setMask (std::vector<Mask>& masks, int local) = 0;

  /**
   * Return number of connections represented by this generator
   */
  virtual int size ();

  /**
   * Start an iteration (must be called before first next)
   */
  virtual void start () = 0;

  /**
   * Advance to next connection or return false
   */
  virtual bool next (int& source, int& target, double* value) = 0;
};

#ifdef CONNECTION_GENERATOR_DEBUG
ConnectionGenerator* makeDummyConnectionGenerator ();
#endif

#endif /* #ifndef CONNECTION_GENERATOR_H */
