@echo off
setlocal

if "%~1"=="" (
  echo Usage: %~nx0 ^<target^>
  exit /b 1
)
set "target=%~1"
set "dir_name="
set "exe_base="

if /I "%target%"=="p1_1" (
  set "dir_name=P1_1_ReadingASM"
  set "exe_base=p1_1"
)
if /I "%target%"=="p1_2" (
  set "dir_name=P1_2_DecodingMultipleInstructions"
  set "exe_base=p1_2"
)
if /I "%target%"=="p1_3" (
  set "dir_name=P1_3_OpcodePatternsIn8086Aritmetic"
  set "exe_base=p1_3"
)

if "%dir_name%"=="" (echo Unknown target "%target%")
set "exe_path=%~dp0%dir_name%\build\%exe_base%.exe"
if not exist "%exe_path%" (echo ERROR: "%exe_path%" not found.)

"%exe_path%"
