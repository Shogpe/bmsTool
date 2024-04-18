@echo off
setlocal
set MY_PATH=D:\Qt\5.12.11\msvc2017_64\bin;C:\Qt\Tools\QtCreator\bin;D:\Program\7zip;
@echo Setting up environment for Qt usage...
if "%~1" == "" (@echo "%PATH%") else (
set PATH="%~1;%PATH%")

set PACK_EXE=.\7z.exe
::指定打包存储文件夹
set RC_PATH=..\..\发布软件
::指定打包的程序
set BIN_PATH=..\output
set BIN_NAME=bms_tool
set BIN_EXE=%BIN_PATH%\%BIN_NAME%.exe
::获取应用程序版本号
for /f "delims=" %%i in ('powershell "(Get-Item -path '%BIN_EXE%').VersionInfo.ProductVersion"') do set version=%%i
@echo %version%
::指定临时文件夹
set OUTPUT=tmp
set BIN_OUT_PAK=%RC_PATH%\%BIN_NAME%_%version%.7z
::set SFX_MODULES=D:\Program\7zip\7zSD.sfx
set SFX_MODULES=.\7zsd_All_x64.sfx
set BIN_OUT_EXE=%RC_PATH%\%BIN_NAME%%version%Setup.exe
@md %OUTPUT%

::for /f "delims=" %%A in ('dir /b *.exe') do (
::@windeployqt %%A --dir %OUTPUT% --release --no-angle --no-opengl-sw --no-translations 
::@copy %%A %OUTPUT% /Y
::)
@copy %BIN_PATH%\data.db3 %OUTPUT% /Y
@windeployqt %BIN_EXE% --dir %OUTPUT% --release --no-angle --no-opengl-sw --no-translations 
@copy %BIN_EXE% %OUTPUT% /Y
@del %OUTPUT%\styles %OUTPUT%\iconengines %OUTPUT%\imageformats %OUTPUT%\bearer %OUTPUT%\Qt5Svg.dll /S /Q
@rmdir %OUTPUT%\styles %OUTPUT%\iconengines %OUTPUT%\imageformats %OUTPUT%\bearer /S /Q 
::复制其他依赖文件
@del %BIN_OUT_PAK% /S /Q
::@for /f "tokens=*" %%i in (%BIN_PATH%\depend.list) do (@xcopy %BIN_PATH%\"%%i" "%OUTPUT%\%%i" /Y/S)
@for /f "tokens=*" %%i in (%BIN_PATH%\depend.list) do (%PACK_EXE% a %BIN_OUT_PAK% "%BIN_PATH%\%%i")
@echo 打包至%BIN_OUT_PAK%...
%PACK_EXE% a %BIN_OUT_PAK% .\%OUTPUT%\*
@echo 打包自解压文件...
@copy /b %SFX_MODULES% + deploy.txt + %BIN_OUT_PAK% %BIN_OUT_EXE%
@echo 清理文件%OUTPUT%...
@del %OUTPUT% /S /Q
@rmdir %OUTPUT% /S /Q
@pause