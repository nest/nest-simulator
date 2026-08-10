#include <vector>

/*
 * Test capacity of vector after single push_back().
 * We want capacity 1 as starting point for later capacity doubling,
 * so return value 0 indicates success.
 */
int
main( void )
{
  std::vector< int > v;
  v.push_back( 7 );

  // Return 0 if capacity is 1 as desired, non-zero otherwise.
  return v.capacity() > 1;
}
