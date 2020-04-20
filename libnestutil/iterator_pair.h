/*
 *  iterator_pair.h
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

#ifndef ITERATOR_PAIR_H_
#define ITERATOR_PAIR_H_

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/tuple/tuple.hpp>

#include "source.h"

namespace boost
{
namespace tuples
{
/**
 * @brief Exchanges values of two two-element tuples.
 * @tparam T Type of the first value in the tuples.
 * @tparam U Type of the second value in the tuples.
 */
template < typename T, typename U >
inline void
swap( boost::tuple< T&, U& > a, boost::tuple< T&, U& > b ) noexcept
{
  using std::swap;
  swap( boost::get< 0 >( a ), boost::get< 0 >( b ) );
  swap( boost::get< 1 >( a ), boost::get< 1 >( b ) );
}

/**
 * @brief Less than relational operator for Boost tuples.
 * @param lhs First operand.
 * @param rhs Second operand.
 *
 * Modified operator, based on the less than operator from the Boost
 * library. Have to implement a custom operator to only compare the
 * first values of the tuples.
 */
template < class T1, class T2, class S1, class S2 >
inline bool operator<( const cons< T1, T2 >& lhs, const cons< S1, S2 >& rhs )
{
  // check that tuple lengths are equal
  BOOST_STATIC_ASSERT( length< T2 >::value == length< S2 >::value );

  return lhs.get_head() < rhs.get_head();
}
} // namespace tuples
} // namespace boost

/**
 * @brief Helper class holding type definitions.
 * @tparam sort_iter_type_ Iterator type of the container being sorted.
 * @tparam perm_iter_type_ Iterator type of the container being permuted.
 */
template < class sort_iter_type_, class perm_iter_type_ >
struct iterator_pair_types
{
  using value_type = boost::tuple< typename std::iterator_traits< sort_iter_type_ >::value_type,
    typename std::iterator_traits< perm_iter_type_ >::value_type >;
  using ref_type = boost::tuple< typename std::iterator_traits< sort_iter_type_ >::reference,
    typename std::iterator_traits< perm_iter_type_ >::reference >;
  using difference_type = typename std::iterator_traits< sort_iter_type_ >::difference_type;
};

/**
 * @brief A combinator of two iterators that can be used to apply
 *        permutations on a second container when sorting the first.
 * @tparam sort_iter_type_ Iterator type of the container being sorted.
 * @tparam perm_iter_type_ Iterator type of the container being permuted.
 */
template < typename sort_iter_type_, typename perm_iter_type_ >
class IteratorPair : public boost::iterator_facade< IteratorPair< sort_iter_type_, perm_iter_type_ >,
                       typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::value_type,
                       std::random_access_iterator_tag,
                       typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::ref_type,
                       typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::difference_type >
{
public:
  IteratorPair() = default;
  IteratorPair( sort_iter_type_, perm_iter_type_ );

private:
  friend class boost::iterator_core_access;

  //! Iterator of the container being sorted.
  sort_iter_type_ sort_iter_;
  //! Iterator of the container being permuted.
  perm_iter_type_ perm_iter_;

  // The following methods are required as building blocks by
  // Boost's iterator_facade.

  /**
   * @brief Advance by one position.
   */
  void increment();

  /**
   * @brief Retreat by one position.
   */
  void decrement();

  /**
   * @brief Compare for equality.
   */
  bool equal( IteratorPair const& ) const;

  /**
   * @brief Advance by a number of positions.
   */
  void advance( typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::difference_type );

  /**
   * @brief Access the value referred to.
   * @return A boost::tuple with two values.
   */
  typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::ref_type dereference() const;

  /**
   * @brief Measure the distance to another iterator.
   */
  typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::difference_type distance_to(
    IteratorPair const& ) const;
};

/**
 * @brief Creates an IteratorPair object, deducing iterator types from the
 *        types of arguments.
 * @tparam sort_iter_type_ Iterator type of the container being sorted.
 * @tparam perm_iter_type_ Iterator type of the container being permuted.
 */
template < class sort_iter_type_, class perm_iter_type_ >
IteratorPair< sort_iter_type_, perm_iter_type_ >
make_iterator_pair( sort_iter_type_ sort_iter, perm_iter_type_ perm_iter )
{
  return { sort_iter, perm_iter };
}

/**
 * @brief A rightshift functor for tuples to be used with Boost's sorting
 *        function.
 */
struct rightshift_iterator_pair
{
  template < typename T >
  inline int operator()( boost::tuples::tuple< int&, T& > s, unsigned offset )
  {
    return boost::get< 0 >( s ) >> offset;
  }

  template < typename T >
  inline int operator()( boost::tuples::tuple< nest::Source&, T& > s, unsigned offset )
  {
    return boost::get< 0 >( s ).get_gid() >> offset;
  }
};

template < typename sort_iter_type_, typename perm_iter_type_ >
inline IteratorPair< sort_iter_type_, perm_iter_type_ >::IteratorPair( sort_iter_type_ sort_iter,
  perm_iter_type_ perm_iter )
  : sort_iter_( sort_iter )
  , perm_iter_( perm_iter )
{
}

template < typename sort_iter_type_, typename perm_iter_type_ >
inline void
IteratorPair< sort_iter_type_, perm_iter_type_ >::increment()
{
  ++sort_iter_;
  ++perm_iter_;
}

template < typename sort_iter_type_, typename perm_iter_type_ >
inline void
IteratorPair< sort_iter_type_, perm_iter_type_ >::decrement()
{
  --sort_iter_;
  --perm_iter_;
}

template < typename sort_iter_type_, typename perm_iter_type_ >
inline bool
IteratorPair< sort_iter_type_, perm_iter_type_ >::equal( IteratorPair const& other ) const
{
  return ( sort_iter_ == other.sort_iter_ );
}

template < typename sort_iter_type_, typename perm_iter_type_ >
inline void
IteratorPair< sort_iter_type_, perm_iter_type_ >::advance(
  typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::difference_type n )
{
  sort_iter_ += n;
  perm_iter_ += n;
}

template < typename sort_iter_type_, typename perm_iter_type_ >
inline typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::ref_type
IteratorPair< sort_iter_type_, perm_iter_type_ >::dereference() const
{
  return ( typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::ref_type( *sort_iter_, *perm_iter_ ) );
}

template < typename sort_iter_type_, typename perm_iter_type_ >
inline typename iterator_pair_types< sort_iter_type_, perm_iter_type_ >::difference_type
IteratorPair< sort_iter_type_, perm_iter_type_ >::distance_to( IteratorPair const& other ) const
{
  return ( other.sort_iter_ - sort_iter_ );
}

#endif
