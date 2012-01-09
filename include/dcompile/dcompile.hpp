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

#ifndef DCOMPILE_DCOMPILE_HPP
#define DCOMPILE_DCOMPILE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/preprocessor.hpp>
#include <boost/unordered_set.hpp>

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

#include <dcompile/mktemp.hpp>

namespace dcompile {
  class UnableToLoadModule {};
  class UnknownOptimizeLevel {};
  class InvalidArgument {};

  enum OptimizeLevel {
    None,
    Less,
    Default,
    Aggressive
  };

  class context_holder {
  public:
    context_holder() : llvm_context( new llvm::LLVMContext ) {
    }
    context_holder( const boost::shared_ptr< llvm::LLVMContext > &context ) : llvm_context( context ) {
    }
  protected:
    const boost::shared_ptr< llvm::LLVMContext > &getContext() const {
      return llvm_context;
    }
  private:
    boost::shared_ptr< llvm::LLVMContext > llvm_context;
  };
  
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   boost::is_pointer< Type >::value
                 > >::type* = 0 ) {
    _dest.PointerVal = value;
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   !boost::is_pointer< Type >::value &&
                   boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, float >::value
                 > >::type* = 0 ) {
    _dest.FloatVal = value;
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   !boost::is_pointer< Type >::value &&
                   boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, double >::value
                 > >::type* = 0 ) {
    _dest.DoubleVal = value;
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   !boost::is_pointer< Type >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, float >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, double >::value
                 > >::type* = 0 ) {
    if( sizeof( Type ) <= 8 )
      *static_cast< Type* >( static_cast< void* >( _dest.Untyped ) ) = value;
    else
      throw InvalidArgument();
  }

  class function : public context_holder {
  public:
    function(
      const boost::shared_ptr< llvm::LLVMContext > &_llvm_context,
      const boost::shared_ptr< llvm::EngineBuilder > &_builder,
      const boost::shared_ptr< llvm::ExecutionEngine > &_engine,
      llvm::Module *_module,
      llvm::Function *_function
    );
    void operator()();
#define DCOMPILE_FUNCTION_CALL_TEMPLATE_ARGS_EACH( z, index, args ) \
  typename BOOST_PP_CAT( Arg, index )

#define DCOMPILE_FUNCTION_CALL_ARGS_EACH( z, index, args ) \
  BOOST_PP_CAT( Arg, index ) BOOST_PP_CAT( arg, index )
  
#define DCOMPILE_FUNCTION_CALL_SETPARAM_EACH( z, index, args ) \
  setParam( run_args[ index ], BOOST_PP_CAT( arg, index ) );

#define DCOMPILE_FUNCTION_CALL_EACH( z, index, args ) \
  template< BOOST_PP_ENUM( index, DCOMPILE_FUNCTION_CALL_TEMPLATE_ARGS_EACH, args ) > \
  void operator()( BOOST_PP_ENUM( index, DCOMPILE_FUNCTION_CALL_ARGS_EACH, args ) ) { \
    std::vector< llvm::GenericValue > run_args( index ); \
      if( entry_point->getFunctionType()->getNumParams() != index || \
        ( entry_point->getFunctionType()->isVarArg() && entry_point->getFunctionType()->getNumParams() > index ) ) \
        throw InvalidArgument(); \
    BOOST_PP_REPEAT( index, DCOMPILE_FUNCTION_CALL_SETPARAM_EACH, args ) \
    llvm::GenericValue Result = engine->runFunction( entry_point, run_args ); \
  }
  
    BOOST_PP_REPEAT_FROM_TO( 1, 20, DCOMPILE_FUNCTION_CALL_EACH, )

  private:
    boost::shared_ptr< llvm::EngineBuilder > builder;
    llvm::Module *llvm_module;
    boost::shared_ptr< llvm::ExecutionEngine > engine;
    llvm::Function *entry_point;
  };
  class module : public context_holder {
  public:
    module(
      const boost::shared_ptr< llvm::LLVMContext > &context,
      OptimizeLevel optlevel,
      const boost::shared_ptr< dcompile::TemporaryFile > &file
    );
    int operator()( const std::vector< std::string > &argv, char * const *envp );
    boost::optional< function > getFunction( const std::string &name );
  private:
    static void deleteBuilder( llvm::EngineBuilder *builder, boost::shared_ptr< llvm::ExecutionEngine > engine );
    boost::shared_ptr< dcompile::TemporaryFile > bc_file;
    boost::shared_ptr< llvm::EngineBuilder > builder;
    llvm::Module *llvm_module;
    boost::shared_ptr< llvm::ExecutionEngine > engine;
  };

  class loader : public context_holder {
  public:
    loader( const boost::shared_ptr< llvm::LLVMContext > &_context );
    bool load( const std::string &name ) const;
    boost::optional< boost::filesystem::path > findLib( const std::string &name ) const;
    boost::optional< boost::filesystem::path > findLibInDirectory( const std::string &name, const boost::filesystem::path &path ) const;
    void enableSystemPath( bool flag = true );
    void disableSystemPath( bool flag = true ) {
      enableSystemPath( !flag );
    }
    void addPath( const boost::filesystem::path path ) {
      user_path.insert( path );
    }
    void delPath( const boost::filesystem::path path ) {
      user_path.erase( path );
    }
    boost::unordered_set< boost::filesystem::path > &getPath() {
      return user_path;
    }
    const boost::unordered_set< boost::filesystem::path > &getPath() const {
      return user_path;
    }
  private:
    boost::unordered_set< boost::filesystem::path > user_path;
    std::vector< boost::filesystem::path > system_path;
    bool enable_system_path;
  };
  class header_path {
  public:
    header_path() : enable_system_path( false ) {
    }
    void enableSystemPath( bool flag = true ) {
      enable_system_path = flag;
    }
    void disableSystemPath( bool flag = true ) {
      enableSystemPath( !flag );
    }
    bool includeSystemPath() const {
      return enable_system_path;
    }
    void addPath( const boost::filesystem::path path ) {
      user_path.insert( path );
    }
    void delPath( const boost::filesystem::path path ) {
      user_path.erase( path );
    }
    boost::unordered_set< boost::filesystem::path > &getPath() {
      return user_path;
    }
    const boost::unordered_set< boost::filesystem::path > &getPath() const {
      return user_path;
    }
  private:
    boost::unordered_set< boost::filesystem::path > user_path;
    bool enable_system_path;
  };
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
    module operator()( const std::string &source_code );
  private:
    header_path header;
    loader library_loader;
    OptimizeLevel optlevel;
    boost::shared_ptr< llvm::LLVMContext > llvm_context;
    clang::CompilerInstance compiler;
    const boost::filesystem::path resource_directory;
  };
}

#endif
