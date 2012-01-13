add_definitions( -DRESOURCE_DIRECTORY=${LLVM_LIB_DIR}/clang/ )

ADD_LIBRARY( dcompile SHARED common.cpp dcompile.cpp mktemp.cpp function.cpp module.cpp loader.cpp dynamic_compiler.cpp )
target_link_libraries( dcompile ${CLANG_LIBS} ${Boost_LIBRARIES} ${LLVM_LIBS} )
set_target_properties( dcompile  PROPERTIES
	  VERSION 0.0.1
	  SOVERSION 0.0.1
	)
ADD_EXECUTABLE( dcompile_sample main.cpp )
target_link_libraries( dcompile_sample dcompile )
ADD_EXECUTABLE( dcompile_sample2 float.cpp )
target_link_libraries( dcompile_sample2 dcompile )
install( TARGETS dcompile
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
