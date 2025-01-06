# Project for DXFImportPlugin plugin session and all IBK libraries

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += DXFImportPlugin \
            QtExt \
			DXFTestBed \
			libdxfrw \
			IBK \
			IBKMK \
			glm \
			TiCPP

DXFTestBed.file = ../../DXFTestBed/projects/Qt/DXFTestBed.pro
DXFImportPlugin.file = ../../externals/DXFImportPlugin/projects/Qt/DXFImportPlugin.pro

IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
QtExt.file = ../../externals/QtExt/projects/Qt/QtExt.pro
libdxfrw.file = ../../externals/libdxfrw/projects/Qt/libdxfrw.pro
glm.file = ../../externals/glm/glm.pro

IBKMK.depends = IBK
QtExt.depends = IBK
DXFImportPlugin.depends = IBK IBKMK TiCPP QtExt libdxfrw glm
DXFTestBed.depends = IBK IBKMK TiCPP QtExt libdxfrw DXFImportPlugin
