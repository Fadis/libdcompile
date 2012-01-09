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

#include <string>
#include <vector>

#include <dcompile/context_holder.hpp>
#include <dcompile/loader.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/unordered_set.hpp>

#include <llvm/LLVMContext.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/DynamicLibrary.h>

namespace dcompile {
  loader::loader( const boost::shared_ptr< llvm::LLVMContext > &_context ) : context_holder( _context ), enable_system_path( false ) {
  }
  void loader::enableSystemPath( bool flag ) {
    enable_system_path = flag;
    if( enable_system_path && system_path.empty() ) {
      std::vector< llvm::sys::Path > llvm_system_path;
      llvm::sys::Path::GetSystemLibraryPaths ( llvm_system_path );
      system_path.reserve( llvm_system_path.size() );
      for(
        std::vector< llvm::sys::Path >::const_iterator iter = llvm_system_path.begin();
        iter != llvm_system_path.end();
        ++iter
      ) system_path.push_back( boost::filesystem::path( iter->c_str() ) );
    }
  }
  bool loader::load( const std::string &name ) const {
    boost::optional< boost::filesystem::path > path = findLib( name );
    if( path )
      return !llvm::sys::DynamicLibrary::LoadLibraryPermanently( path->c_str() );
    else
      return false;
  }
  boost::optional< boost::filesystem::path > loader::findLib( const std::string &name ) const {
    llvm::sys::Path llvm_path;
    llvm_path.set( name.c_str() );
    if ( llvm_path.isDynamicLibrary() || llvm_path.isBitcodeFile() )
      return boost::filesystem::path( name );
    for(
      boost::unordered_set< boost::filesystem::path >::const_iterator iter = user_path.begin();
      iter != user_path.end();
      ++iter
    ) {
      boost::optional< boost::filesystem::path > path = findLibInDirectory( name, *iter );
      if( path )
        return path;
    }
    if( enable_system_path ) {
      for(
        std::vector< boost::filesystem::path >::const_iterator iter = system_path.begin();
        iter != system_path.end();
        ++iter
      ) {
        boost::optional< boost::filesystem::path > path = findLibInDirectory( name, *iter );
        if( path )
          return path;
      }
    }
    return boost::optional< boost::filesystem::path >();
  }
  boost::optional< boost::filesystem::path > loader::findLibInDirectory( const std::string &name, const boost::filesystem::path &path ) const {
    llvm::sys::Path llvm_path;
    llvm_path.set( path.c_str() );
    llvm_path.appendComponent( "lib" + name );
    llvm_path.appendSuffix( llvm::sys::Path::GetDLLSuffix() );
    if ( llvm_path.isDynamicLibrary() || llvm_path.isBitcodeFile() )
      return boost::filesystem::path( llvm_path.c_str() );
    llvm_path.eraseSuffix();
    if ( llvm_path.isDynamicLibrary() || llvm_path.isBitcodeFile() )
      return boost::filesystem::path( llvm_path.c_str() );
    return boost::optional< boost::filesystem::path >();
  }
}
