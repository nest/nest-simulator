/*
 *  Built-in implementation of Knuth's LFG generator.
 *  This code is a C++ adaptation of the code published by Knuth on his
 *  website, http://www-cs-faculty.stanford.edu/~knuth/programs/rng.c,
 *  retrieved 8 Jan 2008. See also Knuth's header comment below.
 */

/* Header comment by  D E Knuth ------------------------------------------ */

/*    This program by D E Knuth is in the public domain and freely copyable.
 *    It is explained in Seminumerical Algorithms, 3rd edition, Section 3.6
 *    (or in the errata to the 2nd edition --- see
 *        http://www-cs-faculty.stanford.edu/~knuth/taocp.html
 *    in the changes to Volume 2 on pages 171 and following).              */

/*    N.B. The MODIFICATIONS introduced in the 9th printing (2002) are
      included here; there's no backwards compatibility with the original. */

/*    This version also adopts Brendan McKay's suggestion to
      accommodate naive users who forget to call ran_start(seed).          */

/*    If you find any bugs, please report them immediately to
 *                 taocp@cs.stanford.edu
 *    (and you will be rewarded if the bug is genuine). Thanks!            */

/************ see the book for explanations and caveats! *******************/
/************ in particular, you need two's complement arithmetic **********/

/* End of Header comment by  D E Knuth ----------------------------------- */

#include "knuthlfg.h"

const long librandom::KnuthLFG::KK_ = 100;
const long librandom::KnuthLFG::LL_ = 37;
const long librandom::KnuthLFG::MM_ = 1L << 30;
const long librandom::KnuthLFG::TT_ = 70;
const long librandom::KnuthLFG::QUALITY_ = 1009;
const double librandom::KnuthLFG::I2DFactor_ = 1.0 / librandom::KnuthLFG::MM_;

librandom::KnuthLFG::KnuthLFG( unsigned long seed )
  : ran_x_( KK_ )
  , ran_buffer_( QUALITY_ )
  , end_( ran_buffer_.begin() + KK_ )
  , next_( end_ )
{
  self_test_(); // minimal check
  ran_start_( seed );
}

void
librandom::KnuthLFG::ran_array_( std::vector< long >& rbuff )
{
  const int n = rbuff.size();
  int i, j;
  for ( j = 0; j < KK_; j++ )
  {
    rbuff[ j ] = ran_x_[ j ];
  }
  for ( ; j < n; j++ )
  {
    rbuff[ j ] = mod_diff_( rbuff[ j - KK_ ], rbuff[ j - LL_ ] );
  }
  for ( i = 0; i < LL_; i++, j++ )
  {
    ran_x_[ i ] = mod_diff_( rbuff[ j - KK_ ], rbuff[ j - LL_ ] );
  }
  for ( ; i < KK_; i++, j++ )
  {
    ran_x_[ i ] = mod_diff_( rbuff[ j - KK_ ], ran_x_[ i - LL_ ] );
  }
}

/* the following routines are from exercise 3.6--15 */
/* after calling ran_start, get new randoms by, e.g., "x=ran_arr_next()" */
void
librandom::KnuthLFG::ran_start_( long seed )
{
  int t, j;
  std::vector< long > x( KK_ + KK_ - 1 ); /* the preparation buffer */
  long ss = ( seed + 2 ) & ( MM_ - 2 );
  for ( j = 0; j < KK_; j++ )
  {
    x[ j ] = ss; /* bootstrap the buffer */
    ss <<= 1;
    if ( ss >= MM_ )
    {
      ss -= MM_ - 2; /* cyclic shift 29 bits */
    }
  }
  x[ 1 ]++; /* make x[1] (and only x[1]) odd */
  for ( ss = seed & ( MM_ - 1 ), t = TT_ - 1; t; )
  {
    for ( j = KK_ - 1; j > 0; j-- )
    {
      x[ j + j ] = x[ j ], x[ j + j - 1 ] = 0; /* "square" */
    }
    for ( j = KK_ + KK_ - 2; j >= KK_; j-- )
    {
      x[ j - ( KK_ - LL_ ) ] = mod_diff_( x[ j - ( KK_ - LL_ ) ], x[ j ] ),
                           x[ j - KK_ ] = mod_diff_( x[ j - KK_ ], x[ j ] );
    }
    if ( is_odd_( ss ) )
    { /* "multiply by z" */
      for ( j = KK_; j > 0; j-- )
      {
        x[ j ] = x[ j - 1 ];
      }
      x[ 0 ] = x[ KK_ ]; /* shift the buffer cyclically */
      x[ LL_ ] = mod_diff_( x[ LL_ ], x[ KK_ ] );
    }
    if ( ss )
    {
      ss >>= 1;
    }
    else
    {
      t--;
    }
  }
  for ( j = 0; j < LL_; j++ )
  {
    ran_x_[ j + KK_ - LL_ ] = x[ j ];
  }
  for ( ; j < KK_; j++ )
  {
    ran_x_[ j - LL_ ] = x[ j ];
  }
  for ( j = 0; j < 10; j++ )
  {
    ran_array_( x ); /* warm things up */
  }

  // mark as needing refill
  next_ = end_;
}

void
librandom::KnuthLFG::self_test_()
{
  int m;
  std::vector< long > tbuff( 1009 ); // buffer for test data
  ran_start_( 310952L );
  for ( m = 0; m <= 2009; m++ )
  {
    ran_array_( tbuff );
  }
  assert( tbuff[ 0 ] == 995235265 );

  tbuff.resize( 2009 );
  ran_start_( 310952L );
  for ( m = 0; m <= 1009; m++ )
  {
    ran_array_( tbuff );
  }
  assert( tbuff[ 0 ] == 995235265 );
}
