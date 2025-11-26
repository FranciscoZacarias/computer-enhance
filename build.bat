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
if /I "%target%"=="p11" (
  set dir_name=P1_ReadingASM
  set src_file=p11.c
  set out_name=p11.exe
)
if "%src_file%"=="" (echo Unknown target "%target%")

REM Files
set src_dir=%outer_dir%%dir_name%\
set build_dir=%src_dir%build\

if not exist "%build_dir%" mkdir "%build_dir%"
pushd "%build_dir%"
cl "..\%src_file%" %cl_default_flags% /Fe"%out_name%"
popd
