/*****************************************************************************
 * Copyright (C) 2012, Naomasa Matsubayashi                                  *
 * All rights reserved.                                                      *
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright         *
 *    notice, this list of conditions and the following disclaimer.          *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR      *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.   *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,          *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  *
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF  *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.         *
 *                                                                           *
 *****************************************************************************/

#include <iostream>
#include <string>

#include <dcompile/dcompile.hpp>
void hoge() {
  std::cout << "moo" << std::endl;
}

int main() {
  float a[] = { 1.0f, 2.0f, 3.0f, 4.0f };

  std::string source_code = 
  "typedef float float1 __attribute__((ext_vector_type(1)));"
  "typedef float float2 __attribute__((ext_vector_type(2)));"
  "typedef float float3 __attribute__((ext_vector_type(3)));"
  "typedef float float4 __attribute__((ext_vector_type(4)));"
  "extern \"C\" void foo( float *a ) {"
  "  float4 p;"
  "  p.x = a[0];"
  "  p.y = a[1];"
  "  p.z = a[2];"
  "  p.w = a[3];"
  "  p = p * p;"
  "  a[ 0 ] = p.x + p.y + p.z + p.w;"
  "}";
  dcompile::dynamic_compiler dc;
  dc.getLoader().enableSystemPath();
  dc.getHeaderPath().enableSystemPath();
  std::cout << dc.dumpAsm( source_code, dcompile::CXX ) << std::endl;
  boost::optional< dcompile::module > lib = dc( source_code, dcompile::CXX );
  if( lib ) {
    boost::optional< dcompile::function > foo = lib->getFunction( "foo" );
    if( foo )
      (*foo)( &a );
  }
  std::cout << a[0] << " " << a[1] << " " << a[2] << " " << a[3] << std::endl;
}

