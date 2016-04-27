@echo off

set TASM_PATH=C:\TASM
set COMPILER=%TASM_PATH%\TASM\TASM.EXE
set LINKER=%TASM_PATH%\TASM\TLINK.EXE
set DEBUGGER=%TASM_PATH%\TD.EXE

set TARGET=copyfile
set EXT=exe
rem list of source files:
set SRC=%TARGET%.asm
set OBJ=%TARGET%.obj

if "%1"=="clean" goto clean
if "%1"=="debug" goto debug

rem if no clean or debug command selected then just compile binary in release
%COMPILER% %SRC%
%LINKER% %OBJ%
goto end

:debug
rem make sure we have compiled binary in debug mode

%COMPILER% /z /zi %SRC%
%LINKER% /v %OBJ%

%DEBUGGER% %TARGET%.%EXT%
goto end

:clean
del %TARGET%.%EXT%
del *.MAP
del %OBJ%
:end
