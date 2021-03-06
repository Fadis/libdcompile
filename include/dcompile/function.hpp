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

#ifndef DCOMPILE_FUNCTION_HPP
#define DCOMPILE_FUNCTION_HPP

#include <dcompile/exceptions.hpp>
#include <dcompile/context_holder.hpp>

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/preprocessor/repeat_from_to.hpp>
#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/variant.hpp>

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace dcompile {
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   boost::is_pointer< Type >::value &&
                   !boost::is_const< typename boost::remove_pointer< Type >::type >::value
                 > >::type* = 0 ) {
    _dest.PointerVal = static_cast< void* >( value );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   boost::is_pointer< Type >::value &&
                   boost::is_const< typename boost::remove_pointer< Type >::type >::value
                 > >::type* = 0 ) {
    _dest.PointerVal = static_cast< void* >( const_cast< typename boost::remove_const< typename boost::remove_pointer< Type >::type >::type* >( value ) );
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
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed char >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( signed char ) * 8, static_cast< uint64_t >( value ), true );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned char >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( unsigned char ) * 8, static_cast< uint64_t >( value ), false );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed short >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( signed short ) * 8, static_cast< uint64_t >( value ), true );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned short >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( unsigned short ) * 8, static_cast< uint64_t >( value ), false );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed int >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( signed int ) * 8, static_cast< uint64_t >( value ), true );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned int >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( unsigned int ) * 8, static_cast< uint64_t >( value ), false );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed long >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( signed long ) * 8, static_cast< uint64_t >( value ), true );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned long >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( unsigned long ) * 8, static_cast< uint64_t >( value ), false );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed long long >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( signed long long ) * 8, static_cast< uint64_t >( value ), true );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                typename boost::enable_if< boost::mpl::bool_<
                boost::is_pod< Type >::value &&
                !boost::is_pointer< Type >::value &&
                boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned long long >::value
                > >::type* = 0 ) {
    _dest.IntVal = llvm::APInt( sizeof( unsigned long long ) * 8, static_cast< uint64_t >( value ), false );
  }
  template< typename Type >
  void setParam( llvm::GenericValue &_dest, Type value,
                 typename boost::enable_if< boost::mpl::bool_<
                   boost::is_pod< Type >::value &&
                   !boost::is_pointer< Type >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, float >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, double >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed char >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned char >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed short >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned short >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed int >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned int >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, signed long >::value &&
                   !boost::is_same< typename boost::remove_cv< typename boost::remove_reference< Type >::type >::type, unsigned long >::value
                 > >::type* = 0 ) {
    if( sizeof( Type ) <= 8 )
      *static_cast< Type* >( static_cast< void* >( _dest.Untyped ) ) = value;
    else
      throw InvalidArgument();
  }
  enum {
    SIGNED_CHAR = 0,
    UNSIGNED_CHAR,
    SIGNED_SHORT,
    UNSIGNED_SHORT,
    SIGNED_INT,
    UNSIGNED_INT,
    SIGNED_LONG,
    UNSIGNED_LONG,
    SIGNED_LONG_LONG,
    UNSIGNED_LONG_LONG,
    SIGNED_FLOAT,
    UNSIGNED_DOUBLE,
    UNSIGNED_POINTER
  };
  typedef boost::variant<
    signed char,
    unsigned char,
    signed short,
    unsigned short,
    signed int,
    unsigned int,
    signed long,
    unsigned long,
    signed long long,
    unsigned long long,
    float,
    double,
    void*
  > return_value;
  return_value getReturnValue( const llvm::Type &type, const llvm::GenericValue &_value );
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
    boost::shared_ptr< llvm::ExecutionEngine > engine;
    llvm::Module *llvm_module;
    llvm::Function *entry_point;
  };
}

#endif
