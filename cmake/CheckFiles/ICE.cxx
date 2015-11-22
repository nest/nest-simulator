class RandomDev 
{
public:
  virtual double operator()(void) =0; 
  virtual double operator()(double x) =0; 
};

class GenericRandomDevFactory 
{
public:
  virtual RandomDev* create() const =0;
};

template <typename DevType>
class RandomDevFactory: public GenericRandomDevFactory 
{  
public:
  RandomDev* create() const
  {
    return new DevType();
  }      
};

template <typename T>
class Wrapper: public T 
{
public:  
  using RandomDev::operator();
  double operator()(void) { return 0.0; }
  double operator()(double x) { return x; }
};

int main(int argc, char const *argv[])
{
  RandomDevFactory<Wrapper<RandomDev> > r;
  return 0;
}
