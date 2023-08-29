# Project file for DXFImportPlugin
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = DXFImportPlugin

QT += gui
greaterThan(QT_MAJOR_VERSION, 4):  QT += widgets

TEMPLATE = lib
CONFIG += plugin
CONFIG += shared

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
	../../../../externals/IBK/src \
	../../../../externals/IBKMK/src \
	../../../../externals/TiCPP/src \
	../../../../externals/QtExt/src 

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
	../../src/DXFImportPlugin.cpp 

HEADERS += \
	../../src/SVCommonPluginInterface.h \
	../../src/SVImportPluginInterface.h \
	../../src/DXFImportPlugin.h \

QMAKE_LIBDIR += ../../../../externals/lib$${DIR_PREFIX}

LIBS += -L../../../../externals/lib$${DIR_PREFIX}

LIBS += \
        -lQtExt \
	-lTiCPP \
	-lIBKMK \
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