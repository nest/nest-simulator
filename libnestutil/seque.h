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

template < typename value_type_ >
class Seque;
template < typename value_type_, typename ref_, typename ptr_ >
class seque_iterator;

constexpr int block_size_shift = 10; // block size = 2^block_size_shift
constexpr int max_block_size = 1L << block_size_shift;
constexpr int max_block_size_sub_1 = max_block_size - 1;

template < typename value_type_, typename ref_, typename ptr_ >
class seque_iterator
{
  friend class Seque< value_type_ >;

  // Making all templated iterators friends to allow converting
  // iterator -> const_iterator.
  // TODO: Explicit conversion?
  template < typename, typename, typename >
  friend class seque_iterator;

private:
  template < typename cv_value_type_ >
  using _iter = seque_iterator< value_type_, cv_value_type_&, cv_value_type_* >;

  const Seque< value_type_ >* seque_;
  size_t block_index_;
  typename std::vector< value_type_ >::const_iterator block_it_;
  typename std::vector< value_type_ >::const_iterator current_block_end_;

public:
  using iterator = _iter< value_type_ >;
  using const_iterator = _iter< const value_type_ >;

  using iterator_category = std::random_access_iterator_tag;
  using value_type = value_type_;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = size_t;

  explicit seque_iterator( const Seque< value_type_ >& );
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

template < typename value_type_ >
class Seque
{
  template < typename cv_value_type_, typename ref_, typename ptr_ >
  friend class seque_iterator;

public:
  using iterator = seque_iterator< value_type_, value_type_&, value_type_* >;
  using const_iterator =
    seque_iterator< value_type_, const value_type_&, const value_type_* >;

  Seque();
  explicit Seque( size_t );
  Seque( const Seque< value_type_ >& );
  virtual ~Seque();

  value_type_& operator[]( const size_t pos );
  const value_type_& operator[]( const size_t pos ) const;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  void push_back( const value_type_& value );
  void clear();
  size_t size() const;
  iterator erase( const_iterator, const_iterator );
  void print_blocks() const;
  int get_max_block_size() const;

private:
  std::vector< std::vector< value_type_ > > blockmap_;
  iterator finish_;
};

/////////////////////////////////////////////////////////////
//               Seque method implementation               //
/////////////////////////////////////////////////////////////

template < typename value_type_ >
inline Seque< value_type_ >::Seque()
  : blockmap_( std::vector< std::vector< value_type_ > >( 1,
      std::vector< value_type_ >( max_block_size ) ) )
  , finish_( begin() )
{
}

template < typename value_type_ >
inline Seque< value_type_ >::Seque( size_t n )
  : blockmap_( std::vector< std::vector< value_type_ > >( 1,
      std::vector< value_type_ >( max_block_size ) ) )
  , finish_( begin() )
{
  size_t num_blocks_needed = std::ceil( ( float ) n / max_block_size );
  for ( size_t i = 0; i < num_blocks_needed - 1; ++i )
  {
    blockmap_.push_back( std::vector< value_type_ >( max_block_size ) );
  }
  finish_ += n;
}

template < typename value_type_ >
inline Seque< value_type_ >::Seque( const Seque< value_type_ >& other )
  : blockmap_( other.blockmap_ )
  , finish_( begin() + ( other.finish_ - other.begin() ) )
{
}

template < typename value_type_ >
inline Seque< value_type_ >::~Seque() = default;

template < typename value_type_ >
inline value_type_& Seque< value_type_ >::operator[]( const size_t pos )
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_sub_1;
  return blockmap_[ block_index ][ element_index ];
}

template < typename value_type_ >
inline const value_type_& Seque< value_type_ >::operator[](
  const size_t pos ) const
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_sub_1;
  return blockmap_[ block_index ][ element_index ];
}

template < typename value_type_ >
inline typename Seque< value_type_ >::iterator
Seque< value_type_ >::begin()
{
  return iterator( *this );
}

template < typename value_type_ >
inline typename Seque< value_type_ >::const_iterator
Seque< value_type_ >::begin() const
{
  return const_iterator( *this );
}

template < typename value_type_ >
inline typename Seque< value_type_ >::iterator
Seque< value_type_ >::end()
{
  return finish_;
}

template < typename value_type_ >
inline typename Seque< value_type_ >::const_iterator
Seque< value_type_ >::end() const
{
  return finish_;
}

template < typename value_type_ >
inline void
Seque< value_type_ >::push_back( const value_type_& value )
{
  // If this is the last element in the current block, add another block
  if ( finish_.block_it_ == finish_.current_block_end_ - 1 )
  {
    blockmap_.emplace_back( max_block_size );
  }
  *finish_ = value;
  ++finish_;
}

template < typename value_type_ >
inline void
Seque< value_type_ >::clear()
{
  for ( auto it = blockmap_.begin(); it != blockmap_.end(); ++it )
  {
    it->clear();
    // Reset to default-initialized elements
    auto new_empty = std::vector< value_type_ >( max_block_size );
    std::swap( *it, new_empty );
  }
  finish_ = begin();
}

template < typename value_type_ >
inline size_t
Seque< value_type_ >::size() const
{
  size_t element_index;
  if ( finish_.block_index_ >= blockmap_.size() )
  {
    element_index = 0;
  }
  else
  {
    element_index =
      finish_.block_it_ - blockmap_[ finish_.block_index_ ].begin();
  }
  return finish_.block_index_ * max_block_size + element_index;
}

template < typename value_type_ >
inline typename Seque< value_type_ >::iterator
Seque< value_type_ >::erase( const_iterator first, const_iterator last )
{
  assert( first.seque_ == this );
  assert( last.seque_ == this );
  assert( last < finish_ or last == finish_ );
  // TODO: Except for some special cases, this method may kill performance as
  // last and every subsequent element has to be moved to fill the erased space.
  if ( first == last )
  {
    return iterator( first );
  }
  else if ( first == begin() && last == end() )
  {
    clear();
    return end();
  }
  else
  {
    auto repl_it = first; // Iterator for elements to be replaced.
    for ( auto element = last; element != end(); ++element )
    {
      *repl_it = std::move( *element );
      ++repl_it;
    }
    // The block that repl_it ends up in is the new final block.
    auto& new_finish_block = blockmap_[ repl_it.block_index_ ];
    // Here, the arithmetic says that we first subtract
    // new_finish_block.begin(), then add it again (which seems unnecessary).
    // But what we really do is extracting the element index of repl_it, then
    // fast-forwarding new_finish_block.begin() to that index.
    auto element_index = repl_it.block_it_ - new_finish_block.begin();
    // Erase everything after the replaced elements in the current block.
    new_finish_block.erase(
      new_finish_block.begin() + element_index, new_finish_block.end() );
    // Refill the erased elements in the current block with default-initialized
    // elements.
    int num_default_init = max_block_size - new_finish_block.size();
    for ( int i = 0; i < num_default_init; ++i )
    {
      new_finish_block.emplace_back( value_type_{} );
    }
    assert( new_finish_block.size() == max_block_size );
    // Erase all subsequent blocks.
    blockmap_.erase(
      blockmap_.begin() + repl_it.block_index_ + 1, blockmap_.end() );
    // Construct new finish_ iterator
    finish_ = repl_it;
    // The iterator which is to be returned is located where the first element
    // after the last deleted element will be after filling the erased elements,
    // which is the position of the first deleted element.
    return first;
  }
}

template < typename value_type_ >
inline void
Seque< value_type_ >::print_blocks() const
{
  std::cerr << "this: \t\t" << this << "\n";
  std::cerr << "finish seque: \t" << finish_.seque_ << "\n";
  std::cerr << "Blockmap size: " << blockmap_.size() << "\n";
  std::cerr << "Finish block: " << finish_.block_index_ << "\n";
  std::cerr << "==============================================\n";
  auto seq_iter = begin();
  for ( size_t block_index = 0;
        block_index != blockmap_.size() and seq_iter != end();
        ++block_index )
  {
    std::cerr << "----------------------------------------------\n";
    auto& block = blockmap_[ block_index ];
    std::cerr << "Block size: " << block.size() << "\n";
    for ( auto block_it = block.begin();
          block_it != block.end() and seq_iter != end();
          ++block_it )
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
Seque< value_type_ >::get_max_block_size() const
{
  return max_block_size;
}

/////////////////////////////////////////////////////////////
//        Seque iterator method implementation             //
/////////////////////////////////////////////////////////////

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >::seque_iterator(
  const Seque< value_type_ >& seque )
  : seque_( &seque )
  , block_index_( 0 )
  , block_it_( seque_->blockmap_[ block_index_ ].begin() )
  , current_block_end_( seque_->blockmap_[ block_index_ ].end() )
{
}

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >::seque_iterator(
  const iterator& other )
  : seque_( other.seque_ )
  , block_index_( other.block_index_ )
  , block_it_( other.block_it_ )
  , current_block_end_( other.current_block_end_ )
{
}

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >::seque_iterator(
  const const_iterator& other )
  : seque_( other.seque_ )
  , block_index_( other.block_index_ )
  , block_it_( other.block_it_ )
  , current_block_end_( other.current_block_end_ )
{
}

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >&
  seque_iterator< value_type_, ref_, ptr_ >::
  operator=( const seque_iterator< value_type_, ref_, ptr_ >& other )
{
  assert( seque_ == other.seque_ );
  seque_ = other.seque_;
  block_index_ = other.block_index_;
  block_it_ = other.block_it_;
  current_block_end_ = other.current_block_end_;
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >&
  seque_iterator< value_type_, ref_, ptr_ >::
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

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >&
  seque_iterator< value_type_, ref_, ptr_ >::
  operator--()
{
  // If we are still within the block, we can just decrement the block iterator.
  // If not, we need to switch to the previous block.
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

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >&
  seque_iterator< value_type_, ref_, ptr_ >::
  operator+=( size_t val )
{
  for ( size_t i = 0; i < val; ++i )
  {
    operator++();
  }
  return *this;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline seque_iterator< value_type_, ref_, ptr_ >
  seque_iterator< value_type_, ref_, ptr_ >::operator+( size_t val )
{
  seque_iterator tmp = *this;
  return tmp += val;
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename seque_iterator< value_type_, ref_, ptr_ >::reference
  seque_iterator< value_type_, ref_, ptr_ >::
  operator*() const
{
  // TODO: Using const_cast  to remove the constness isn't the most elegant
  // solution. There is probably a better way to do this.
  return const_cast< reference >( *block_it_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename seque_iterator< value_type_, ref_, ptr_ >::pointer
  seque_iterator< value_type_, ref_, ptr_ >::
  operator->() const
{
  // TODO: Again, using const_cast  to remove the constness isn't the most
  // elegant solution. There is probably a better way to do this.
  return const_cast< pointer >( &( *block_it_ ) );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline typename seque_iterator< value_type_, ref_, ptr_ >::difference_type
  seque_iterator< value_type_, ref_, ptr_ >::
  operator-( const seque_iterator< value_type_, ref_, ptr_ >& other ) const
{
  // TODO: Might not return what it should.. (but see source_table.h::L413)
  auto this_element_index =
    block_it_ - seque_->blockmap_[ block_index_ ].begin();
  auto other_element_index =
    other.block_it_ - other.seque_->blockmap_[ other.block_index_ ].begin();
  return ( block_index_ - other.block_index_ ) * max_block_size
    + ( this_element_index - other_element_index );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool seque_iterator< value_type_, ref_, ptr_ >::operator==(
  const seque_iterator< value_type_, ref_, ptr_ >& rhs ) const
{
  return ( block_index_ == rhs.block_index_ and block_it_ == rhs.block_it_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool seque_iterator< value_type_, ref_, ptr_ >::operator!=(
  const seque_iterator< value_type_, ref_, ptr_ >& rhs ) const
{
  return ( block_index_ != rhs.block_index_ or block_it_ != rhs.block_it_ );
}

template < typename value_type_, typename ref_, typename ptr_ >
inline bool seque_iterator< value_type_, ref_, ptr_ >::operator<(
  const seque_iterator& rhs ) const
{
  return ( block_index_ < rhs.block_index_
    or ( block_index_ == rhs.block_index_ and block_it_ < rhs.block_it_ ) );
}

#endif /* SEQUE_H_ */
