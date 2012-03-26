#include <iostream>

extern "C"
void moo( float *a ) {
  std::cout << *a << " inslide" << std::endl;
  std::cout << "Hello, world!" << std::endl;
  *a += 0.1f;
}
