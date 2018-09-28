/*
 *  seque.h
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

#ifndef SEQUE_H_
#define SEQUE_H_

#include <cmath>
#include <vector>
#include <iostream>
#include <iterator>
#include <cassert>

template< typename _value_type > class Seque;
template< typename _value_type, typename _ref, typename _ptr > class seque_iterator;

constexpr int block_size_shift = 10; // block size = 2^block_size_shift

template< typename _value_type, typename _ref, typename _ptr >
class seque_iterator
{
  friend class Seque< _value_type > ;

  // Making all templated iterators friends to allow converting
  // iterator <--> const_iterator.
  template< typename, typename, typename > friend class seque_iterator;

private:
  template< typename _cv_value_type >
  using _iter = seque_iterator< _value_type, _cv_value_type&, _cv_value_type* >;

  const Seque< _value_type >* seque_;
  size_t block_index_;
  size_t element_index_;

public:
  using iterator = _iter<_value_type>;
  using const_iterator = _iter<const _value_type>;

  using iterator_category = std::random_access_iterator_tag;
  using value_type = _value_type;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = size_t;

  explicit seque_iterator( const Seque< _value_type >& seque, size_t block_num,
      size_t block_pos );

  seque_iterator( const iterator& );
  seque_iterator( const const_iterator& );

  seque_iterator& operator=( const seque_iterator& );
  seque_iterator& operator++();
  seque_iterator& operator--();
  seque_iterator& operator+=( size_t );
  seque_iterator operator+( size_t );
  reference operator*() const;
  pointer operator->() const;

  difference_type operator-( const seque_iterator& ) const;

  bool operator==( const seque_iterator& ) const;
  bool operator!=( const seque_iterator& ) const;
  bool operator<( const seque_iterator& ) const;
};

template< typename _value_type >
class Seque
{
  template< typename _cv_value_type, typename _ref, typename _ptr >
  friend class seque_iterator;
public:

  using iterator = seque_iterator<_value_type, _value_type&, _value_type*>;
  using const_iterator = seque_iterator<_value_type, const _value_type&, const _value_type*>;

  Seque();
  explicit Seque( size_t );
  virtual ~Seque();

  void push_back( const _value_type& value );

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  _value_type& operator[]( const size_t pos );
  const _value_type& operator[]( const size_t pos ) const;

  void clear();
  size_t size() const;
  iterator erase( const_iterator, const_iterator );
  void print_blocks() const;

private:
  std::vector< std::vector< _value_type > > blockmap_;
  const size_t max_block_size_;
  const size_t max_block_size_m_1_;
  iterator finish_;
};

/////////////////////////////////////////////////////////////
//
// Seque method implementation
//
/////////////////////////////////////////////////////////////

template < typename _value_type >
inline typename Seque< _value_type >::iterator Seque< _value_type >::begin()
{
  return iterator( *this, 0, 0 );
}

template< typename _value_type >
inline typename Seque< _value_type >::const_iterator
Seque< _value_type >::begin() const
{
  return const_iterator( *this, 0, 0 );
}

template< typename _value_type >
inline typename Seque< _value_type >::iterator Seque< _value_type >::end()
{
  return finish_;
}

template< typename _value_type >
inline typename Seque< _value_type >::const_iterator Seque< _value_type >::end() const
{
  return finish_;
}

template < typename _value_type >
inline const _value_type& Seque< _value_type >::operator[](
    const size_t pos ) const
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_pos = pos >> block_size_shift;
  const auto pos_pos = pos & max_block_size_m_1_;
  return blockmap_[ block_pos ][ pos_pos ];
}

template < typename _value_type >
inline _value_type& Seque< _value_type >::operator[]( const size_t pos )
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_m_1_;
  return blockmap_[ block_index ][ element_index ];
}

template < typename _value_type >
inline Seque< _value_type >::Seque()
  : max_block_size_( 1L << block_size_shift )
  , max_block_size_m_1_( max_block_size_ - 1 )
  , finish_( begin() )
{
  blockmap_.push_back( std::vector< _value_type >( max_block_size_ ) );
}

template < typename _value_type >
inline Seque< _value_type >::Seque( size_t n )
  : max_block_size_( 1L << block_size_shift )
  , max_block_size_m_1_( max_block_size_ - 1 )
  , finish_( begin() )
{
  size_t num_blocks_needed = std::ceil( ( float ) n / max_block_size_ );
  for ( size_t i = 0; i < num_blocks_needed; ++i )
  {
    blockmap_.push_back( std::vector< _value_type >( max_block_size_ ) );
  }
  finish_ += n;
}

template < typename _value_type >
inline Seque< _value_type >::~Seque() = default;

template < typename _value_type >
inline void
Seque< _value_type >::push_back( const _value_type& value )
{
  if ( finish_.block_index_ == blockmap_.size() )
  {
    blockmap_.push_back( std::vector< _value_type >( max_block_size_ ) );
  }
  blockmap_[ finish_.block_index_ ][ finish_.element_index_ ] = value;
  ++finish_;
}

template < typename _value_type >
inline void
Seque< _value_type >::clear()
{
  auto b = begin();
  for ( auto block : blockmap_ )
  {
    block.clear();
  }
  blockmap_.shrink_to_fit();
  finish_ = b;
}

template < typename _value_type >
inline size_t
Seque< _value_type >::size() const
{
  return finish_.block_index_ * max_block_size_ + finish_.element_index_;
}

template < typename _value_type >
inline typename Seque< _value_type >::iterator
Seque< _value_type >::erase( const_iterator first, const_iterator last )
{
  // TODO: Except for some special cases, this method may kill performance as every
  // element after last has to be moved to fill the erased space.
  if ( first == last )
  {
    return iterator( first );
  }
  else if ( first == begin() && last == end() )
  {
    clear();
    return end();
  }
  //  else if ( first.element_index_ == 0 && last.element_index_ == 0 )
  //  {
  //  }
  else
  {
    // TODO: This needs to be cleaned up / made clearer.
    auto repl_it = first; // Replacement iterator
    // Saving iterator to be returned, which is where the first element after
    // the last deleted element will be after filling the erased elements.
    const const_iterator returnable_iterator = first;
    for ( auto element = last; element != end(); ++element )
    {
      *repl_it = *element; // TODO: use std::move?
      ++repl_it;
    }
    // Erase everything after the new end.
    auto& new_finish_block = blockmap_[ repl_it.block_index_ ];
    new_finish_block.erase( new_finish_block.begin() + repl_it.element_index_,
      new_finish_block.end() );
    blockmap_.erase(
      blockmap_.begin() + repl_it.block_index_ + 1, blockmap_.end() );
    blockmap_.shrink_to_fit();
    finish_ = repl_it;
    return returnable_iterator;
  }
}

template < typename _value_type >
inline void
Seque< _value_type >::print_blocks() const
{
  std::cerr << "==============================================\n";
  for ( size_t block_index = 0; block_index != blockmap_.size(); ++block_index )
  {
    std::cerr << "----------------------------------------------\n";
    for ( size_t element_index = 0;
          element_index != blockmap_[ block_index ].size();
          ++element_index )
    {
      if ( block_index == finish_.block_index_
        and element_index == finish_.element_index_ )
      {
        break;
      }
      std::cerr << blockmap_[ block_index ][ element_index ] << " ";
    }
    std::cerr << "\n----------------------------------------------\n";
  }
  std::cerr << "==============================================\n";
}

/////////////////////////////////////////////////////////////
//
// Seque iterator method implementation
//
/////////////////////////////////////////////////////////////

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
  const Seque< _value_type >& seque,
  size_t block_num, size_t block_pos )
  : seque_( &seque )
  , block_index_( block_num )
  , element_index_( block_pos )
{
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
    const iterator& other )
    : seque_( other.seque_ ), block_index_( other.block_index_ ), element_index_(
        other.element_index_ )
{
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
    const const_iterator& other )
    : seque_( other.seque_ ), block_index_( other.block_index_ ), element_index_(
        other.element_index_ )
{
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::operator=(
    const seque_iterator< _value_type, _ref, _ptr >& other )
{
  seque_ = other.seque_;
  block_index_ = other.block_index_;
  element_index_ = other.element_index_;
  return *this;
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::
operator++()
{
  ++element_index_;
  if ( element_index_ == seque_->max_block_size_ )
  {
    ++block_index_;
    element_index_ = 0;
  }
  return *this;
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::
operator--()
{
  // Because element_index is unsigned, we need to check if it will go out of
  // bounds first.
  if ( element_index_ == 0 )
  {
    --block_index_;
    element_index_ = seque_->max_block_size_m_1_;
  }
  else
  {
    --element_index_;
  }
  return *this;
}

template< typename _value_type, typename _ref, typename _ptr >
inline typename seque_iterator< _value_type, _ref, _ptr >::reference
seque_iterator<
    _value_type, _ref, _ptr >::
  operator*() const
{
  // TODO: Using const_cast  to remove the constness isn't the most elegant
  // solution.
  // There is probably a better way to do this.
  return const_cast< reference >(
    seque_->blockmap_[ block_index_ ][ element_index_ ] );
}

template< typename _value_type, typename _ref, typename _ptr >
inline typename seque_iterator< _value_type, _ref, _ptr >::pointer
seque_iterator<
    _value_type, _ref, _ptr >::
  operator->() const
{
  // TODO: Again, using const_cast  to remove the constness isn't the most
  // elegant solution.
  // There is probably a better way to do this.
  return const_cast< pointer >(
    &( seque_->blockmap_[ block_index_ ][ element_index_ ] ) );
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::
operator+=(
    size_t val )
{
  for ( size_t i = 0; i < val; ++i )
  {
    operator++();
  }
  return *this;
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr > seque_iterator<
    _value_type, _ref, _ptr >::
operator+(
    size_t val )
{
  seque_iterator tmp = *this;
  return tmp += val;
}


template< typename _value_type, typename _ref, typename _ptr >
inline typename seque_iterator< _value_type, _ref, _ptr >::difference_type
seque_iterator<
    _value_type, _ref, _ptr >::operator-(
    const seque_iterator< _value_type, _ref, _ptr >& other ) const
{
  // TODO: Might not return what it should.. (but see source_table.h::L413)
  return ( this->block_index_ - other.block_index_ ) * seque_->max_block_size_
    + ( this->element_index_ - other.element_index_ );
}

template< typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator!=(
    const seque_iterator< _value_type, _ref, _ptr >& rhs ) const
{
  return (
    element_index_ != rhs.element_index_ or block_index_ != rhs.block_index_ );
}

template< typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator==(
    const seque_iterator< _value_type, _ref, _ptr >& rhs ) const
{
  return (
    element_index_ == rhs.element_index_ and block_index_ == rhs.block_index_ );
}

template< typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator <(
    const seque_iterator& rhs ) const
{
  return ( block_index_ < rhs.block_index_
      and element_index_ < rhs.element_index_ );
}

#endif /* SEQUE_H_ */
