# Detect LLVM and set various variable to link against the different component of LLVM
#
# NOTE: This is a modified version of the module originally found in the OpenGTL project
# at www.opengtl.org
#
# LLVM_BIN_DIR : directory with LLVM binaries
# LLVM_LIB_DIR : directory with LLVM library
# LLVM_INCLUDE_DIR : directory with LLVM include
#
# LLVM_COMPILE_FLAGS : compile flags needed to build a program using LLVM headers
# LLVM_LDFLAGS : ldflags needed to link
# LLVM_LIBS : ldflags needed to link against a LLVM library

if (LLVM_INCLUDE_DIR)
  set(LLVM_FOUND TRUE)
else (LLVM_INCLUDE_DIR)

find_program(LLVM_CONFIG_EXECUTABLE
  NAMES llvm-config
  PATHS
  /opt/local/bin
)

MACRO(FIND_LLVM_LIBS LLVM_CONFIG_EXECUTABLE _libname_ LIB_VAR OBJECT_VAR)
  exec_program( ${LLVM_CONFIG_EXECUTABLE} ARGS --libs ${_libname_}  OUTPUT_VARIABLE ${LIB_VAR} )
  STRING(REGEX MATCHALL "[^ ]*[.]o[ $]"  ${OBJECT_VAR} ${${LIB_VAR}})
  SEPARATE_ARGUMENTS(${OBJECT_VAR})
  STRING(REGEX REPLACE "[^ ]*[.]o[ $]" ""  ${LIB_VAR} ${${LIB_VAR}})
ENDMACRO(FIND_LLVM_LIBS)


exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --bindir OUTPUT_VARIABLE LLVM_BIN_DIR )
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --libdir OUTPUT_VARIABLE LLVM_LIB_DIR )
#MESSAGE(STATUS "LLVM lib dir: " ${LLVM_LIB_DIR})
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR )


exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --cppflags  OUTPUT_VARIABLE LLVM_COMPILE_FLAGS )
MESSAGE(STATUS "LLVM CXX flags: " ${LLVM_COMPILE_FLAGS})
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --ldflags   OUTPUT_VARIABLE LLVM_LDFLAGS )
MESSAGE(STATUS "LLVM LD flags: " ${LLVM_LDFLAGS})
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --libs asmparser asmprinter bitreader bitwriter executionengine jit mcjit native nativecodegen codegen ipo core linker instrumentation OUTPUT_VARIABLE LLVM_LIBS )
MESSAGE(STATUS "LLVM libs: " ${LLVM_LIBS})

if(LLVM_INCLUDE_DIR)
  set(LLVM_FOUND TRUE)
endif(LLVM_INCLUDE_DIR)

if(LLVM_FOUND)
  message(STATUS "Found LLVM: ${LLVM_INCLUDE_DIR}")
else(LLVM_FOUND)
  if(LLVM_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find LLVM")
  endif(LLVM_FIND_REQUIRED)
endif(LLVM_FOUND)

endif (LLVM_INCLUDE_DIR)
