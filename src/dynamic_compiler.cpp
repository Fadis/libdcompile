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

#include <vector>
#include <string>

#include <dcompile/common.hpp>
#include <dcompile/context_holder.hpp>
#include <dcompile/module.hpp>
#include <dcompile/loader.hpp>
#include <dcompile/header_path.hpp>
#include <dcompile/mktemp.hpp>
#include <dcompile/dynamic_compiler.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/unordered_set.hpp>

#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/TargetSelect.h>

#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Version.h>


namespace dcompile {
  dynamic_compiler::dynamic_compiler()
  : resource_directory( boost::filesystem::path( BOOST_PP_STRINGIZE( RESOURCE_DIRECTORY ) ) / CLANG_VERSION_STRING ),
    library_loader( getContext() ), optlevel( Aggressive ) {
    }
  dynamic_compiler::dynamic_compiler( const boost::filesystem::path &resource )
  : resource_directory( resource ), library_loader( getContext() ), optlevel( Aggressive ) {
  }
  void dynamic_compiler::setOptimizeLevel( OptimizeLevel new_level ) {
    optlevel = new_level;
  }
  OptimizeLevel dynamic_compiler::getOptimizeLevel() const {
    return optlevel;
  }
  module dynamic_compiler::operator()( const std::string &source_code ) {
    TemporaryFile source_file_name( 64, ".cpp" );
    boost::shared_ptr< TemporaryFile > bc_file_name( new TemporaryFile( 64, ".bc" ) );
    {
      std::fstream source_file( source_file_name.getPath().c_str(), std::ios::out );
      source_file << source_code;
    }
    std::vector< std::string > arguments;
    arguments.push_back( "dcompile" );
    switch( optlevel ) {
      case None:
        arguments.push_back( "-O0" );
        break;
      case Less:
        arguments.push_back( "-O1" );
        break;
      case Default:
        arguments.push_back( "-O2" );
        break;
      case Aggressive:
        arguments.push_back( "-O3" );
        break;
      default:
        throw UnknownOptimizeLevel();
    };
    if( !header.includeSystemPath() )
      arguments.push_back( "-nostdsysteminc" );
    for(
      boost::unordered_set< boost::filesystem::path >::const_iterator iter = header.getPath().begin();
      iter != header.getPath().end();
      ++iter
    ) {
      arguments.push_back( "-I" );
      arguments.push_back( iter->string() );
    }
    arguments.push_back( source_file_name.getPath().string() );
    arguments.push_back( "-o" );
    arguments.push_back( bc_file_name->getPath().string() );
    arguments.push_back( "-resource-dir" );
    arguments.push_back( resource_directory.string() );
    arguments.push_back( "" );

    std::vector< const char* > cstyle_arguments;
    for( std::vector< std::string >::iterator iter = arguments.begin(); iter != arguments.end(); ++iter )
      cstyle_arguments.push_back( iter->c_str() );


    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    compiler.createDiagnostics( cstyle_arguments.size(), cstyle_arguments.data(), NULL );
    clang::CompilerInvocation::CreateFromArgs(
      compiler.getInvocation(),
      &cstyle_arguments[ 1 ], &cstyle_arguments.back(),
      compiler.getDiagnostics()
    );

    clang::TargetOptions target_opts;
    target_opts.Triple = LLVM_DEFAULT_TARGET_TRIPLE;
    compiler.setTarget( clang::TargetInfo::CreateTargetInfo(
                        compiler.getDiagnostics(),
                        target_opts
                    ) );
    clang::EmitBCAction action( getContext().get() );

    compiler.ExecuteAction ( action );
    module lib( getContext(), optlevel, bc_file_name );
    return lib;
  }
}
