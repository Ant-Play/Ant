@echo off
pushd %~dp0
call %ANT_DIR%\vendor\bin\premake5.exe vs2022
popd
