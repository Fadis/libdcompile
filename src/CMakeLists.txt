add_definitions( -DRESOURCE_DIRECTORY=${LLVM_LIB_DIR}/clang/ )
add_definitions( -DSAMPLE_SOURCE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}/resources )

ADD_LIBRARY( dcompile SHARED common.cpp dcompile.cpp mktemp.cpp function.cpp module.cpp loader.cpp dynamic_compiler.cpp object.cpp )
target_link_libraries( dcompile ${CLANG_LIBS} ${Boost_LIBRARIES} ${LLVM_LIBS} )
set_target_properties( dcompile  PROPERTIES
	  VERSION 0.0.1
	  SOVERSION 0.0.1
	)
ADD_EXECUTABLE( sample1 sample1.cpp )
target_link_libraries( sample1 dcompile )
ADD_EXECUTABLE( sample2 sample2.cpp )
target_link_libraries( sample2 dcompile )
ADD_EXECUTABLE( sample3 sample3.cpp )
target_link_libraries( sample3 dcompile )
ADD_EXECUTABLE( sample4 sample4.cpp )
target_link_libraries( sample4 dcompile )
ADD_EXECUTABLE( sample5 sample5.cpp )
target_link_libraries( sample5 dcompile )
install( TARGETS dcompile
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
