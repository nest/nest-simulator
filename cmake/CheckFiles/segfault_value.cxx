int
main()
{
  // volatile prevents the compiler from eliminating the null dereference as dead code (UB elimination)
  volatile int* p = nullptr;
  *p = 0;
  return 0;
}
