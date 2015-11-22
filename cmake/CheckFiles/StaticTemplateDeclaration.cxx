class A 
{
  int c;
public:
  A(int a)
      :c(a)
  {}
};

template <typename T, typename L>
class B 
{
 protected:
  static A f;
};
template<> A B<int, double>::f;
template<> A B<int,double>::f(9);

int main(int argc, char const *argv[])
{
  /* code */
  return 0;
}
