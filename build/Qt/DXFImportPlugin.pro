# Project for DXFImportPlugin plugin session and all IBK libraries

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += DXFImportPlugin \
QtExt \
DXFTestBed \
			libdxfrw \
			IBK \
			IBKMK \
			TiCPP

DXFTestBed.file = ../../DXFTestBed/projects/Qt/DXFTestBed.pro
DXFImportPlugin.file = ../../externals/DXFImportPlugin/projects/Qt/DXFImportPlugin.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
QtExt.file = ../../externals/QtExt/projects/Qt/QtExt.pro
libdxfrw.file = ../../externals/libdxfrw/projects/Qt/libdxfrw.pro

DXFTestBed.depends = DXFImportPlugin IBK IBKMK TiCPP QtExt libdxfrw
QtExt.depends = IBK
DXFImportPlugin.depends = IBK IBKMK TiCPP QtExt libdxfrw
IBKMK.depends = IBK
