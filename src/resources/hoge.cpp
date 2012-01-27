#include <iostream>

extern "C" void moo( float *a );

extern "C" void foo( float *a ) {
  moo( a );
}
