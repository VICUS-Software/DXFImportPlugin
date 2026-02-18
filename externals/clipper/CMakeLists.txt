# CMakeLists.txt file for Clipper library

project( clipper )

# collect a list of all source files of the library
file (GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp )

# build the library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
)
