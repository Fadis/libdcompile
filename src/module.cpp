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

#include <dcompile/common.hpp>
#include <dcompile/context_holder.hpp>
#include <dcompile/function.hpp>
#include <dcompile/mktemp.hpp>
#include <dcompile/module.hpp>
#include <dcompile/native_target.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>

#include <llvm/Support/raw_ostream.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/SourceMgr.h>

namespace dcompile {
  module::module(
    const boost::shared_ptr< llvm::LLVMContext > &context,
    OptimizeLevel optlevel,
    llvm::Module *_module
  ) : context_holder( context ), llvm_module( _module ) {
    native_target::init();
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
}
