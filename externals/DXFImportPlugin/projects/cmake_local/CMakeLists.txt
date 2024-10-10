# CMakeLists.txt file for DXFImportPlugin library

project( DXFImportPlugin )

# add include directories
include_directories(	
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers

	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../QtExt/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../libdxfrw/src
	${PROJECT_SOURCE_DIR}/../../../libdxfrw/src/intern
	
	${Qt5Widgets_INCLUDE_DIRS}
)

# collect a list of all source files of the DXFImportPlugin library
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files of the DXFImportPlugin library
file( GLOB LIB_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h)

# collect a list of all form files of the DXFImportPlugin library
file( GLOB LIB_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )
    
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj ")
endif()

# run uic and moc compilers
qt5_add_resources( LIB_RC_SRCS ${LIB_QRCS} )
qt5_wrap_ui( LIB_UI_SRCS ${LIB_UIS} )
qt5_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# build the library
add_library( DXFImportPlugin SHARED
	${LIB_SRCS}
	${LIB_HDRS}
	${LIB_MOC_SRCS}
	${LIB_UI_SRCS}
	${LIB_RC_SRCS}
)


