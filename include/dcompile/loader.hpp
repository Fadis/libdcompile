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

#ifndef DCOMPILE_LOADER_HPP
#define DCOMPILE_LOADER_HPP

#include <string>
#include <vector>

#include <dcompile/context_holder.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/unordered_set.hpp>

#include <llvm/LLVMContext.h>

namespace dcompile {
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
    void addPath( const boost::filesystem::path &path ) {
      user_path.insert( path );
    }
    void delPath( const boost::filesystem::path &path ) {
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
}

#endif
