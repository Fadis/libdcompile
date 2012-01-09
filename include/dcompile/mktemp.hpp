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

#ifndef DCOMPILE_MKTEMP_HPP
#define DCOMPILE_MKTEMP_HPP

#include <dcompile/exceptions.hpp>

#include <string>
#include <fstream>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace dcompile {
  template< typename CharType >
  boost::filesystem::path temp_file_path( int length, const std::basic_string< CharType > &prefix, const std::basic_string< CharType > &postfix ) {
    std::string filename_template;
    for( int index = 0; index < length; ++index ) filename_template += '%';
    boost::filesystem::path temp_file;
    int try_count = 0;
    do {
      if( try_count == 1000 ) throw UnableToCreateUniqueFile();
      ++try_count;
      temp_file = boost::filesystem::temp_directory_path() / (
        prefix +
        boost::filesystem::unique_path( filename_template ).generic_string< std::basic_string< CharType > >() +
        postfix
      );
    } while( boost::filesystem::exists( temp_file ) );
    return temp_file;
  }

  template< typename CharType >
  boost::filesystem::path temp_file_path( int length, const std::basic_string< CharType > &postfix ) {
    return temp_file_path( length, std::basic_string< CharType >(), postfix );
  }

  template< typename CharType >
  boost::filesystem::path temp_file_path( int length, const CharType *prefix, const CharType *postfix ) {
    return temp_file_path( length, std::basic_string< CharType >( prefix ), std::basic_string< CharType >( postfix ) );
  }

  template< typename CharType >
  boost::filesystem::path temp_file_path( int length, const CharType *postfix ) {
    return temp_file_path( length, std::basic_string< CharType >(), std::basic_string< CharType >( postfix ) );
  }

  boost::filesystem::path temp_file_path( int length );


  class TemporaryFile {
    static boost::mutex mutex;
  public:
    TemporaryFile( int length, const std::string &prefix, const std::string &postfix ) {
      boost::mutex::scoped_lock( mutex );
      path = temp_file_path( length, prefix, postfix );
      std::fstream stream( path.c_str() );
      if( stream.bad() )
        throw UnableToCreateUniqueFile();
    }
    ~TemporaryFile() {
      boost::mutex::scoped_lock( mutex );
      boost::filesystem::remove( path );
    }
    TemporaryFile( int length, const std::string &postfix ) {
      boost::mutex::scoped_lock( mutex );
      path = temp_file_path( length, postfix );
      std::fstream stream( path.c_str() );
      if( stream.bad() )
        throw UnableToCreateUniqueFile();
    }
    TemporaryFile( int length, const char *prefix, const char *postfix ) {
      boost::mutex::scoped_lock( mutex );
      path = temp_file_path( length, prefix, postfix );
      std::fstream stream( path.c_str() );
      if( stream.bad() )
        throw UnableToCreateUniqueFile();
    }
    TemporaryFile( int length, const char *postfix ) {
      boost::mutex::scoped_lock( mutex );
      path = temp_file_path( length, postfix );
      std::fstream stream( path.c_str() );
      if( stream.bad() )
        throw UnableToCreateUniqueFile();
    }
    explicit TemporaryFile( int length = 64 ) {
      boost::mutex::scoped_lock( mutex );
      path = temp_file_path( length );
      std::fstream stream( path.c_str() );
      if( stream.bad() )
        throw UnableToCreateUniqueFile();
    }
    const boost::filesystem::path &getPath() {
      return path;
    }
  private:
    TemporaryFile( const TemporaryFile& ) {} 
    void operator=( const TemporaryFile& ) {} 
    boost::filesystem::path path;
  };
}

#endif
