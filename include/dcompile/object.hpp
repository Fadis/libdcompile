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

#ifndef DCOMPILE_OBJECT_HPP
#define DCOMPILE_OBJECT_HPP

#include <string>
#include <vector>

#include <dcompile/common.hpp>
#include <dcompile/context_holder.hpp>
#include <dcompile/function.hpp>
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
  class object : public context_holder {
  public:
    object(
      const boost::shared_ptr< llvm::LLVMContext > &context,
      OptimizeLevel optlevel,
      boost::shared_ptr< llvm::Module > _module
    );
    boost::shared_ptr< llvm::Module > get() const {
      return boost::shared_ptr< llvm::Module >( llvm::CloneModule( llvm_module.get() ) );
    }
    OptimizeLevel getOptimizeLevel() const {
      return optlevel;
    }
  private:
    boost::shared_ptr< llvm::Module > llvm_module;
    OptimizeLevel optlevel;
  };
}

#endif
