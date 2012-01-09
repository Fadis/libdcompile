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

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
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
#include <llvm/Support/DynamicLibrary.h>

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
    const boost::shared_ptr< llvm::LLVMContext > &_llvm_context,
    const boost::shared_ptr< llvm::EngineBuilder > &_builder,
    const boost::shared_ptr< llvm::ExecutionEngine > &_engine,
    llvm::Module *_module,
    llvm::Function *_function
  ) : context_holder( _llvm_context ), builder( _builder ), engine( _engine ), llvm_module( _module ),
      entry_point( _function ) {}
  void function::operator()() {
    std::vector< llvm::GenericValue > run_args( 0 );
    if( entry_point->getFunctionType()->getNumParams() != 0 )
      throw InvalidArgument();
    llvm::GenericValue Result = engine->runFunction( entry_point, run_args );
  }

  module::module(
     const boost::shared_ptr< llvm::LLVMContext > &context,
     OptimizeLevel optlevel,
     const boost::shared_ptr< TemporaryFile > &file
  ) : context_holder( context ), bc_file( file ) {
    llvm::SMDiagnostic Err;
    llvm_module = llvm::ParseIRFile( bc_file->getPath().c_str(), Err, *getContext() );
    if (!llvm_module) {
      Err.print( "dcompile::module", llvm::errs());
      throw UnableToLoadModule();
    }
    std::string ErrorMsg;
    if (llvm_module->MaterializeAllPermanently(&ErrorMsg)) {
      llvm::errs() << "dcompile::module" << ": bitcode didn't read correctly.\n";
      llvm::errs() << "Reason: " << ErrorMsg << "\n";
      throw UnableToLoadModule();
    }
    llvm::EngineBuilder *engine_builder = new llvm::EngineBuilder( llvm_module );
    engine_builder->setErrorStr(&ErrorMsg);
    engine_builder->setEngineKind(llvm::EngineKind::JIT);
    switch( optlevel ) {
      case None:
        engine_builder->setOptLevel( llvm::CodeGenOpt::None );
        break;
      case Less:
        engine_builder->setOptLevel( llvm::CodeGenOpt::Less );
        break;
      case Default:
        engine_builder->setOptLevel( llvm::CodeGenOpt::Default );
        break;
      case Aggressive:
        engine_builder->setOptLevel( llvm::CodeGenOpt::Aggressive );
        break;
      default:
        throw UnknownOptimizeLevel();
    };
    engine.reset( engine_builder->create() );
    builder.reset( engine_builder, boost::bind( &module::deleteBuilder, _1, engine ) );
    builder->setRelocationModel(llvm::Reloc::Default);
    builder->setCodeModel( llvm::CodeModel::JITDefault );
    builder->setUseMCJIT(true);
    if (!engine) {
      if (!ErrorMsg.empty())
        llvm::errs() << "dcompile::module" << ": error creating EE: " << ErrorMsg << "\n";
      else
        llvm::errs() << "dcompile::module" << ": unknown error creating EE!\n";
      throw UnableToLoadModule();
    }
    engine->RegisterJITEventListener(llvm::createOProfileJITEventListener());
    engine->DisableLazyCompilation(true);
    engine->runStaticConstructorsDestructors(false);
  }
  int module::operator()( const std::vector< std::string > &argv, char * const *envp ) {
    llvm::Function *entry_point = llvm_module->getFunction( "main" );
    return engine->runFunctionAsMain( entry_point, argv, envp );
  }
  boost::optional< function > module::getFunction( const std::string &name ) {
    llvm::Function *entry_point = llvm_module->getFunction( name.c_str() );
    if( !entry_point )
      return boost::optional< function >();
    return function( getContext(), builder, engine, llvm_module, entry_point );
  }
  void module::deleteBuilder( llvm::EngineBuilder *builder, boost::shared_ptr< llvm::ExecutionEngine > engine ) {
    engine->runStaticConstructorsDestructors(true);
    delete builder;
  }
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
  dynamic_compiler::dynamic_compiler()
  : resource_directory( boost::filesystem::path( BOOST_PP_STRINGIZE( RESOURCE_DIRECTORY ) ) / CLANG_VERSION_STRING ),
    optlevel( Aggressive ), library_loader( getContext() ) {
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
