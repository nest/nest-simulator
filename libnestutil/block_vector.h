/*
 *  block_vector.h
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

#ifndef BLOCK_VECTOR_H_
#define BLOCK_VECTOR_H_

#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <vector>

#include "sliexceptions.h"

template < typename value_type_ >
class BlockVector;
template < typename value_type_, typename ref_, typename ptr_ >
class bv_iterator;

constexpr int block_size_shift = 10; //!< max_block_size = 2^block_size_shift
constexpr int max_block_size = 1L << block_size_shift;
constexpr int max_block_size_sub_1 = max_block_size - 1;

/**
 * @brief A BlockVector::iterator.
 * @tparam value_type_ Type of element.
 * @tparam ref_ Type of reference to the element.
 * @tparam ptr_ Type of pointer to the element.
 *
 * BlockVector holds one of this internally, marking the end of the valid range.
 */
template < typename value_type_, typename ref_, typename ptr_ >
class bv_iterator
{
  friend class BlockVector< value_type_ >;

  // Making all templated iterators friends to allow converting
  // iterator to const_iterator.
  template < typename, typename, typename >
  friend class bv_iterator;

private:
  template < typename cv_value_type_ >
  using iter_ = bv_iterator< value_type_, cv_value_type_&, cv_value_type_* >;

  //! BlockVector to which this iterator points
  const BlockVector< value_type_ >* block_vector_;
  //! Iterator for the current block in the blockmap
  typename std::vector< std::vector< value_type_ > >::const_iterator block_vector_it_;
  //! Iterator pointing to the current element in the current block.
  typename std::vector< value_type_ >::const_iterator block_it_;
  //! Iterator pointing to the end of the current block.
  typename std::vector< value_type_ >::const_iterator current_block_end_;

public:
  using iterator = iter_< value_type_ >;
  using const_iterator = iter_< const value_type_ >;

  using iterator_category = std::random_access_iterator_tag;
  using value_type = value_type_;
  using pointer = ptr_;
  using reference = ref_;
  using difference_type = typename BlockVector< value_type >::difference_type;

  bv_iterator()
    : block_vector_( nullptr )
  {
  }

  /**
   * @brief Creates an iterator pointing to the first element in a BlockVector.
   * @param block_vector BlockVector to which the iterator will point to.
   */
  explicit bv_iterator( const BlockVector< value_type_ >& );

  /**
   * @brief Iterator copy constructor.
   * @param other Iterator to be copied.
   */
  bv_iterator( const iterator& );

  /**
   * @brief Creates an iterator with specified parameters.
   * @param block_vector BlockVector to point to.
   * @param block_index Index of current block.
   * @param block_it Iterator pointing to current element in the current block.
   * @param current_block_end Iterator pointing to the end of the current block.
   */
  bv_iterator( const BlockVector< value_type_ >*,
    const typename std::vector< std::vector< value_type_ > >::const_iterator,
    const typename std::vector< value_type_ >::const_iterator,
    const typename std::vector< value_type_ >::const_iterator );

  bv_iterator& operator++();
  bv_iterator& operator--();
  bv_iterator& operator+=( difference_type );
  bv_iterator& operator-=( difference_type );
  bv_iterator operator+( difference_type ) const;
  bv_iterator operator-( difference_type ) const;
  bv_iterator operator++( int );
  bv_iterator operator--( int );
  reference operator*() const;
  pointer operator->() const;
  difference_type operator-( const iterator& ) const;
  difference_type operator-( const const_iterator& ) const;

  iterator& operator=( const iterator& );

  reference operator[]( difference_type n ) const;

  bool operator==( const bv_iterator& ) const;
  bool operator!=( const bv_iterator& ) const;
  bool operator<( const bv_iterator& ) const;
  bool operator>( const bv_iterator& ) const;
  bool operator<=( const bv_iterator& ) const;
  bool operator>=( const bv_iterator& ) const;

private:
  /**
   * @brief Converts the iterator to a non-const iterator.
   */
  iterator const_cast_() const;
};

/**
 * @brief Container with a vector-of-vectors structure.
 * @tparam value_type_ Type of element.
 *
 * Elements are stored in blocks held in a blockmap. Each block is of fixed
 * size, with elements default-initialised on creation of the block. A new block
 * is automatically created when a block is filled. The size of each block is a
 * power of two, which allows use of bitwise operators to efficiently map an
 * index to the right block and the right position in that block.
 */
template < typename value_type_ >
class BlockVector
{
  template < typename cv_value_type_, typename ref_, typename ptr_ >
  friend class bv_iterator;

public:
  using value_type = value_type_;
  using difference_type = typename std::vector< value_type >::difference_type;
  using const_reference = const value_type&;
  using const_pointer = const value_type*;
  using iterator = bv_iterator< value_type_, value_type_&, value_type_* >;
  using const_iterator = bv_iterator< value_type_, const value_type_&, const value_type_* >;
  using reverse_iterator = std::reverse_iterator< iterator >;
  using const_reverse_iterator = std::reverse_iterator< const_iterator >;
  using size_type = size_t;

  /**
   * @brief Creates an empty BlockVector.
   */
  BlockVector();

  /**
   * @brief Creates a BlockVector containing a number of elements.
   * @param n Number of elements.
   */
  explicit BlockVector( size_t );

  /**
   * @brief BlockVector copy constructor.
   * @param other BlockVector to copy.
   */
  BlockVector( const BlockVector< value_type_ >& );
  virtual ~BlockVector();

  /**
   * @brief Subscript access to the data contained in the BlockVector.
   * @param pos The index of the element for which data should be accessed.
   * @return  Read/write reference to data.
   *
   * Note that data access with this operator is unchecked.
   */
  value_type_& operator[]( const size_t pos );

  /**
   * @brief Subscript access to the data contained in the BlockVector.
   * @param pos The index of the element for which data should be accessed.
   * @return  Read-only (constant) reference to data.
   *
   * Note that data access with this operator is unchecked.
   */
  const value_type_& operator[]( const size_t pos ) const;

  /**
   * Returns a read/write iterator that points to the first element in the
   * BlockVector. Iteration is done in ordinary element order.
   */
  iterator begin();

  /**
   * Returns a read-only (constant) iterator that points to the first element
   * in the BlockVector. Iteration is done in ordinary element order.
   */
  const_iterator begin() const;

  /**
   * Returns a read/write iterator that points one past the last element in the
   * BlockVector. Iteration is done in ordinary element order.
   */
  iterator end();

  /**
   * Returns a read-only (constant) iterator that points one past the last
   * element in the BlockVector. Iteration is done in ordinary element order.
   */
  const_iterator end() const;

  /**
   * @brief Add data to the end of the BlockVector.
   * @param value Data to be added.
   *
   * Assigns given data to the element at the end of the BlockVector.
   */
  void push_back( const value_type_& value );

  /**
   * Erases all the elements.
   */
  void clear();

  /**
   * Returns the number of elements in the BlockVector.
   */
  size_t size() const;

  /**
   * @brief Remove a range of elements.
   * @param first Iterator pointing to the first element to be erased.
   * @param last Iterator pointing one past the last element to be erased.
   * @return An iterator pointing to the element pointed to by @a last prior
   *         to erasing (or end()).
   *
   * This function will erase the elements in the range [first, last)
   * and shorten the BlockVector accordingly.
   */
  iterator erase( const_iterator, const_iterator );

  /**
   * @brief Writes the contents of the BlockVector, separated into blocks, to
   * cerr.
   */
  void print_blocks() const;

  /**
   * @brief Returns the block-size.
   */
  int get_max_block_size() const;

  /**
   * @brief Returns the size() of the largest possible BlockVector.
   */
  size_type max_size() const;

  /**
   * @brief Returns a read/write reverse iterator that points to the last
   * element in the BlockVector. Iteration is done in reverse element
   * order.
   */
  reverse_iterator rbegin();

  /**
   * @brief Returns a read-only (constant) reverse iterator that points to
   * the last element in the BlockVector. Iteration is done in reverse
   * element order.
   */
  reverse_iterator rbegin() const;

  /**
   * @brief Returns a read/write reverse iterator that points to one
   * before the first element in the BlockVector. Iteration is done in
   * reverse element order.
   */
  reverse_iterator rend();

  /**
   * @brief Returns a read-only (constant) reverse iterator that points to
   * one before the first element in the BlockVector. Iteration is done in
   * reverse element order.
   */
  reverse_iterator rend() const;

private:
  //! Vector holding blocks containing data.
  std::vector< std::vector< value_type_ > > blockmap_;
  iterator finish_; //!< Iterator pointing to one past the last element.
};

/////////////////////////////////////////////////////////////
//               BlockVector method implementation         //
/////////////////////////////////////////////////////////////

template < typename value_type_ >
inline BlockVector< value_type_ >::BlockVector()
  : blockmap_( std::vector< std::vector< value_type_ > >( 1, std::vector< value_type_ >( max_block_size ) ) )
  , finish_( begin() )
{
}

template < typename value_type_ >
inline BlockVector< value_type_ >::BlockVector( size_t n )
  : blockmap_( std::vector< std::vector< value_type_ > >( 1, std::vector< value_type_ >( max_block_size ) ) )
  , finish_( begin() )
{
  size_t num_blocks_needed = std::ceil( static_cast< double >( n ) / max_block_size );
  for ( size_t i = 0; i < num_blocks_needed - 1; ++i )
  {
    blockmap_.emplace_back( max_block_size );
  }
  finish_ = begin(); // Because the blockmap has changed we need to recreate the iterator
  finish_ += n;
}

template < typename value_type_ >
inline BlockVector< value_type_ >::BlockVector( const BlockVector< value_type_ >& other )
  : blockmap_( other.blockmap_ )
  , finish_( begin() + ( other.finish_ - other.begin() ) )
{
}

template < typename value_type_ >
inline BlockVector< value_type_ >::~BlockVector() = default;

template < typename value_type_ >
inline value_type_&
BlockVector< value_type_ >::operator[]( const size_t pos )
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_sub_1;
  return blockmap_[ block_index ][ element_index ];
}

template < typename value_type_ >
inline const value_type_&
BlockVector< value_type_ >::operator[]( const size_t pos ) const
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_sub_1;
  return blockmap_[ block_index ][ element_index ];
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::iterator
BlockVector< value_type_ >::begin()
{
  return iterator( *this );
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::const_iterator
BlockVector< value_type_ >::begin() const
{
  return const_iterator( *this );
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::iterator
BlockVector< value_type_ >::end()
{
  return finish_;
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::const_iterator
BlockVector< value_type_ >::end() const
{
  return finish_;
}

template < typename value_type_ >
inline void
BlockVector< value_type_ >::push_back( const value_type_& value )
{
  // If this is the last element in the current block, add another block
  if ( finish_.block_it_ == finish_.current_block_end_ - 1 )
  {
    // Need to get the current position here, then recreate the iterator after we extend the blockmap,
    // because after the blockmap is changed the iterator becomes invalid.
    const auto current_block = finish_.block_vector_it_ - finish_.block_vector_->blockmap_.begin();
    blockmap_.emplace_back( max_block_size );
    finish_.block_vector_it_ = finish_.block_vector_->blockmap_.begin() + current_block;
  }
  *finish_ = value;
  ++finish_;
}

template < typename value_type_ >
inline void
BlockVector< value_type_ >::clear()
{
  for ( auto it = blockmap_.begin(); it != blockmap_.end(); ++it )
  {
    it->clear();
  }
  blockmap_.clear();
  // Initialise the first block
  blockmap_.emplace_back( max_block_size );
  finish_ = begin();
}

template < typename value_type_ >
inline size_t
BlockVector< value_type_ >::size() const
{
  size_t element_index; // Where we are in the current block
  if ( finish_.block_vector_it_ >= blockmap_.end() )
  {
    // If the current block is completely filled
    element_index = 0;
  }
  else
  {
    element_index = finish_.block_it_ - finish_.block_vector_it_->begin();
  }
  return ( finish_.block_vector_it_ - finish_.block_vector_->blockmap_.begin() ) * max_block_size + element_index;
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::iterator
BlockVector< value_type_ >::erase( const_iterator first, const_iterator last )
{
  assert( first.block_vector_ == this );
  assert( last.block_vector_ == this );
  assert( last < finish_ or last == finish_ );
  if ( first == last )
  {
    return iterator( first.const_cast_() );
  }
  else if ( first == begin() and last == end() )
  {
    clear();
    return end();
  }
  else
  {
    auto repl_it = first.const_cast_(); // Iterator for elements to be replaced.
    for ( auto element = last; element != end(); ++element )
    {
      *repl_it = std::move( *element );
      ++repl_it;
    }
    // The block that repl_it ends up in is the new final block. Using iterator arithmetics to avoid issues with the
    // const iterator.
    auto& new_final_block = blockmap_[ repl_it.block_vector_it_ - blockmap_.begin() ];
    // Here, the arithmetic says that we first subtract
    // new_final_block.begin(), then add it again (which seems unnecessary).
    // But what we really do is extracting the element index of repl_it, then
    // fast-forwarding new_final_block.begin() to that index.
    auto element_index = repl_it.block_it_ - new_final_block.begin();
    // Erase everything after the replaced elements in the current block.
    new_final_block.erase( new_final_block.begin() + element_index, new_final_block.end() );
    // Refill the erased elements in the final block with default-initialised
    // elements.
    int num_default_init = max_block_size - new_final_block.size();
    for ( int i = 0; i < num_default_init; ++i )
    {
      new_final_block.emplace_back();
    }
    assert( new_final_block.size() == max_block_size );
    // Erase all subsequent blocks.
    blockmap_.erase( repl_it.block_vector_it_ + 1, blockmap_.end() );
    // Construct new finish_ iterator
    finish_ = repl_it;
    // The iterator which is to be returned is located where the first element
    // after the last deleted element will be after filling the erased elements,
    // which is the position of the first deleted element.
    return first.const_cast_();
  }
}

template < typename value_type_ >
inline void
BlockVector< value_type_ >::print_blocks() const
{
  std::cerr << "this: \t\t" << this << "\n";
  std::cerr << "finish block_vector: \t" << finish_.block_vector_ << "\n";
  std::cerr << "Blockmap size: " << blockmap_.size() << "\n";
  std::cerr << "==============================================\n";
  auto seq_iter = begin();
  for ( size_t block_index = 0; block_index != blockmap_.size() and seq_iter != end(); ++block_index )
  {
    std::cerr << "----------------------------------------------\n";
    auto& block = blockmap_[ block_index ];
    std::cerr << "Block size: " << block.size() << "\n";
    for ( auto block_it = block.begin(); block_it != block.end() and seq_iter != end(); ++block_it )
    {
      std::cerr << *block_it << " ";
      ++seq_iter;
    }
    std::cerr << "\n----------------------------------------------\n";
  }
  std::cerr << "==============================================\n";
}

template < typename value_type_ >
inline int
BlockVector< value_type_ >::get_max_block_size() const
{
  return max_block_size;
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::size_type
BlockVector< value_type_ >::max_size() const
{
  throw NotImplemented( "BlockVector max_size() is not implemented." );
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::reverse_iterator
BlockVector< value_type_ >::rbegin()
{
  throw NotImplemented( "BlockVector rbegin() is not implemented." );
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::reverse_iterator
BlockVector< value_type_ >::rbegin() const
{
  throw NotImplemented( "BlockVector rbegin() is not implemented." );
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::reverse_iterator
BlockVector< value_type_ >::rend()
{
  throw NotImplemented( "BlockVector rend() is not implemented." );
}

template < typename value_type_ >
inline typename BlockVector< value_type_ >::reverse_iterator
BlockVector< value_type_ >::rend() const
{
  throw NotImplemented( "BlockVector rend() is not implemented." );
}

/////////////////////////////////////////////////////////////
//        BlockVector iterator method implementation       //
/////////////////////////////////////////////////////////////

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >::bv_iterator( const BlockVector< value_type_ >& block_vector )
  : block_vector_( &block_vector )
  , block_vector_it_( block_vector_->blockmap_.begin() )
  , block_it_( block_vector_it_->begin() )
  , current_block_end_( block_vector_it_->end() )
{
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >::bv_iterator( const iterator& other )
  : block_vector_( other.block_vector_ )
  , block_vector_it_( other.block_vector_it_ )
  , block_it_( other.block_it_ )
  , current_block_end_( other.current_block_end_ )
{
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >::bv_iterator( const BlockVector< value_type_ >* block_vector,
  const typename std::vector< std::vector< value_type_ > >::const_iterator block_vector_it,
  const typename std::vector< value_type_ >::const_iterator block_it,
  const typename std::vector< value_type_ >::const_iterator current_block_end )
  : block_vector_( block_vector )
  , block_vector_it_( block_vector_it )
  , block_it_( block_it )
  , current_block_end_( current_block_end )
{
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >&
bv_iterator< value_type_, ref_, ptr_ >::operator++()
{
  ++block_it_;
  if ( block_it_ == current_block_end_ )
  {
    ++block_vector_it_;
    // If we are at end() now, we are outside of the blockmap, which is
    // undefined. The outer iterator can be in an undefined position that
    // will compare as safely larger or equal to end(), but we don't set
    // the inner iterators.
    if ( block_vector_it_ != block_vector_->blockmap_.end() )
    {
      block_it_ = block_vector_it_->begin();
      current_block_end_ = block_vector_it_->end();
    }
  }
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >&
bv_iterator< value_type_, ref_, ptr_ >::operator--()
{
  // If we are still within the block, we can just decrement the block iterator.
  // If not, we need to switch to the previous block.
  if ( block_it_ != block_vector_it_->begin() )
  {
    --block_it_;
  }
  else if ( block_vector_it_ != block_vector_->blockmap_.begin() )
  {
    --block_vector_it_;
    current_block_end_ = block_vector_it_->end();
    block_it_ = current_block_end_ - 1;
  }
  else
  {
    // We are before begin(), which is undefined. We mark this by moving the
    // outer iterator to an undefined position that will compare as safely smaller
    // than begin(). According to C++17 Standard, §27.2.6, decrementing an
    // iterator that is equal to begin() yields undefined behavior.
    --block_vector_it_;
  }
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >&
bv_iterator< value_type_, ref_, ptr_ >::operator+=( difference_type val )
{
  if ( val < 0 )
  {
    return operator-=( -val );
  }
  for ( difference_type i = 0; i < val; ++i )
  {
    operator++();
  }
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >&
bv_iterator< value_type_, ref_, ptr_ >::operator-=( difference_type val )
{
  if ( val < 0 )
  {
    return operator+=( -val );
  }
  for ( difference_type i = 0; i < val; ++i )
  {
    operator--();
  }
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >
bv_iterator< value_type_, ref_, ptr_ >::operator+( difference_type val ) const
{
  bv_iterator tmp = *this;
  return tmp += val;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >
bv_iterator< value_type_, ref_, ptr_ >::operator-( difference_type val ) const
{
  bv_iterator tmp = *this;
  return tmp -= val;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >
bv_iterator< value_type_, ref_, ptr_ >::operator++( int )
{
  bv_iterator< value_type_, ref_, ptr_ > old( *this );
  ++( *this );
  return old;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >
bv_iterator< value_type_, ref_, ptr_ >::operator--( int )
{
  bv_iterator< value_type_, ref_, ptr_ > old( *this );
  --( *this );
  return old;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::reference
bv_iterator< value_type_, ref_, ptr_ >::operator*() const
{
  // TODO: Using const_cast  to remove the constness isn't the most elegant
  // solution. There is probably a better way to do this.
  return const_cast< reference >( *block_it_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::pointer
bv_iterator< value_type_, ref_, ptr_ >::operator->() const
{
  // TODO: Again, using const_cast  to remove the constness isn't the most
  // elegant solution. There is probably a better way to do this.
  return const_cast< pointer >( &( *block_it_ ) );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::difference_type
bv_iterator< value_type_, ref_, ptr_ >::operator-( const iterator& other ) const
{
  const auto this_element_index = block_it_ - block_vector_it_->begin();
  const auto other_element_index = other.block_it_ - other.block_vector_it_->begin();
  return ( block_vector_it_ - other.block_vector_it_ ) * max_block_size + ( this_element_index - other_element_index );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::difference_type
bv_iterator< value_type_, ref_, ptr_ >::operator-( const const_iterator& other ) const
{
  const auto this_element_index = block_it_ - block_vector_it_->begin();
  const auto other_element_index = other.block_it_ - other.block_vector_it_->begin();
  return ( block_vector_it_ - other.block_vector_it_ ) * max_block_size + ( this_element_index - other_element_index );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::iterator&
bv_iterator< value_type_, ref_, ptr_ >::operator=( const iterator& other )
{
  block_vector_ = other.block_vector_;
  block_vector_it_ = other.block_vector_it_;
  block_it_ = other.block_it_;
  current_block_end_ = other.current_block_end_;
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::reference
bv_iterator< value_type_, ref_, ptr_ >::operator[]( difference_type n ) const
{
  return *( *this + n );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool
bv_iterator< value_type_, ref_, ptr_ >::operator==( const bv_iterator< value_type_, ref_, ptr_ >& rhs ) const
{
  return ( block_vector_it_ == rhs.block_vector_it_ and block_it_ == rhs.block_it_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool
bv_iterator< value_type_, ref_, ptr_ >::operator!=( const bv_iterator< value_type_, ref_, ptr_ >& rhs ) const
{
  return ( block_vector_it_ != rhs.block_vector_it_ or block_it_ != rhs.block_it_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool
bv_iterator< value_type_, ref_, ptr_ >::operator<( const bv_iterator& rhs ) const
{
  return ( block_vector_it_ < rhs.block_vector_it_
    or ( block_vector_it_ == rhs.block_vector_it_ and block_it_ < rhs.block_it_ ) );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool
bv_iterator< value_type_, ref_, ptr_ >::operator>( const bv_iterator& rhs ) const
{
  return ( block_vector_it_ > rhs.block_vector_it_
    or ( block_vector_it_ == rhs.block_vector_it_ and block_it_ > rhs.block_it_ ) );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool
bv_iterator< value_type_, ref_, ptr_ >::operator<=( const bv_iterator& rhs ) const
{
  return operator<( rhs ) or operator==( rhs );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool
bv_iterator< value_type_, ref_, ptr_ >::operator>=( const bv_iterator& rhs ) const
{
  return operator>( rhs ) or operator==( rhs );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename bv_iterator< value_type_, ref_, ptr_ >::iterator
bv_iterator< value_type_, ref_, ptr_ >::const_cast_() const
{
  return iterator( block_vector_, block_vector_it_, block_it_, current_block_end_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bv_iterator< value_type_, ref_, ptr_ >
operator+( typename bv_iterator< value_type_, ref_, ptr_ >::difference_type n,
  bv_iterator< value_type_, ref_, ptr_ >& x )
{
  return x + n;
}

#endif /* BLOCK_VECTOR_H_ */
