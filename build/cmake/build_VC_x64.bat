@echo off

:: Build script for CI (GitHub Actions) and local use.
:: In CI, VC and Qt environment are set up by actions.
:: For local use, set JOM_PATH and CMAKE_PREFIX_PATH, or let the defaults apply.

setlocal

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\Tools\QtCreator\bin\jom
)

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%

set BUILD_DIR=bb_VC_x64
:: create and change into build subdir
mkdir %BUILD_DIR% 2>nul
pushd %BUILD_DIR%

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" .. -DCMAKE_BUILD_TYPE:String="Release"
jom
if ERRORLEVEL 1 GOTO fail

popd

:: copy plugin to bin/release_x64 dir
mkdir ..\..\bin\release_x64 2>nul
xcopy /Y .\%BUILD_DIR%\DXFImportPlugin\DXFImportPlugin.dll ..\..\bin\release_x64\

exit /b 0

:fail
echo ** Build Failed **
exit /b 1
