# CMakeLists.txt file for DXFImportPlugin library

project( DXFImportPlugin )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/QtExt/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../externals/libdxfrw/src
	${PROJECT_SOURCE_DIR}/../../../externals/libdxfrw/src/intern
)

# collect a list of all source files of the IBK library
file( GLOB LIB_SRCS 
  ${PROJECT_SOURCE_DIR}/../../src/*.cpp 
)
    
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj ")
endif()

add_definitions(-DIFCQUERY_AS_DYNAMIC_LIBRARY)
add_definitions(-DIFCQUERY_LIB)

# build the library
add_library( DXFImportPlugin SHARED
	${LIB_SRCS}
)

