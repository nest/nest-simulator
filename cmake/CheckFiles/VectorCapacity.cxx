#include <vector>

/*
 * Test capacity of vector after single push_back().
 *
 * Return 0 only if capacity is 1.
 */
int
main( void )
{
  std::vector< int > vec;
  vec.push_back( 7 );

  // Return 0 if capacity is 1, otherwise return non-zero value.
  return vec.capacity() > 1;
}
