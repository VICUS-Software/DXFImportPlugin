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

DXFTestBed.file = DXFTestBed/DXFTestBed.pro
DXFImportPlugin.file = externals/DXFImportPlugin/DXFImportPlugin.pro

IBK.file = externals/IBK/IBK.pro
IBKMK.file = externals/IBKMK/IBKMK.pro
TiCPP.file = externals/TiCPP/TiCPP.pro
QtExt.file = externals/QtExt/QtExt.pro
libdxfrw.file = externals/libdxfrw/libdxfrw.pro
glm.file = externals/glm/glm.pro

IBKMK.depends = IBK
QtExt.depends = IBK
DXFImportPlugin.depends = IBK IBKMK TiCPP QtExt libdxfrw glm
DXFTestBed.depends = IBK IBKMK TiCPP QtExt libdxfrw DXFImportPlugin
libdxfrw.depends = IBK
