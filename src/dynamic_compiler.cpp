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
#include <algorithm>

#include <dcompile/common.hpp>
#include <dcompile/context_holder.hpp>
#include <dcompile/native_target.hpp>
#include <dcompile/module.hpp>
#include <dcompile/object.hpp>
#include <dcompile/loader.hpp>
#include <dcompile/header_path.hpp>
#include <dcompile/mktemp.hpp>
#include <dcompile/dynamic_compiler.hpp>
#include <dcompile/native_target.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/unordered_set.hpp>

#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRReader.h>

#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Version.h>
#include <clang/Basic/FileManager.h>

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
  void dynamic_compiler::buildArguments( std::vector< std::string > &arguments ) const {
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
    arguments.push_back( "-resource-dir" );
    arguments.push_back( resource_directory.string() );
  }
  void dynamic_compiler::buildArguments( std::vector< std::string > &arguments, const boost::filesystem::path &_from, const boost::filesystem::path &_to ) const {
    buildArguments( arguments );
    arguments.push_back( _from.string() );
    arguments.push_back( "-o" );
    arguments.push_back( _to.string() );
    arguments.push_back( "" );
  }
  void dynamic_compiler::setupCompiler( clang::CompilerInstance &compiler, const std::vector< std::string > &arguments ) const {
    std::vector< const char* > cstyle_arguments;
    for( std::vector< std::string >::const_iterator iter = arguments.begin(); iter != arguments.end(); ++iter )
      cstyle_arguments.push_back( iter->c_str() );
    compiler.createDiagnostics( cstyle_arguments.size(), cstyle_arguments.data(), NULL );
    clang::CompilerInvocation::CreateFromArgs(
      compiler.getInvocation(),
      &cstyle_arguments[ 1 ], &cstyle_arguments.back(),
      compiler.getDiagnostics()
    );
    clang::TargetOptions target_opts;
    target_opts.Triple = LLVM_DEFAULT_TARGET_TRIPLE;
    target_opts.CPU = llvm::sys::getHostCPUName();
    compiler.setTarget( clang::TargetInfo::CreateTargetInfo(
                        compiler.getDiagnostics(),
                        target_opts
                    ) );
    compiler.getTarget().setForcedLangOptions(compiler.getLangOpts());
    compiler.createFileManager();
    compiler.createSourceManager( compiler.getFileManager() );
  }
  void dynamic_compiler::setupCompiler( clang::CompilerInstance &compiler, const boost::filesystem::path &_from, const boost::filesystem::path &_to ) const {
    std::vector< std::string > arguments;
    buildArguments( arguments, _from, _to );
    setupCompiler( compiler, arguments );
  }
  bool dynamic_compiler::begin( clang::CompilerInstance &compiler, clang::FrontendAction &action, const clang::FrontendInputFile &file ) const {
    action.setCurrentInput( file );
    action.setCompilerInstance( &compiler );
      //    if( action.BeginInvocation( compiler ) )
      //      goto FAILED;
      /////////// STUB ///////////
  }
  void dynamic_compiler::execute( clang::CompilerInstance &compiler, clang::FrontendAction &action ) const {
    for (unsigned i = 0, e = compiler.getFrontendOpts().Inputs.size(); i != e; ++i) {
      if ( compiler.hasSourceManager())
        compiler.getSourceManager().clearIDTables();
      if ( action.BeginSourceFile( compiler, compiler.getFrontendOpts().Inputs[i] ) ) {
        action.Execute();
        action.EndSourceFile();
      }
    }
    compiler.getDiagnostics().getClient()->finish();
  }
  void dynamic_compiler::compileEachSource( boost::shared_ptr< TemporaryFile > &bc, const boost::filesystem::path &src ) const {
    bc.reset( new TemporaryFile( 64, ".bc" ) );
    clang::CompilerInstance compiler;
    setupCompiler( compiler, src, bc->getPath() );
    clang::EmitBCAction action( getContext().get() );
    execute ( compiler, action );
  }
  module dynamic_compiler::operator()( const std::string &source_code, Language lang ) const {
    TemporaryFile source_file_name( 64, getFileSuffix( lang ) );
    TemporaryFile bc_file_name( 64, ".bc" );
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, source_file_name.getPath(), bc_file_name.getPath() );
    compiler.createFileManager();
    const clang::FileEntry *ram_file = compiler.getFileManager().getVirtualFile ( "/foo", source_code.size(), 10 );
    std::cout << ram_file << std::endl;
    std::string error;
    std::cout << compiler.getFileManager().getFile(
                                                            "/foo"
                                                            ) << std::endl;
    std::cout << error << std::endl;
      //->getBuffer() = llvm::StringRef( source_code );
    {
      std::fstream source_file( source_file_name.getPath().c_str(), std::ios::out );
      source_file << source_code;
    }

    return getModule( compiler, bc_file_name );
  }
  object dynamic_compiler::getObject( const std::string &source_code, Language lang ) const {
    TemporaryFile source_file_name( 64, getFileSuffix( lang ) );
    TemporaryFile bc_file_name( 64, ".bc" );
    {
      std::fstream source_file( source_file_name.getPath().c_str(), std::ios::out );
      source_file << source_code;
    }
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, source_file_name.getPath(), bc_file_name.getPath() );
    
    return getObject( compiler, bc_file_name );
  }
  std::string dynamic_compiler::dumpLLVM( const std::string &source_code, Language lang ) const {
    TemporaryFile source_file_name( 64, getFileSuffix( lang ) );
    TemporaryFile ast_file_name( 64, ".ll" );
    {
      std::fstream source_file( source_file_name.getPath().c_str(), std::ios::out );
      source_file << source_code;
    }
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, source_file_name.getPath(), ast_file_name.getPath() );
    
    clang::EmitLLVMAction action( getContext().get() );
    execute ( compiler, action );
    std::string result;
    std::fstream asm_file( ast_file_name.getPath().c_str(), std::ios::in );
    return std::string( std::istreambuf_iterator<char>(asm_file), std::istreambuf_iterator<char>() );
  }
  std::string dynamic_compiler::dumpAsm( const std::string &source_code, Language lang ) const {
    native_target::init();
    TemporaryFile source_file_name( 64, getFileSuffix( lang ) );
    TemporaryFile ast_file_name( 64, ".S" );
    {
      std::fstream source_file( source_file_name.getPath().c_str(), std::ios::out );
      source_file << source_code;
    }
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, source_file_name.getPath(), ast_file_name.getPath() );
    
    clang::EmitAssemblyAction action( getContext().get() );
    execute ( compiler, action );
    std::string result;
    std::fstream asm_file( ast_file_name.getPath().c_str(), std::ios::in );
    return std::string( std::istreambuf_iterator<char>(asm_file), std::istreambuf_iterator<char>() );
  }
  module dynamic_compiler::operator()( const boost::filesystem::path &path ) const {
    TemporaryFile bc_file_name( 64, ".bc" );
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, path, bc_file_name.getPath() );
    
    return getModule( compiler, bc_file_name );
  }
  object dynamic_compiler::getObject( const boost::filesystem::path &path ) const {
    TemporaryFile bc_file_name( 64, ".bc" );
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, path, bc_file_name.getPath() );
    
    return getObject( compiler, bc_file_name );
  }
  std::string dynamic_compiler::dumpLLVM( const boost::filesystem::path &path ) const {
    TemporaryFile ast_file_name( 64, ".ll" );
    clang::CompilerInstance compiler;
    setupCompiler( compiler, path, ast_file_name.getPath() );
    
    clang::EmitLLVMAction action( getContext().get() );
    execute ( compiler, action );
    std::string result;
    std::fstream asm_file( ast_file_name.getPath().c_str(), std::ios::in );
    return std::string( std::istreambuf_iterator<char>(asm_file), std::istreambuf_iterator<char>() );
  }
  std::string dynamic_compiler::dumpAsm( const boost::filesystem::path &path ) const {
    native_target::init();
    TemporaryFile ast_file_name( 64, ".S" );
    clang::CompilerInstance compiler; 
    setupCompiler( compiler, path, ast_file_name.getPath() );
    
    clang::EmitAssemblyAction action( getContext().get() );
    execute ( compiler, action );
    std::string result;
    std::fstream asm_file( ast_file_name.getPath().c_str(), std::ios::in );
    return std::string( std::istreambuf_iterator<char>(asm_file), std::istreambuf_iterator<char>() );
  }
  module dynamic_compiler::getModule( clang::CompilerInstance &compiler, TemporaryFile &bc_file_name ) const {
    clang::EmitBCAction action( getContext().get() );
    execute ( compiler, action );
    native_target::init();
    llvm::SMDiagnostic Err;
    llvm::Module *llvm_module = llvm::ParseIRFile( bc_file_name.getPath().c_str(), Err, *getContext() );
    std::string ErrorMsg;
    if ( llvm_module->MaterializeAllPermanently(&ErrorMsg)) {
      llvm::errs() << "dcompile::module" << ": bitcode didn't read correctly.\n";
      llvm::errs() << "Reason: " << ErrorMsg << "\n";
      throw UnableToLoadModule();
    }
    module lib( getContext(), optlevel, llvm_module );
    return lib;
  }
  object dynamic_compiler::getObject( clang::CompilerInstance &compiler, TemporaryFile &bc_file_name ) const {
    clang::EmitBCAction action( getContext().get() );
    execute ( compiler, action );
    native_target::init();
    llvm::SMDiagnostic Err;
    boost::shared_ptr< llvm::Module > llvm_module( llvm::ParseIRFile( bc_file_name.getPath().c_str(), Err, *getContext() ) );
    std::string ErrorMsg;
    if ( !llvm_module || llvm_module->MaterializeAllPermanently(&ErrorMsg)) {
      llvm::errs() << "dcompile::module" << ": bitcode didn't read correctly.\n";
      llvm::errs() << "Reason: " << ErrorMsg << "\n";
      throw UnableToLoadModule();
    }
    object lib( getContext(), optlevel, llvm_module );
    return lib;
  }
}
