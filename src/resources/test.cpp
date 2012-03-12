typedef float* host_buffer;
const unsigned int vector_size = 64;
typedef float buffer __attribute__((ext_vector_type(64)));
typedef bool bool_buffer __attribute__((ext_vector_type(64)));
extern "C" float floorf(float x);
extern "C" float ceilf(float x);


const unsigned int sin_table_size = 512;
const float sin_table[ sin_table_size ] = {
  0,0.0122715,0.0245412,0.0368072,0.0490677,0.0613207,0.0735646,0.0857973,
  0.0980171,0.110222,0.122411,0.134581,0.14673,0.158858,0.170962,0.18304,
  0.19509,0.207111,0.219101,0.231058,0.24298,0.254866,0.266713,0.27852,
  0.290285,0.302006,0.313682,0.32531,0.33689,0.348419,0.359895,0.371317,
  0.382683,0.393992,0.405241,0.41643,0.427555,0.438616,0.449611,0.460539,
  0.471397,0.482184,0.492898,0.503538,0.514103,0.52459,0.534998,0.545325,
  0.55557,0.565732,0.575808,0.585798,0.595699,0.605511,0.615232,0.62486,
  0.634393,0.643832,0.653173,0.662416,0.671559,0.680601,0.689541,0.698376,
  0.707107,0.715731,0.724247,0.732654,0.740951,0.749136,0.757209,0.765167,
  0.77301,0.780737,0.788346,0.795837,0.803208,0.810457,0.817585,0.824589,
  0.83147,0.838225,0.844854,0.851355,0.857729,0.863973,0.870087,0.87607,
  0.881921,0.88764,0.893224,0.898674,0.903989,0.909168,0.91421,0.919114,
  0.92388,0.928506,0.932993,0.937339,0.941544,0.945607,0.949528,0.953306,
  0.95694,0.960431,0.963776,0.966976,0.970031,0.97294,0.975702,0.978317,
  0.980785,0.983105,0.985278,0.987301,0.989177,0.990903,0.99248,0.993907,
  0.995185,0.996313,0.99729,0.998118,0.998795,0.999322,0.999699,0.999925,
  1,0.999925,0.999699,0.999322,0.998795,0.998118,0.99729,0.996313,
  0.995185,0.993907,0.99248,0.990903,0.989177,0.987301,0.985278,0.983105,
  0.980785,0.978317,0.975702,0.97294,0.970031,0.966976,0.963776,0.960431,
  0.95694,0.953306,0.949528,0.945607,0.941544,0.937339,0.932993,0.928506,
  0.92388,0.919114,0.91421,0.909168,0.903989,0.898674,0.893224,0.88764,
  0.881921,0.87607,0.870087,0.863973,0.857729,0.851355,0.844854,0.838225,
  0.83147,0.824589,0.817585,0.810457,0.803207,0.795837,0.788346,0.780737,
  0.77301,0.765167,0.757209,0.749136,0.740951,0.732654,0.724247,0.715731,
  0.707107,0.698376,0.689541,0.680601,0.671559,0.662416,0.653173,0.643832,
  0.634393,0.624859,0.615232,0.605511,0.595699,0.585798,0.575808,0.565732,
  0.55557,0.545325,0.534998,0.52459,0.514103,0.503538,0.492898,0.482184,
  0.471397,0.460539,0.449611,0.438616,0.427555,0.41643,0.405241,0.393992,
  0.382684,0.371317,0.359895,0.348419,0.33689,0.32531,0.313682,0.302006,
  0.290285,0.27852,0.266713,0.254866,0.24298,0.231058,0.219101,0.207111,
  0.19509,0.18304,0.170962,0.158858,0.146731,0.134581,0.122411,0.110222,
  0.0980171,0.0857972,0.0735644,0.0613208,0.0490677,0.0368072,0.0245412,0.0122715,
  -8.74228e-08,-0.0122714,-0.0245411,-0.0368072,-0.0490677,-0.0613208,-0.0735646,-0.0857974,
  -0.098017,-0.110222,-0.122411,-0.134581,-0.14673,-0.158858,-0.170962,-0.18304,
  -0.19509,-0.207111,-0.219101,-0.231058,-0.24298,-0.254866,-0.266713,-0.27852,
  -0.290285,-0.302006,-0.313682,-0.32531,-0.33689,-0.348419,-0.359895,-0.371317,
  -0.382683,-0.393992,-0.405241,-0.41643,-0.427555,-0.438616,-0.449611,-0.460539,
  -0.471397,-0.482184,-0.492898,-0.503538,-0.514103,-0.52459,-0.534998,-0.545325,
  -0.55557,-0.565732,-0.575808,-0.585798,-0.595699,-0.605511,-0.615232,-0.62486,
  -0.634393,-0.643831,-0.653173,-0.662416,-0.671559,-0.680601,-0.689541,-0.698376,
  -0.707107,-0.715731,-0.724247,-0.732654,-0.740951,-0.749136,-0.757209,-0.765167,
  -0.77301,-0.780737,-0.788346,-0.795837,-0.803208,-0.810457,-0.817585,-0.824589,
  -0.831469,-0.838225,-0.844853,-0.851355,-0.857729,-0.863973,-0.870087,-0.87607,
  -0.881921,-0.88764,-0.893224,-0.898674,-0.903989,-0.909168,-0.91421,-0.919114,
  -0.923879,-0.928506,-0.932993,-0.937339,-0.941544,-0.945607,-0.949528,-0.953306,
  -0.95694,-0.960431,-0.963776,-0.966977,-0.970031,-0.97294,-0.975702,-0.978317,
  -0.980785,-0.983105,-0.985278,-0.987301,-0.989177,-0.990903,-0.99248,-0.993907,
  -0.995185,-0.996313,-0.99729,-0.998118,-0.998795,-0.999322,-0.999699,-0.999925,
  -1,-0.999925,-0.999699,-0.999322,-0.998795,-0.998118,-0.99729,-0.996313,
  -0.995185,-0.993907,-0.99248,-0.990903,-0.989177,-0.987301,-0.985278,-0.983105,
  -0.980785,-0.978317,-0.975702,-0.97294,-0.970031,-0.966977,-0.963776,-0.960431,
  -0.95694,-0.953306,-0.949528,-0.945607,-0.941544,-0.937339,-0.932993,-0.928506,
  -0.923879,-0.919114,-0.91421,-0.909168,-0.903989,-0.898674,-0.893224,-0.88764,
  -0.881921,-0.87607,-0.870087,-0.863973,-0.857729,-0.851355,-0.844853,-0.838225,
  -0.83147,-0.824589,-0.817585,-0.810457,-0.803208,-0.795837,-0.788346,-0.780737,
  -0.77301,-0.765167,-0.757209,-0.749136,-0.740951,-0.732654,-0.724247,-0.715731,
  -0.707107,-0.698376,-0.689541,-0.680601,-0.671559,-0.662416,-0.653173,-0.643831,
  -0.634393,-0.624859,-0.615231,-0.605511,-0.595699,-0.585798,-0.575808,-0.565732,
  -0.55557,-0.545325,-0.534998,-0.52459,-0.514103,-0.503538,-0.492898,-0.482184,
  -0.471397,-0.460539,-0.449612,-0.438616,-0.427555,-0.41643,-0.405241,-0.393992,
  -0.382683,-0.371317,-0.359895,-0.348419,-0.33689,-0.32531,-0.313682,-0.302006,
  -0.290285,-0.27852,-0.266713,-0.254866,-0.24298,-0.231058,-0.219101,-0.207111,
  -0.19509,-0.18304,-0.170962,-0.158858,-0.14673,-0.134581,-0.122411,-0.110222,
  -0.0980172,-0.0857974,-0.0735646,-0.0613207,-0.0490676,-0.0368072,-0.0245411,-0.0122714,
};


const unsigned int max_uniforms = 200;
const unsigned int max_variables = 16;
const unsigned int max_locals = 16;

float uniforms[ max_uniforms ];
unsigned int variables_size[ max_variables ];
const float *variables[ max_variables ];

inline float uniform( unsigned int name ) {
  return uniforms[ name ];
}
inline buffer vectorize( float src ) {
  buffer temp;
  for( unsigned int index = 0; index != vector_size; ++index )
    temp[ index ] = src;
  return temp;
}
inline buffer count() {
  buffer temp;
  for( unsigned int index = 0; index != vector_size; ++index )
    temp[ index ] = index;
  return temp;
}
inline float linear( float a, float b, float src ) {
  return a * src + b;
}
inline buffer linear( float a, float b, const buffer &src ) {
  return vectorize( a ) * src + vectorize( b );
}
inline buffer linear( const buffer & a, const buffer & b, const buffer &src ) {
  return a * src + b;
}
inline float interpolate( float l, float r, float p ) {
  return l * p + r * ( 1.0f - p );
}
inline buffer interpolate( const buffer &l, const buffer &r, const buffer &p ) {
  return l * p + r * ( vectorize( 1.0f ) - p );
}
inline float floor( float src ) {
  return floorf( src );
}
inline buffer floor( const buffer &src ) {
  buffer temp;
  for( unsigned int index = 0; index != vector_size; ++index ) {
    temp[ index ] = floor( src[ index ] );
  }
  return temp;
}
inline float ceil( float src ) {
  return ceilf( src );
}
inline buffer ceil( const buffer &src ) {
  buffer temp;
  for( unsigned int index = 0; index != vector_size; ++index ) {
    temp[ index ] = ceil( src[ index ] );
  }
  return temp;
}
inline float loop( float src ) {
  return src - floor( src );
}
inline buffer loop( const buffer &src ) {
  return src - floor( src );
}
inline float sin( float src ) {
  float level = loop( src ) * sin_table_size;
  float lower = floor( level );
  float higher = ceil( level );
  lower = sin_table[ static_cast< int >( lower ) ];
  higher = sin_table[ static_cast< int >( higher ) % sin_table_size ];
  return interpolate( lower, higher, loop( level ) );
}
inline buffer sin( const buffer &src ) {
  buffer level = loop( src ) * vectorize( sin_table_size );
  buffer lower = floor( level );
  buffer higher = ceil( level );
  for( unsigned int index = 0; index != vector_size; ++index ) {
    lower[ index ] = sin_table[ static_cast< int >( lower[ index ] ) ];
    higher[ index ] = sin_table[ static_cast< int >( higher[ index ] ) % sin_table_size ];
  }
  return interpolate( lower, higher, loop( level ) );
}
inline float cos( float src ) {
  return sin( src + 0.5f );
}
inline buffer cos( const buffer &src ) {
  return sin( src + vectorize( 0.5f ) );
}
inline float variable( unsigned int name, float src ) {
  float level = loop( src ) * variables_size[ name ];
  float lower = floor( level );
  float higher = ceil( level );
  lower = variables[ name ][ static_cast< int >( lower ) ];
  higher = variables[ name ][ static_cast< int >( higher ) % variables_size[ name ] ];
  return interpolate( lower, higher, loop( level ) );
}
inline buffer variable( unsigned int name, const buffer &src ) {
  buffer level = loop( src ) * vectorize( variables_size[ name ] );
  buffer lower = floor( level );
  buffer higher = ceil( level );
  for( unsigned int index = 0; index != vector_size; ++index ) {
    lower[ index ] = variables[ name ][ static_cast< int >( lower[ index ] ) ];
    higher[ index ] = variables[ name ][ static_cast< int >( higher[ index ] ) % variables_size[ name ] ];
  }
  return interpolate( lower, higher, loop( level ) );
}
inline float envelope( float src, float attack, float decay, float sustain, float release ) {
  if( src < attack )
    return interpolate( 0.0f, 1.0f, src/attack );
  src -= attack;
  if( src < decay )
    return interpolate( 1.0f, sustain, src/decay );
  return sustain;
}
inline buffer envelope( buffer src, buffer attack, buffer decay, buffer sustain, buffer release ) {
  buffer result;
  for( unsigned int index = 0; index != vector_size; ++index ) {
    result[ index ] = envelope( src[ index ], attack[ index ], decay[ index ], sustain[ index ], release[ index ] );
  }
  return result;
}

struct context {
  inline buffer operator()() {
    buffer time = linear( uniform( 0 ), uniform( 1 ), count() );
    return sin( variable( 2, time ) );
  }
  inline float local( unsigned int name ) {
    return uniforms[ name ];
  }
  float uniforms[ max_locals ];
  buffer result;
};

extern "C" void setVariables( unsigned int name, unsigned int size, float *value ) {
  variables[ name ] = value;
  variables_size[ name ] = size;
}

extern "C" void setUniform( unsigned int name, float value ) {
  uniforms[ name ] =value;
}

extern "C" void getContextSize( unsigned int *size ) {
  *size = sizeof( context );
}

extern "C" void createContext( context *inst ) {
}

extern "C" void destroyContext( context *inst ) {
}

extern "C" void setLocal( context *ctx, unsigned int name, float value ) {
  ctx->uniforms[ name ] =value;
}

extern "C" void getResult( context *ctx, float *value ) {
  ctx->result = (*ctx)();
  for( unsigned int index = 0; index != vector_size; ++index ) {
    value[ index ] = ctx->result[ index ];
  }
}

#include <iostream>

int main() {
  context *ctx = createContext();
  float final[ 64 ];
  float v[ 64 ];
  for( int index = 0; index != 64; ++index )
    v[ index ] = index;
  setVariables( 2, 64, v );
  setUniform( 0, 0.01f );
  setUniform( 1, 0.5f );
  getResult( ctx, final );
  destroyContext( ctx );
  for( int index = 0; index != 64; ++index )
    std::cout << final[ index ] << " ";
  std::cout << std::endl;
}
