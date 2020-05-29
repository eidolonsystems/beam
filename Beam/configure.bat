@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
SET DIRECTORY=%~dp0
SET DEPENDENCIES=
SET IS_DEPENDENCY=
:begin_args
SET ARG=%~1
IF "!IS_DEPENDENCY!" == "1" (
  SET DEPENDENCIES=!ARG!
  SET IS_DEPENDENCY=
  SHIFT
  GOTO begin_args
) ELSE IF NOT "!ARG!" == "" (
  IF "!ARG:~0,3!" == "-DD" (
    SET IS_DEPENDENCY=1
  )
  SHIFT
  GOTO begin_args
)
IF "!DEPENDENCIES!" == "" (
  SET DEPENDENCIES=!ROOT!\Dependencies
)
IF NOT EXIST "!DEPENDENCIES!" (
  MD "!DEPENDENCIES!"
)
PUSHD "!DEPENDENCIES!"
SET RUN_SETUP=
IF NOT EXIST last_check.txt (
  SET RUN_SETUP=1
) ELSE (
  FOR /F %%i IN (
      'ls -l --time-style=full-iso "!DIRECTORY!setup.bat" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
        'ls -l --time-style=full-iso last_check.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" == "%%j" (
        SET RUN_SETUP=1
      )
    )
  )
)
IF "!RUN_SETUP!" == "1" (
  CALL "!DIRECTORY!setup.bat"
  ECHO timestamp > last_check.txt
)
POPD
IF NOT "!DEPENDENCIES!" == "!ROOT!\Dependencies" (
  IF EXIST Dependencies (
    RD /S /Q Dependencies
  )
  mklink /j Dependencies "!DEPENDENCIES!" > NUL
)
SET RUN_CMAKE=
FOR /F %%i IN ('DIR /a-d /b /s "!DIRECTORY!Include\*.hpp" ^| wc -l') DO (
  IF EXIST CMakeFiles\hpp_count.txt (
    FOR /F %%j IN ('TYPE CMakeFiles\hpp_count.txt') DO (
      IF NOT "%%i" == "%%j" (
        SET RUN_CMAKE=1
      )
    )
  ) ELSE (
    SET RUN_CMAKE=1
  )
  IF "!RUN_CMAKE!" == "1" (
    ECHO %%i > CMakeFiles\hpp_count.txt
  )
)
FOR /F %%i IN ('DIR /a-d /b /s "!DIRECTORY!Source\*.cpp" ^| wc -l') DO (
  IF EXIST CMakeFiles\cpp_count.txt (
    FOR /F %%j IN ('TYPE CMakeFiles\cpp_count.txt') DO (
      IF NOT "%%i" == "%%j" (
        SET RUN_CMAKE=1
      )
    )
  ) ELSE (
    SET RUN_CMAKE=1
  )
  IF "!RUN_CMAKE!" == "1" (
    ECHO %%i > CMakeFiles\cpp_count.txt
  )
)
IF "!RUN_CMAKE!" == "1" (
  cmake -S !DIRECTORY! -DD=!DEPENDENCIES!
)
ENDLOCAL
