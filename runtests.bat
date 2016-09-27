@echo off

set SRC_DIR=%~dp0
set BUILD_DIR=%SRC_DIR%build\
if not defined config set config=Release

cd %BUILD_DIR%

if defined APPVEYOR if "%APPVEYOR%" == "True" goto skip-build

@rem Now to build the tests
cmake .. -G"%CMAKE_GENERATOR_NAME%"
msbuild uvw.sln /p:Configuration=%config% /p:Platform="%msbuild_platform%" /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo /m

:skip-build
@rem one more thing before running the tests, the executables need `libuv.dll`
@rem in the loader path. The easiest is in the same directory.
copy %SRC_DIR%deps\libuv\%config%\libuv.dll %BUILD_DIR%test\bin\%config%

@rem run the tests now
msbuild RUN_TESTS.vcxproj /p:Configuration=%config% /p:Platform="%msbuild_platform%" /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo /m

cd %SRC_DIR%
