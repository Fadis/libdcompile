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
#include <string>
#include <stdint.h>

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

class node {
public:
  node() {
  }
  virtual unsigned int getMaxPlaceholder() const = 0;
  virtual std::string getInitializeCode() const = 0;
  virtual std::string getCode() const = 0;
private:
};

template< typename Type, unsigned int size >
class constant : public node {
public:
  constant( const boost::array< Type, size > &_values ) : values( _values ) {
  }
  unsigned int getMaxPlaceholder() const {
    return 0;
  }
  std::string getInitializeCode() const {
    std::string code( "static const st c" );
    code += boost::lexical_cast< std::string >( this ) + "={";
    for( typename boost::array< Type, size >::const_iterator iter = values.begin(); iter != values.end(); ++iter )
      code += boost::lexical_cast< std::string >( *iter ) + ",";
    code += "};";
    return code;
  }
  std::string getCode() const {
    std::string code( "c" );
    code += boost::lexical_cast< std::string >( this );
    return code;
  }
private:
  const boost::array< Type, size > values;
};

class placeholder : public node {
public:
  placeholder( unsigned int _pos ) : pos( _pos ) {
  }
  unsigned int getMaxPlaceholder() const {
    return pos;
  }
  std::string getInitializeCode() const {
    std::string code( "const st a" );
    code += boost::lexical_cast< std::string >( pos ) + "=pack(arg" + boost::lexical_cast< std::string >( pos ) + ");";
    return code;
  }
  std::string getCode() const {
    std::string code( "a" );
    code += boost::lexical_cast< std::string >( pos );
    return code;
  }
private:
  const unsigned int pos;
};

class oper_1l : public node {
public:
  oper_1l( const std::string &_oper, const boost::shared_ptr< node > &_arg0 ) : arg0( _arg0 ), oper( _oper ) {
  }
  unsigned int getMaxPlaceholder() const {
    return arg0->getMaxPlaceholder();
  }
  std::string getInitializeCode() const {
    return arg0->getInitializeCode();
  }
  std::string getCode() const {
    std::string code( "(" );
    code += oper + arg0->getCode() + ")";
    return code;
  }
private:
  boost::shared_ptr< node > arg0;
  const std::string oper;
};

class oper_1r : public node {
public:
  oper_1r( const std::string &_oper, const boost::shared_ptr< node > &_arg0 ) : arg0( _arg0 ), oper( _oper ) {
  }
  unsigned int getMaxPlaceholder() const {
    return arg0->getMaxPlaceholder();
  }
  std::string getInitializeCode() const {
    return arg0->getInitializeCode();
  }
  std::string getCode() const {
    std::string code( "(" );
    code += arg0->getCode() + oper + ")";
    return code;
  }
private:
  boost::shared_ptr< node > arg0;
  const std::string oper;
};

class oper_2 : public node {
public:
  oper2( const std::string &_oper, const boost::shared_ptr< node > &_arg0, const boost::shared_ptr< node > &_arg1 ) : arg0( _arg0 ), arg1( _arg1 ), oper( _oper ) {
  }
  unsigned int getMaxPlaceholder() const {
    return std::max( arg0->getMaxPlaceholder(), arg1->getMaxPlaceholder() );
  }
  std::string getInitializeCode() const {
    return arg0->getInitializeCode() + arg1->getInitializeCode();
  }
  std::string getCode() const {
    std::string code( "(" );
    code += arg0->getCode() + "+" + arg1->getCode() + ")";
    return code;
  }
private:
  boost::shared_ptr< node > arg0;
  boost::shared_ptr< node > arg1;
  const std::string oper;
};

class left_increment : public oper_1l {
public:
  left_increment( const boost::shared_ptr< node > &_arg0 ) : oper_1l( "++", _arg0 ) {}
};

class right_increment : public oper_1r {
public:
  left_increment( const boost::shared_ptr< node > &_arg0 ) : oper_1r( "++", _arg0 ) {}
};


class sub : public node {
public:
  sub( const boost::shared_ptr< node > &_left, const boost::shared_ptr< node > &_right ) : left( _left ), right( _right ) {
  }
  unsigned int getMaxPlaceholder() const {
    return std::max( left->getMaxPlaceholder(), right->getMaxPlaceholder() );
  }
  std::string getInitializeCode() const {
    return left->getInitializeCode() + right->getInitializeCode();
  }
  std::string getCode() const {
    std::string code( "(" );
    code += left->getCode() + "-" + right->getCode() + ")";
    return code;
  }
private:
  boost::shared_ptr< node > left;
  boost::shared_ptr< node > right;
};

class mul : public node {
public:
  sub( const boost::shared_ptr< node > &_left, const boost::shared_ptr< node > &_right ) : left( _left ), right( _right ) {
  }
  unsigned int getMaxPlaceholder() const {
    return std::max( left->getMaxPlaceholder(), right->getMaxPlaceholder() );
  }
  std::string getInitializeCode() const {
    return left->getInitializeCode() + right->getInitializeCode();
  }
  std::string getCode() const {
    std::string code( "(" );
    code += left->getCode() + "-" + right->getCode() + ")";
    return code;
  }
private:
  boost::shared_ptr< node > left;
  boost::shared_ptr< node > right;
};

template< typename Type, unsigned int size >
class simd {
public:
  simd( const boost::shared_ptr< node > &_node ) : internal_node( _node ) {
  }
  simd( const boost::array< Type, size > &_array ) : internal_node( new constant< Type, size >( _array ) ) {
  }
  simd( unsigned int _pos ) : internal_node( new placeholder( _pos ) ) {
  }
  const boost::shared_ptr< node > &getNode() const {
    return internal_node;
  }
  void operator()() {
    std::string code;
    code += "typedef float* sst;";
    code += "typedef float st __attribute__((ext_vector_type(16)));";
    code += "st pack(sst src){st dest;for(int i=0;i!=16;++i)dest[i]=src[i];return dest;}";
    code += "void unpack(sst dest,st src){for(int i=0;i!=16;++i)dest[i]=src[i];}";
    code += "void calc(sst dest";
    unsigned int max_placeholder = internal_node->getMaxPlaceholder() + 1;
    for( unsigned int index = 0; index != max_placeholder; ++index )
      code += std::string( ",const sst arg" ) + boost::lexical_cast< std::string >( index );
    code += "){";
    code += internal_node->getInitializeCode();
    code += "unpack(dest,";
    code += internal_node->getCode();
    code += ");}";
    std::cout << code << std::endl;
  }
private:
  boost::shared_ptr< node > internal_node;
};

template< typename Type, unsigned int size >
simd< Type, size > operator+( const simd< Type, size > &_left, const simd< Type, size > &_right ) {
  boost::shared_ptr< node > temp( new add( _left.getNode(), _right.getNode() ) );
  return simd< Type, size >( temp );
}

template< typename Type, unsigned int size >
simd< Type, size > operator-( const simd< Type, size > &_left, const simd< Type, size > &_right ) {
  boost::shared_ptr< node > temp( new sub( _left.getNode(), _right.getNode() ) );
  return simd< Type, size >( temp );
}

int main() {
  boost::array< float, 16 > a = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
  typedef simd< float, 16 > s;
  s f = s( a ) + s( 1 ) - s( 2 );
  f();
}

