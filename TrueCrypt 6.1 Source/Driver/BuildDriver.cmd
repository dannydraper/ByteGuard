::
:: Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.
::
:: Governed by the TrueCrypt License 2.6 the full text of which is contained
:: in the file License.txt included in TrueCrypt binary and source code
:: distribution packages.
::

:: Usage: BuildDriver <-build|-rebuild|-clean> <-release|-debug> <-x86|-x64> <dir1> [dir2] ...

@echo off
set TC_ARG_CMD=%~1
shift
set TC_ARG_TYPE=%~1
shift
set TC_ARG_ARCH=%~1
shift


:: Check for spaces in the current directory path

cd | find " " >NUL:

if %ERRORLEVEL% == 0 (
	echo BuildDriver.cmd: error: MS Build does not support building of projects stored in a path containing spaces. >&2
	exit /B 1
)


:: Build options

set TC_C_DEFINES=-D_WIN32 -DNT4_DRIVER
set TC_C_FLAGS=-nologo -I..
set TC_LIBRARIAN_FLAGS=-nologo
set TC_LINKER_FLAGS=-nologo
set NO_SAFESEH=1


:: Windows DDK root

set TC_WINDDK_BUILD=6001.18001

set TC_WINDDK_ROOT=%SYSTEMDRIVE%\WinDDK\%TC_WINDDK_BUILD%
if exist "%TC_WINDDK_ROOT%\bin\setenv.bat" goto ddk_found

set TC_WINDDK_ROOT=%WINDDK_ROOT%\%TC_WINDDK_BUILD%
if exist "%TC_WINDDK_ROOT%\bin\setenv.bat" goto ddk_found

set TC_WINDDK_ROOT=%WINDDK_ROOT%
if exist "%TC_WINDDK_ROOT%\bin\setenv.bat" goto ddk_found

echo BuildDriver.cmd: error: Windows Driver Development Kit not found in the default directory. Set WINDDK_ROOT environment variable to point to your Windows DDK installation directory. >&2
exit /B 1

:ddk_found


:: CPU architecture

if "%TC_ARG_ARCH%"=="-x64" (
	set TC_BUILD_ARCH=x64 WNET
	set TC_BUILD_ARCH_DIR=amd64
	set TC_ARCH=x64
	set TC_ARCH_SUFFIX=-x64
) else (
	set TC_BUILD_ARCH=WXP
	set TC_BUILD_ARCH_DIR=i386
	set TC_ARCH=x86
	set TC_ARCH_SUFFIX=
)


:: Build type

if "%TC_ARG_TYPE%"=="-debug" (
	set TC_BUILD_TYPE=chk
	set TC_C_DEFINES=%TC_C_DEFINES% -DDEBUG -D_DEBUG
	set TC_BUILD_ALT_DIR=_driver_debug
	set TC_COPY_DIR="..\Debug"
) else (
	set TC_BUILD_TYPE=fre
	set TC_BUILD_ALT_DIR=_driver_release
	set TC_C_FLAGS=%TC_C_FLAGS% -w34189
	set TC_COPY_DIR="..\Release"
)


:: WDK environment

pushd .
call %TC_WINDDK_ROOT%\bin\setenv %TC_WINDDK_ROOT% %TC_BUILD_TYPE% %TC_BUILD_ARCH% || exit /B %errorlevel%
popd


:: Build

if "%TC_ARG_CMD%"=="-rebuild" (set TC_BUILD_OPTS=-c)

pushd .
:build_dirs

	if "%~1"=="" goto done
	cd /D "%~1" || exit /B %errorlevel%

	if "%TC_ARG_CMD%"=="-clean" (
		rd /s /q obj%TC_BUILD_ALT_DIR%\%TC_BUILD_ARCH_DIR% 2>NUL:
		rd /q obj%TC_BUILD_ALT_DIR% 2>NUL:
	) else (

		set USER_C_FLAGS=%TC_C_FLAGS% -FAcs -Fa%~1\obj%TC_BUILD_ALT_DIR%\%TC_BUILD_ARCH_DIR%\
		set C_DEFINES=%TC_C_DEFINES%
		set RCOPTIONS=/I %MFC_INC_PATH%
		set LIBRARIAN_FLAGS=%TC_LIBRARIAN_FLAGS%
		set LINKER_FLAGS=%TC_LINKER_FLAGS%
		set BUILD_ALT_DIR=%TC_BUILD_ALT_DIR%

		build %TC_BUILD_OPTS% -Z -w -nmake /S -nmake /C 2>build_errors.log 1>&2
		
		if errorlevel 1 (
			type build_errors.log
			exit /B 1
		)
		del /q build_errors.log build%BUILD_ALT_DIR%.* 2>NUL:
	)

	shift
	
goto build_dirs
:done
popd


if "%TC_ARG_CMD%"=="-clean" exit /B 0

md "%TC_COPY_DIR%\Setup Files" >NUL: 2>NUL:
copy /B /Y obj%TC_BUILD_ALT_DIR%\%TC_BUILD_ARCH_DIR%\truecrypt.sys "%TC_COPY_DIR%\Setup Files\truecrypt%TC_ARCH_SUFFIX%.sys" >NUL:

if errorlevel 1 (
	echo BuildDriver.cmd: error: Cannot copy target.>&2
	exit /B 1
)

exit /B 0
