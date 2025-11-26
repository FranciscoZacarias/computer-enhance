@echo off
setlocal

set outer_dir=%~dp0
set cl_default_flags=/nologo /FC /Zi /DEBUG
if "%1"=="" (echo Provide a target. Example: build p11)
set target=%1

REM Targets
set dir_name=
set src_file=
set out_name=
if /I "%target%"=="p1_1" (
  set dir_name=P1_1_ReadingASM
  set src_file=p1_1.c
  set out_name=p1_1.exe
)
if /I "%target%"=="p1_2" (
  set dir_name=P1_2_DecodingMultipleInstructions
  set src_file=p1_2.c
  set out_name=p1_2.exe
)
if "%src_file%"=="" (echo Unknown target "%target%")

REM Files
set src_dir=%outer_dir%%dir_name%\
set build_dir=%src_dir%build\

if not exist "%build_dir%" mkdir "%build_dir%"
pushd "%build_dir%"
cl "..\%src_file%" %cl_default_flags% /Fe"%out_name%"
popd
