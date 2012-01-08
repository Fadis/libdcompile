add_definitions( -DRESOURCE_DIRECTORY=${LLVM_LIB_DIR}/clang/ )

ADD_LIBRARY( dcompile SHARED dcompile.cpp mktemp.cpp )
target_link_libraries( dcompile ${CLANG_LIBS} ${Boost_LIBRARIES} ${LLVM_LIBS} )
ADD_EXECUTABLE( sample main.cpp )
target_link_libraries( sample dcompile )
