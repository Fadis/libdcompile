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

#ifndef DCOMPILE_DYNAMIC_COMPILER_HPP
#define DCOMPILE_DYNAMIC_COMPILER_HPP

#include <string>

#include <dcompile/common.hpp>
#include <dcompile/context_holder.hpp>
#include <dcompile/native_target.hpp>
#include <dcompile/module.hpp>
#include <dcompile/object.hpp>
#include <dcompile/loader.hpp>
#include <dcompile/header_path.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include <llvm/LLVMContext.h>
#include <llvm/Linker.h>
#include <llvm/Support/Path.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/CodeGen/CodeGenAction.h>

namespace dcompile {
/*  class result {
  public:
    result( unsigned int _errs, unsigned int _warns, const std::string &_ )
  }*/
  
  class dynamic_compiler : public context_holder {
  public:
    dynamic_compiler();
    dynamic_compiler( const boost::filesystem::path &resource );
    void setOptimizeLevel( OptimizeLevel new_level );
    OptimizeLevel getOptimizeLevel() const;
    loader &getLoader() {
      return library_loader;
    }
    const loader &getLoader() const {
      return library_loader;
    }
    header_path &getHeaderPath() {
      return header;
    }
    const header_path &getHeaderPath() const {
      return header;
    }
    module operator()( const std::string &source_code, Language lang ) const;
    object getObject( const std::string &source_code, Language lang ) const;
    std::string dumpLLVM( const std::string &source_code, Language lang ) const;
    std::string dumpAsm( const std::string &source_code, Language lang ) const;
    module operator()( const boost::filesystem::path &path ) const;
    object getObject( const boost::filesystem::path &path ) const;
    std::string dumpLLVM( const boost::filesystem::path &path ) const;
    std::string dumpAsm( const boost::filesystem::path &path ) const;
    void compileEachSource( boost::shared_ptr< TemporaryFile > &bc, const boost::filesystem::path &src ) const;
    template< typename Iterator >
    module operator()( Iterator begin, Iterator end,
                      typename boost::enable_if< boost::is_same< typename boost::remove_cv< typename boost::iterator_value< Iterator >::type >::type, boost::filesystem::path > >::type* = 0
                      ) const {
      native_target::init();
      std::vector< boost::shared_ptr< TemporaryFile > > bc_file_names( std::distance( begin, end ) );
      typename std::vector< boost::shared_ptr< TemporaryFile > >::iterator bc_iter;
      Iterator src_iter;
      for( src_iter = begin, bc_iter = bc_file_names.begin(); src_iter != end, bc_iter != bc_file_names.end(); ++src_iter, ++bc_iter ) {
        compileEachSource( *bc_iter, *src_iter );
      }
      llvm::Linker linker( "linker", "composite", *getContext().get(), 0 );
      for( bc_iter = bc_file_names.begin(); bc_iter != bc_file_names.end(); ++bc_iter ) {
        llvm::sys::Path bc_path;
        bc_path.set( (*bc_iter)->getPath().c_str() );
        bool is_native;
        linker.LinkInFile( bc_path, is_native );
      }
      return module( getContext(), optlevel, linker.releaseModule() );
    }
    template< typename Iterator >
    object getObject( Iterator begin, Iterator end,
                      typename boost::enable_if< boost::is_same< typename boost::remove_cv< typename boost::iterator_value< Iterator >::type >::type, boost::filesystem::path > >::type* = 0
                      ) const {
      std::vector< boost::shared_ptr< TemporaryFile > > bc_file_names( std::distance( begin, end ) );
      typename std::vector< boost::shared_ptr< TemporaryFile > >::iterator bc_iter;
      Iterator src_iter;
      for( src_iter = begin, bc_iter = bc_file_names.begin(); src_iter != end, bc_iter != bc_file_names.end(); ++src_iter, ++bc_iter ) {
        compileEachSource( *bc_iter, *src_iter );
      }
      llvm::Linker linker( "linker", "composite", *getContext().get(), 0 );
      for( bc_iter = bc_file_names.begin(); bc_iter != bc_file_names.end(); ++bc_iter ) {
        llvm::sys::Path bc_path;
        bc_path.set( (*bc_iter)->getPath().c_str() );
        bool is_native;
        linker.LinkInFile( bc_path, is_native );
      }
      return object( getContext(), optlevel, linker.releaseModule() );
    }
  private:
    void buildArguments( std::vector< std::string > &arguments ) const;
    void buildArguments( std::vector< std::string > &arguments, const boost::filesystem::path &_from, const boost::filesystem::path &_to ) const;
    void setupCompiler( clang::CompilerInstance &compiler, const std::vector< std::string > &arguments ) const;
    void setupCompiler( clang::CompilerInstance &compiler, const boost::filesystem::path &_from, const boost::filesystem::path &_to ) const;
    bool begin( clang::CompilerInstance &compiler, clang::FrontendAction &action, const clang::FrontendInputFile &file ) const;
    void execute( clang::CompilerInstance &compiler, clang::FrontendAction &action ) const;
    module getModule( clang::CompilerInstance &compiler, TemporaryFile &bc_file_name ) const;
    object getObject( clang::CompilerInstance &compiler, TemporaryFile &bc_file_name ) const;
    const boost::filesystem::path resource_directory;
    header_path header;
    loader library_loader;
    OptimizeLevel optlevel;
    boost::shared_ptr< llvm::LLVMContext > llvm_context;
  };
}

#endif
