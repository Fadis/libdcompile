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
#include <dcompile/module.hpp>
#include <dcompile/loader.hpp>
#include <dcompile/header_path.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/type_traits.hpp>

#include <llvm/LLVMContext.h>

#include <clang/Frontend/CompilerInstance.h>

namespace dcompile {
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
    module operator()( const std::string &source_code, Language lang = CXX ) const;
    std::string dumpLLVM( const std::string &source_code, Language lang = CXX ) const;
    std::string dumpAsm( const std::string &source_code, Language lang = CXX ) const;
  private:
    void buildArguments( std::vector< std::string > &arguments, const boost::filesystem::path &_from, const boost::filesystem::path &_to ) const;
    void setupCompiler( clang::CompilerInstance &compiler, const boost::filesystem::path &_from, const boost::filesystem::path &_to ) const;
    const boost::filesystem::path resource_directory;
    header_path header;
    loader library_loader;
    OptimizeLevel optlevel;
    boost::shared_ptr< llvm::LLVMContext > llvm_context;
      //    clang::CompilerInstance compiler;
  };
}

#endif
