add_definitions( -DRESOURCE_DIRECTORY=${LLVM_LIB_DIR}/clang/ )

ADD_LIBRARY( dcompile SHARED dcompile.cpp mktemp.cpp function.cpp module.cpp loader.cpp dynamic_compiler.cpp )
target_link_libraries( dcompile ${CLANG_LIBS} ${Boost_LIBRARIES} ${LLVM_LIBS} )
set_target_properties( dcompile  PROPERTIES
	  VERSION 0.0.1
	  SOVERSION 0.0.1
	)
ADD_EXECUTABLE( dcompile_sample main.cpp )
target_link_libraries( dcompile_sample dcompile )
install( TARGETS dcompile dcompile_sample
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
