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
#include <boost/preprocessor/stringize.hpp>

#include <dcompile/dcompile.hpp>

int main() {
  float a = 5.0f;

  boost::filesystem::path sample_source_dir( BOOST_PP_STRINGIZE( SAMPLE_SOURCE_DIRECTORY ) );
  dcompile::dynamic_compiler dc;
  dc.getLoader().enableSystemPath();
  dc.getHeaderPath().enableSystemPath();
  std::vector< dcompile::object > objs;
  objs.push_back( dc.getObject( sample_source_dir/"hoge.cpp" ) );
  objs.push_back( dc.getObject( sample_source_dir/"fuga.cpp" ) );
  dcompile::module mod = dcompile::load( dcompile::link( objs.begin(), objs.end() ) );
  boost::optional< dcompile::function > foo = mod.getFunction( "foo" );
  if( foo )
    (*foo)( &a );
  else
    std::cout << "No function!!" << std::endl;
  std::cout << a << " outside" << std::endl;
}

