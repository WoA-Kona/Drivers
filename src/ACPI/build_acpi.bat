@echo off

set WORKSPACE=%CD%

for /f %%a in (%WORKSPACE%\src\VARIANTS) do (
    if not exist %WORKSPACE%\src\%%a (
        echo ERROR: "%WORKSPACE%\src\%%a" doesn't exist!
        goto :EOF
    )
    call :build %%a
)

goto :EOF

:build
set VARIANT=%1
set BUILD_FOLDER=%INTDIR%\TEMP\%VARIANT%
set ACPI_OUT_FOLDER=%OUTDIR%\ACPI\%VARIANT%

if not exist %BUILD_FOLDER% (
    mkdir %BUILD_FOLDER%
)

if not exist %ACPI_OUT_FOLDER% (
    mkdir %ACPI_OUT_FOLDER%
)

del /q %BUILD_FOLDER%\*
del /q %ACPI_OUT_FOLDER%\*

for /f %%a in (%WORKSPACE%\src\%VARIANT%\PARENTS) do (
    echo Copying %WORKSPACE%\src\%%a\* to %BUILD_FOLDER%
    copy /y %WORKSPACE%\src\%%a\* %BUILD_FOLDER%
)

echo Copying %WORKSPACE%\src\%VARIANT%\* to %BUILD_FOLDER%
copy /y %WORKSPACE%\src\%VARIANT%\* %BUILD_FOLDER%

for /f %%a in ('dir /b %BUILD_FOLDER%\*.aslc') do (
    cl /nologo /Fo%BUILD_FOLDER%\%%~na.obj /WX /c /TC /I%WORKSPACE%\inc %BUILD_FOLDER%\%%a
    if not errorlevel 0 goto :EOF
    if errorlevel 1 goto :EOF
    link /DLL /MACHINE:ARM64 /NODEFAULTLIB /NOENTRY /NOLOGO /OUT:%BUILD_FOLDER%\%%~na.dll %BUILD_FOLDER%\%%~na.obj
    if not errorlevel 0 goto :EOF
    if errorlevel 1 goto :EOF
    python %WORKSPACE%\tools\extract-acpi.py %BUILD_FOLDER%\%%~na.dll -o %ACPI_OUT_FOLDER%\%%~na.aml
    if not errorlevel 0 goto :EOF
    if errorlevel 1 goto :EOF
)

pushd %BUILD_FOLDER%

for /f %%a in ('dir /b %BUILD_FOLDER%\dsdt.asl') do (
    %WORKSPACE%\tools\asl /nologo /Fo=%ACPI_OUT_FOLDER%\%%~na.aml %BUILD_FOLDER%\%%~na.asl
    if not errorlevel 0 goto :EOF
)

popd