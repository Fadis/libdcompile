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
#include <fstream>
#include <string>
#include <vector>

#include <boost/type_traits.hpp>
#include <boost/preprocessor.hpp>

#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Linker.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Support/MemoryBuffer.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Version.h>

#include <dcompile/dcompile.hpp>
#include <dcompile/mktemp.hpp>

namespace dcompile {
  function::function(
    boost::shared_ptr< llvm::LLVMContext > _llvm_context,
    boost::shared_ptr< llvm::EngineBuilder > _builder,
    llvm::ExecutionEngine *_engine,
    llvm::Module *_module,
    llvm::Function *_function
  ) : llvm_context( _llvm_context ), builder( _builder ), engine( _engine ), module( _module ),
      entry_point( _function ) {}
  void function::operator()() {
    std::vector< llvm::GenericValue > run_args( 0 );
    if( entry_point->getFunctionType()->getNumParams() != 0 )
      throw InvalidArgument();
    llvm::GenericValue Result = engine->runFunction( entry_point, run_args );
  }

  library::library(
     const boost::shared_ptr< llvm::LLVMContext > &context,
     OptimizeLevel optlevel,
     const boost::shared_ptr< TemporaryFile > &file
  ) : llvm_context( context ), bc_file( file ) {
    llvm::SMDiagnostic Err;
    module = llvm::ParseIRFile( bc_file->getPath().c_str(), Err, *llvm_context );
    if (!module) {
      Err.print( "dcompile::library", llvm::errs());
      throw UnableToLoadModule();
    }
    std::string ErrorMsg;
    if (module->MaterializeAllPermanently(&ErrorMsg)) {
      llvm::errs() << "dcompile::library" << ": bitcode didn't read correctly.\n";
      llvm::errs() << "Reason: " << ErrorMsg << "\n";
      throw UnableToLoadModule();
    }
    builder.reset( new llvm::EngineBuilder( module ) );
    builder->setErrorStr(&ErrorMsg);
    builder->setEngineKind(llvm::EngineKind::JIT);
    switch( optlevel ) {
      case None:
        builder->setOptLevel( llvm::CodeGenOpt::None );
        break;
      case Less:
        builder->setOptLevel( llvm::CodeGenOpt::Less );
        break;
      case Default:
        builder->setOptLevel( llvm::CodeGenOpt::Default );
        break;
      case Aggressive:
        builder->setOptLevel( llvm::CodeGenOpt::Aggressive );
        break;
      default:
        throw UnknownOptimizeLevel();
    };
    engine = builder->create();
    builder->setRelocationModel(llvm::Reloc::Default);
    builder->setCodeModel( llvm::CodeModel::JITDefault );
    builder->setUseMCJIT(true);
    if (!engine) {
      if (!ErrorMsg.empty())
        llvm::errs() << "dcompile::library" << ": error creating EE: " << ErrorMsg << "\n";
      else
        llvm::errs() << "dcompile::library" << ": unknown error creating EE!\n";
      throw UnableToLoadModule();
    }
    engine->RegisterJITEventListener(llvm::createOProfileJITEventListener());
    engine->DisableLazyCompilation(true);
  }
  int library::operator()( const std::vector< std::string > &argv, char * const *envp ) {
    llvm::Function *entry_point = module->getFunction( "main" );
    return engine->runFunctionAsMain( entry_point, argv, envp );
  }
  boost::optional< function > library::getFunction( const std::string &name ) {
    llvm::Function *entry_point = module->getFunction( name.c_str() );
    if( !entry_point )
      return boost::optional< function >();
    return function( llvm_context, builder, engine, module, entry_point );
  }
  dynamic_compiler::dynamic_compiler()
  : resource_directory( boost::filesystem::path( BOOST_PP_STRINGIZE( RESOURCE_DIRECTORY ) ) / CLANG_VERSION_STRING ),
    llvm_context( new llvm::LLVMContext ), optlevel( Aggressive ) {}
  dynamic_compiler::dynamic_compiler( const boost::filesystem::path &resource )
  : resource_directory( resource ), llvm_context( new llvm::LLVMContext ), optlevel( Aggressive ) {}
  void dynamic_compiler::setOptimizeLevel( OptimizeLevel new_level ) {
    optlevel = new_level;
  }
  OptimizeLevel dynamic_compiler::getOptimizeLevel() const {
    return optlevel;
  }
  library dynamic_compiler::operator()( const std::string &source_code ) {
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

    llvm::Triple target_triple;
    target_triple.setArch(llvm::Triple::x86_64);
    target_triple.setVendor(llvm::Triple::Apple);
    target_triple.setOS(llvm::Triple::Darwin);

    clang::TargetOptions target_opts;
    target_opts.Triple = target_triple.getTriple();
    compiler.setTarget( clang::TargetInfo::CreateTargetInfo(
                        compiler.getDiagnostics(),
                        target_opts
                    ) );
    clang::EmitBCAction action( llvm_context.get() );

    compiler.ExecuteAction ( action );
    library lib( llvm_context, optlevel, bc_file_name );
    return lib;
  }
}
