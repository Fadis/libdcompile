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

#ifndef DCOMPILE_LINKER_HPP
#define DCOMPILE_LINKER_HPP

#include <string>
#include <vector>

#include <dcompile/common.hpp>
#include <dcompile/object.hpp>
#include <dcompile/module.hpp>
#include <dcompile/mktemp.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/utility/enable_if.hpp>

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/DerivedTypes.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Transforms/Utils/Cloning.h>

namespace dcompile {
  template< typename Iterator >
  object link(
    Iterator begin, Iterator end,
    typename boost::enable_if< boost::is_same< typename boost::remove_cv< typename boost::iterator_value< Iterator >::type >::type, object > >::type* = 0
  ) {
    if( std::distance( begin, end ) == 0 )
      throw LinkFailed();
    llvm::Linker linker( "linker", "composite", *begin->getContext().get(), 0 );
    for( Iterator iter = begin; iter != end; ++iter ) {
      if( linker.LinkInModule( llvm::CloneModule( iter->get().get() ) ) )
        throw LinkFailed();
    }
    boost::shared_ptr< llvm::Module > llvm_module( linker.releaseModule() );
    return object( begin->getContext(), begin->getOptimizeLevel(), llvm_module );
  }
  module load( const object &_obj ) {
    llvm::Module *llvm_module( llvm::CloneModule( _obj.get().get() ) );
    return module( _obj.getContext(), _obj.getOptimizeLevel(), llvm_module );
  }
}

#endif
