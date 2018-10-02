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
  // iterator -> const_iterator.
  template< typename, typename, typename > friend class seque_iterator;

private:
  template< typename _cv_value_type >
  using _iter = seque_iterator< _value_type, _cv_value_type&, _cv_value_type* >;

  const Seque< _value_type >* seque_;
  size_t block_index_;
  // TODO: auto?
  typename std::vector< _value_type >::const_iterator block_it_;
  typename std::vector< _value_type >::const_iterator current_block_end_;

public:
  using iterator = _iter<_value_type>;
  using const_iterator = _iter<const _value_type>;

  using iterator_category = std::random_access_iterator_tag; // TODO: needed?
  using value_type = _value_type;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = size_t;

  explicit seque_iterator( const Seque< _value_type >& );

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
  Seque( const Seque< _value_type >& );
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
  const size_t max_block_size_; // TODO: Can these be constexpr?
  const size_t max_block_size_m_1_;
  std::vector< std::vector< _value_type > > blockmap_;
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
  return iterator( *this );
}

template< typename _value_type >
inline typename Seque< _value_type >::const_iterator
Seque< _value_type >::begin() const
{
  return const_iterator( *this );
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
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_m_1_;
  return blockmap_[ block_index ][ element_index ];
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
, blockmap_(
        std::vector< std::vector< _value_type > >( 1,
            std::vector< _value_type >( max_block_size_ ) ) )
  , finish_( begin() )
{
}

template < typename _value_type >
inline Seque< _value_type >::Seque( size_t n )
  : max_block_size_( 1L << block_size_shift )
  , max_block_size_m_1_( max_block_size_ - 1 )
  , blockmap_(
        std::vector< std::vector< _value_type > >( 1,
            std::vector< _value_type >( max_block_size_ ) ) )
  , finish_( begin() )
{
  size_t num_blocks_needed = std::ceil( ( float ) n / max_block_size_ );
  for ( size_t i = 0; i < num_blocks_needed - 1; ++i )
  {
    blockmap_.push_back( std::vector< _value_type >( max_block_size_ ) );
  }
  finish_ += n;
}

template< typename _value_type >
inline Seque< _value_type >::Seque( const Seque< _value_type >& other )
    : max_block_size_( other.max_block_size_ ), max_block_size_m_1_(
        other.max_block_size_m_1_ ), blockmap_( other.blockmap_ )
        , finish_( begin() + ( other.finish_ - other.begin() ) )
{
}

template < typename _value_type >
inline Seque< _value_type >::~Seque() = default;

template < typename _value_type >
inline void
Seque< _value_type >::push_back( const _value_type& value )
{
  // If this is the last element in the current block, add another block
  if ( finish_.block_it_ == finish_.current_block_end_ - 1 )
  {
    blockmap_.emplace_back( max_block_size_ );
  }
  *finish_ = value;
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
  size_t element_index;
  if ( finish_.block_index_ >= blockmap_.size() )
  {
    element_index = 0;
  }
  else
  {
    element_index = finish_.block_it_
        - blockmap_[ finish_.block_index_ ].begin();
  }
  return finish_.block_index_ * max_block_size_ + element_index;
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
    // Here, the arithmetic says that we first subtract
    // new_finish_block.begin(), then add it again (which seems unnecessary).
    // But what we really do is extracting the element index of repl_it, then
    // fast-forwarding new_finish_block.begin() to that index.
    auto element_index = repl_it.block_it_ - new_finish_block.begin();
    new_finish_block.erase( new_finish_block.begin() + element_index,
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
//    for ( size_t element_index = 0;
//          element_index != blockmap_[ block_index ].size();
//          ++element_index )
    auto& blockmap = blockmap_[ block_index ];
    for ( auto block_it = blockmap.begin(); block_it != blockmap.end();
        ++block_it )
    {
      if ( block_it != finish_.block_it_ )
      {
        std::cerr << *block_it << " ";
      }
      else
      {
        break;
      }
      
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
    const Seque< _value_type >& seque )
  : seque_( &seque )
  , block_index_( 0 )
  , block_it_(
        seque_->blockmap_[ block_index_ ].begin() )
  , current_block_end_(
        seque_->blockmap_[ block_index_ ].end() )
{
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
    const iterator& other )
    : seque_( other.seque_ ), block_index_( other.block_index_ ), block_it_(
        other.block_it_ ), current_block_end_( other.current_block_end_ )
{
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
    const const_iterator& other )
    : seque_( other.seque_ ), block_index_( other.block_index_ ), block_it_(
        other.block_it_ ), current_block_end_( other.current_block_end_ )
{
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::operator=(
    const seque_iterator< _value_type, _ref, _ptr >& other )
{
  seque_ = other.seque_;
  block_index_ = other.block_index_;
  block_it_ = other.block_it_;
  current_block_end_ = other.current_block_end_;
  return *this;
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::
operator++()
{
  ++block_it_;
  if ( block_it_ == current_block_end_ )
  {
    ++block_index_;
    block_it_ = seque_->blockmap_[ block_index_ ].begin();
    current_block_end_ = seque_->blockmap_[ block_index_ ].end();
  }
  return *this;
}

template< typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >& seque_iterator<
    _value_type, _ref, _ptr >::
operator--()
{
  // Because element_index is unsigned, we need to check if it will go out of
  // bounds first. TODO: fix this comment
  if ( block_it_ != seque_->blockmap_[ block_index_ ].begin() )
  {
    --block_it_;
  }
  else
  {
    --block_index_;
    current_block_end_ = seque_->blockmap_[ block_index_ ].end();
    block_it_ = current_block_end_ - 1;
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
//  return const_cast< reference >(
//    seque_->blockmap_[ block_index_ ][ element_index_ ] );
  return const_cast< reference >( *block_it_ );
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
//  return const_cast< pointer >(
//    &( seque_->blockmap_[ block_index_ ][ element_index_ ] ) );
  return const_cast< pointer >( &( *block_it_ ) );
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
  auto this_element_index = block_it_
      - seque_->blockmap_[ block_index_ ].begin();
  auto other_element_index = other.block_it_
      - other.seque_->blockmap_[ other.block_index_ ].begin();
  return ( block_index_ - other.block_index_ ) * seque_->max_block_size_
      + ( this_element_index - other_element_index );
}

template< typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator!=(
    const seque_iterator< _value_type, _ref, _ptr >& rhs ) const
{
  return (
    block_index_ != rhs.block_index_ or block_it_ != rhs.block_it_ );
}

template< typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator==(
    const seque_iterator< _value_type, _ref, _ptr >& rhs ) const
{
  return (
    block_index_ == rhs.block_index_ and block_it_ == rhs.block_it_ );
}

template< typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator <(
    const seque_iterator& rhs ) const
{
  return ( block_index_ < rhs.block_index_
      and block_it_ < rhs.block_it_ );
}

#endif /* SEQUE_H_ */
