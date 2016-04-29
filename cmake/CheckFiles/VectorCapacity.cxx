#include <vector>

int main(void)
{
  std::vector< int > v;
  v.push_back( 7 );
  return v.capacity() > 1;
}
