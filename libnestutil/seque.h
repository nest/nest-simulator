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

template < typename _value_type >
class Seque;
template < typename _value_type, typename _ref, typename _ptr >
class seque_iterator;

constexpr int block_size_shift = 10; // block size = 2^block_size_shift
constexpr int max_block_size = 1L << block_size_shift;
constexpr int max_block_size_sub_1 = max_block_size - 1;

template < typename _value_type, typename _ref, typename _ptr >
class seque_iterator
{
  friend class Seque< _value_type >;

  // Making all templated iterators friends to allow converting
  // iterator -> const_iterator.
  // TODO: Explicit conversion?
  template < typename, typename, typename >
  friend class seque_iterator;

private:
  template < typename _cv_value_type >
  using _iter = seque_iterator< _value_type, _cv_value_type&, _cv_value_type* >;

  const Seque< _value_type >* seque_;
  size_t block_index_;
  typename std::vector< _value_type >::const_iterator block_it_;
  typename std::vector< _value_type >::const_iterator current_block_end_;

public:
  using iterator = _iter< _value_type >;
  using const_iterator = _iter< const _value_type >;

  using iterator_category = std::random_access_iterator_tag;
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

template < typename _value_type >
class Seque
{
  template < typename _cv_value_type, typename _ref, typename _ptr >
  friend class seque_iterator;

public:
  using iterator = seque_iterator< _value_type, _value_type&, _value_type* >;
  using const_iterator =
    seque_iterator< _value_type, const _value_type&, const _value_type* >;

  Seque();
  explicit Seque( size_t );
  Seque( const Seque< _value_type >& );
  virtual ~Seque();

  _value_type& operator[]( const size_t pos );
  const _value_type& operator[]( const size_t pos ) const;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  void push_back( const _value_type& value );
  void clear();
  size_t size() const;
  iterator erase( const_iterator, const_iterator );
  void print_blocks() const;
  int get_max_block_size() const;

private:
  std::vector< std::vector< _value_type > > blockmap_;
  iterator finish_;
};

/////////////////////////////////////////////////////////////
//               Seque method implementation               //
/////////////////////////////////////////////////////////////

template < typename _value_type >
inline Seque< _value_type >::Seque()
  : blockmap_( std::vector< std::vector< _value_type > >( 1,
      std::vector< _value_type >( max_block_size ) ) )
  , finish_( begin() )
{
}

template < typename _value_type >
inline Seque< _value_type >::Seque( size_t n )
  : blockmap_( std::vector< std::vector< _value_type > >( 1,
      std::vector< _value_type >( max_block_size ) ) )
  , finish_( begin() )
{
  size_t num_blocks_needed = std::ceil( ( float ) n / max_block_size );
  for ( size_t i = 0; i < num_blocks_needed - 1; ++i )
  {
    blockmap_.push_back( std::vector< _value_type >( max_block_size ) );
  }
  finish_ += n;
}

template < typename _value_type >
inline Seque< _value_type >::Seque( const Seque< _value_type >& other )
  : blockmap_( other.blockmap_ )
  , finish_( begin() + ( other.finish_ - other.begin() ) )
{
}

template < typename _value_type >
inline Seque< _value_type >::~Seque() = default;

template < typename _value_type >
inline _value_type& Seque< _value_type >::operator[]( const size_t pos )
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_sub_1;
  return blockmap_[ block_index ][ element_index ];
}

template < typename _value_type >
inline const _value_type& Seque< _value_type >::operator[](
  const size_t pos ) const
{
  // Using bitwise operations to efficiently map the index to the
  // right block and element.
  const auto block_index = pos >> block_size_shift;
  const auto element_index = pos & max_block_size_sub_1;
  return blockmap_[ block_index ][ element_index ];
}

template < typename _value_type >
inline typename Seque< _value_type >::iterator
Seque< _value_type >::begin()
{
  return iterator( *this );
}

template < typename _value_type >
inline typename Seque< _value_type >::const_iterator
Seque< _value_type >::begin() const
{
  return const_iterator( *this );
}

template < typename _value_type >
inline typename Seque< _value_type >::iterator
Seque< _value_type >::end()
{
  return finish_;
}

template < typename _value_type >
inline typename Seque< _value_type >::const_iterator
Seque< _value_type >::end() const
{
  return finish_;
}

template < typename _value_type >
inline void
Seque< _value_type >::push_back( const _value_type& value )
{
  // If this is the last element in the current block, add another block
  if ( finish_.block_it_ == finish_.current_block_end_ - 1 )
  {
    blockmap_.emplace_back( max_block_size );
  }
  *finish_ = value;
  ++finish_;
}

template < typename _value_type >
inline void
Seque< _value_type >::clear()
{
  for ( auto it = blockmap_.begin(); it != blockmap_.end(); ++it )
  {
    it->clear();
    // Reset to default-initialized elements
    auto new_empty = std::vector< _value_type >( max_block_size );
    std::swap( *it, new_empty );
  }
  finish_ = begin();
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
    element_index =
      finish_.block_it_ - blockmap_[ finish_.block_index_ ].begin();
  }
  return finish_.block_index_ * max_block_size + element_index;
}

template < typename _value_type >
inline typename Seque< _value_type >::iterator
Seque< _value_type >::erase( const_iterator first, const_iterator last )
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
    // Saving the iterator which is to be returned. This iterator is  located
    // where the first element after
    // the last deleted element will be after filling the erased elements.
    const const_iterator returnable_iterator = first;
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
      new_finish_block.emplace_back( _value_type{} );
    }
    assert( new_finish_block.size() == max_block_size );
    // Erase all subsequent blocks.
    blockmap_.erase(
      blockmap_.begin() + repl_it.block_index_ + 1, blockmap_.end() );
    // Construct new finish_ iterator
    size_t pos = repl_it.block_index_ * max_block_size + element_index;
    finish_ = begin() + pos;
    assert( finish_ == repl_it );
    return returnable_iterator;
  }
}

template < typename _value_type >
inline void
Seque< _value_type >::print_blocks() const
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

template < typename _value_type >
inline int
Seque< _value_type >::get_max_block_size() const
{
  return max_block_size;
}

/////////////////////////////////////////////////////////////
//        Seque iterator method implementation             //
/////////////////////////////////////////////////////////////

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
  const Seque< _value_type >& seque )
  : seque_( &seque )
  , block_index_( 0 )
  , block_it_( seque_->blockmap_[ block_index_ ].begin() )
  , current_block_end_( seque_->blockmap_[ block_index_ ].end() )
{
}

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
  const iterator& other )
  : seque_( other.seque_ )
  , block_index_( other.block_index_ )
  , block_it_( other.block_it_ )
  , current_block_end_( other.current_block_end_ )
{
}

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >::seque_iterator(
  const const_iterator& other )
  : seque_( other.seque_ )
  , block_index_( other.block_index_ )
  , block_it_( other.block_it_ )
  , current_block_end_( other.current_block_end_ )
{
}

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >&
  seque_iterator< _value_type, _ref, _ptr >::
  operator=( const seque_iterator< _value_type, _ref, _ptr >& other )
{
  assert( seque_ == other.seque_ );
  seque_ = other.seque_;
  block_index_ = other.block_index_;
  block_it_ = other.block_it_;
  current_block_end_ = other.current_block_end_;
  return *this;
}

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >&
  seque_iterator< _value_type, _ref, _ptr >::
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

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >&
  seque_iterator< _value_type, _ref, _ptr >::
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

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >&
  seque_iterator< _value_type, _ref, _ptr >::
  operator+=( size_t val )
{
  for ( size_t i = 0; i < val; ++i )
  {
    operator++();
  }
  return *this;
}

template < typename _value_type, typename _ref, typename _ptr >
inline seque_iterator< _value_type, _ref, _ptr >
  seque_iterator< _value_type, _ref, _ptr >::operator+( size_t val )
{
  seque_iterator tmp = *this;
  return tmp += val;
}

template < typename _value_type, typename _ref, typename _ptr >
inline typename seque_iterator< _value_type, _ref, _ptr >::reference
  seque_iterator< _value_type, _ref, _ptr >::
  operator*() const
{
  // TODO: Using const_cast  to remove the constness isn't the most elegant
  // solution. There is probably a better way to do this.
  return const_cast< reference >( *block_it_ );
}

template < typename _value_type, typename _ref, typename _ptr >
inline typename seque_iterator< _value_type, _ref, _ptr >::pointer
  seque_iterator< _value_type, _ref, _ptr >::
  operator->() const
{
  // TODO: Again, using const_cast  to remove the constness isn't the most
  // elegant solution. There is probably a better way to do this.
  return const_cast< pointer >( &( *block_it_ ) );
}

template < typename _value_type, typename _ref, typename _ptr >
inline typename seque_iterator< _value_type, _ref, _ptr >::difference_type
  seque_iterator< _value_type, _ref, _ptr >::
  operator-( const seque_iterator< _value_type, _ref, _ptr >& other ) const
{
  // TODO: Might not return what it should.. (but see source_table.h::L413)
  auto this_element_index =
    block_it_ - seque_->blockmap_[ block_index_ ].begin();
  auto other_element_index =
    other.block_it_ - other.seque_->blockmap_[ other.block_index_ ].begin();
  return ( block_index_ - other.block_index_ ) * max_block_size
    + ( this_element_index - other_element_index );
}

template < typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator==(
  const seque_iterator< _value_type, _ref, _ptr >& rhs ) const
{
  return ( block_index_ == rhs.block_index_ and block_it_ == rhs.block_it_ );
}

template < typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator!=(
  const seque_iterator< _value_type, _ref, _ptr >& rhs ) const
{
  return ( block_index_ != rhs.block_index_ or block_it_ != rhs.block_it_ );
}

template < typename _value_type, typename _ref, typename _ptr >
inline bool seque_iterator< _value_type, _ref, _ptr >::operator<(
  const seque_iterator& rhs ) const
{
  return ( block_index_ < rhs.block_index_
    or ( block_index_ == rhs.block_index_ and block_it_ < rhs.block_it_ ) );
}

#endif /* SEQUE_H_ */
