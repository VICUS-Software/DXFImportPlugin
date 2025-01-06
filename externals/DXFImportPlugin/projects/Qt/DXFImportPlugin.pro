# Project file for DXFImportPlugin
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = DXFImportPlugin

QT += gui widgets

TEMPLATE = lib
CONFIG += plugin
CONFIG += static_and_dynamic

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# check if 32 or 64 bit version and set prefix variable for using in output paths
contains(QT_ARCH, i386): {
    DIR_PREFIX =
} else {
    DIR_PREFIX = _x64
}

INCLUDEPATH = \
    ../../src \
	../../../../externals/libdxfrw/src \
	../../../../externals/libdxfrw/src/intern \
	../../../../externals/IBK/src \
	../../../../externals/IBKMK/src \
	../../../../externals/QtExt/src \
	../../../../externals/glm/src \
	../../../../externals/glm/src\glm \
	../../../../externals/glm/src\gtx \
	../../../../externals/TiCPP/src

CONFIG(debug, debug|release) {
    OBJECTS_DIR = debug$${DIR_PREFIX}
	windows {
	    DESTDIR = ../../../lib$${DIR_PREFIX}
		DLLDESTDIR = ../../../../bin/debug$${DIR_PREFIX}
	}
	else {
	    DESTDIR = ../../../../bin/debug$${DIR_PREFIX}
	}
}
else {
    OBJECTS_DIR = release
	windows {
	    DESTDIR = ../../../lib$${DIR_PREFIX}
		DLLDESTDIR = ../../../../bin/release$${DIR_PREFIX}
	}
	else {
	    DESTDIR = ../../../../bin/release$${DIR_PREFIX}
	}
}

!windows {
#    QMAKE_POST_LINK += ../../../lib$${DIR_PREFIX}/libDXFImportPlugin.so ../../../../bin/debug$${DIR_PREFIX}/libDXFImportPlugin.so
}

MOC_DIR = moc
UI_DIR = ui

SOURCES += \
    ../../src/Constants.cpp \
	../../src/DXFImportPlugin.cpp  \
	../../src/Drawing.cpp \
	../../src/DrawingLayer.cpp \
	../../src/ImportDXFDialog.cpp \
	../../src/Object.cpp \
	../../src/Utilities.cpp

HEADERS += \
    ../../src/Constants.h \
	../../src/Drawing.h \
	../../src/DrawingLayer.h \
	../../src/ImportDXFDialog.h \
	../../src/Object.h \
	../../src/RotationMatrix.h \
	../../src/SVCommonPluginInterface.h \
	../../src/SVImportPluginInterface.h \
	../../src/DXFImportPlugin.h \
	../../src/Utilities.h

QMAKE_LIBDIR += ../../../../externals/lib$${DIR_PREFIX}

LIBS += -L../../../../externals/lib$${DIR_PREFIX}

LIBS += \
    -lTiCPP \
	-lIBKMK \
	-llibdxfrw \
	-lQtExt \
	-lglm \
	-lIBK

win32:LIBS += -liphlpapi
win32:LIBS += -lshell32

win32-msvc* {
    QMAKE_CXXFLAGS += /std:c++17
}

# Default rules for deployment.
# unix {
# 	target.path = $$[QT_INSTALL_PLUGINS]/generic
# }
# !isEmpty(target.path): INSTALLS += target


TRANSLATIONS += ../../resources/translations/DXFImportPlugin_de.ts
CODECFORSRC = UTF-8

FORMS += \
    ../../src/ImportDXFDialog.ui

