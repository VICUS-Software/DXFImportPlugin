TEMPLATE = lib

TARGET = libdxfrw

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

LIBS += -lIBK


INCLUDEPATH = \
../../../IBK/src

INCLUDEPATH += \
../../../IBK/src

HEADERS += \
../../src/drw_classes.h \
../../src/drw_entities.h \
../../src/drw_header.h \
	../../src/drw_interface.h \
	../../src/drw_objects.h \
	../../src/intern/drw_cptable932.h \
	../../src/intern/drw_cptable936.h \
	../../src/intern/drw_cptable949.h \
	../../src/intern/drw_cptable950.h \
	../../src/intern/drw_cptables.h \
	../../src/intern/drw_dbg.h \
	../../src/intern/drw_textcodec.h \
	../../src/intern/dwgbuffer.h \
	../../src/intern/dwgreader.h \
	../../src/intern/dwgreader15.h \
	../../src/intern/dwgreader18.h \
	../../src/intern/dwgreader21.h \
	../../src/intern/dwgreader24.h \
	../../src/intern/dwgreader27.h \
	../../src/intern/dwgutil.h \
	../../src/intern/dxfreader.h \
	../../src/intern/dxfwriter.h \
	../../src/intern/rscodec.h \
	../../src/libdwgr.h \
	../../src/libdxfrw.h \
	../../src/drw_base.h

SOURCES += \
../../src/drw_classes.cpp \
../../src/drw_entities.cpp \
../../src/drw_header.cpp \
	../../src/drw_objects.cpp \
	../../src/intern/drw_dbg.cpp \
	../../src/intern/drw_textcodec.cpp \
	../../src/intern/dwgbuffer.cpp \
	../../src/intern/dwgreader.cpp \
	../../src/intern/dwgreader15.cpp \
	../../src/intern/dwgreader18.cpp \
	../../src/intern/dwgreader21.cpp \
	../../src/intern/dwgreader24.cpp \
	../../src/intern/dwgreader27.cpp \
	../../src/intern/dwgutil.cpp \
	../../src/intern/dxfreader.cpp \
	../../src/intern/dxfwriter.cpp \
	../../src/intern/rscodec.cpp \
	../../src/libdwgr.cpp \
	../../src/libdxfrw.cpp

QMAKE_CXXFLAGS += -Wall

