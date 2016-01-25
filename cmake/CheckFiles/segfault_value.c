#include <stdio.h>
int
main()
{
  char* c = 0;
  sprintf( c, "this operation should provoke a segfault!" );
  return 0; // never reached
}
