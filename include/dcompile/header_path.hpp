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

#ifndef DCOMPILE_HEADER_PATH_HPP
#define DCOMPILE_HEADER_PATH_HPP

#include <string>

#include <boost/unordered_set.hpp>

namespace dcompile {
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
    bool enable_system_path;
  };
}

#endif
