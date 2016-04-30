@echo off

rem compiler settings
set TASM_PATH=C:\TASM
set COMPILER=%TASM_PATH%\TASM\TASM.EXE
set LINKER=%TASM_PATH%\TASM\TLINK.EXE
set DEBUGGER=%TASM_PATH%\TD.EXE

rem target settings
set TARGET_DIR=build
set TARGET=test
set EXT=exe

rem list of source files
set SRC=%TARGET%.asm
set OBJ=%TARGET_DIR%\%TARGET%.obj

if "%1"=="clean" goto clean
if "%1"=="debug" goto debug

rem no need to clog source directory
if not exist %TARGET_DIR% mkdir %TARGET_DIR%

rem FIXME: sometimes compiler try make obj-file in wrong path
rem        but if clean target folder before make, all is ok

rem if no clean or debug command selected then just compile binary in release
%COMPILER% %SRC% %OBJ%
%LINKER% %OBJ%
goto end

:debug
rem make sure we have compiled binary in debug mode

%COMPILER% /z /zi %SRC% %OBJ%
%LINKER% /v %OBJ%

rem run target in debugger
%DEBUGGER% %TARGET_DIR%\%TARGET%.%EXT%
goto end

:clean
echo removing %TARGET_DIR%\%TARGET%.%EXT% ...
del %TARGET_DIR%\%TARGET%.%EXT%

echo removing all map files ...
del %TARGET_DIR%\*.MAP

echo removing %OBJ% ...
del %OBJ%
:end
