#include <iostream>
#include <jpeglib.h>

extern "C" void moo( float *a );

extern "C" void foo( float *a ) {
  struct jpeg_compress_struct cinfo;
  jpeg_create_compress( &cinfo );
  moo( a );
}
