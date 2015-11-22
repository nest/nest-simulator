#include <vector>

int main( void )
{
  std::vector< int > v;
  long l;
  int i;
  v.push_back( 7 );
  l = v.capacity();
  for( i = 0; i<4; i++)
  {
   while (1)
   { 
    v.push_back(7);
    if (v.capacity()!=l) 
     {
      if (v.capacity()/2 != l)
        return 1;
      l = v.capacity();
      break;
     }
   }
  } 
  return 0;
}
