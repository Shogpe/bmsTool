@echo off
setlocal
set MY_PATH=D:\Qt\5.12.11\msvc2017_64\bin;C:\Qt\Tools\QtCreator\bin;D:\Program\7zip;
@echo Setting up environment for Qt usage...
if "%~1" == "" (@echo "%PATH%") else (
set PATH="%~1;%PATH%")
echo %TARGET_NAME%
set PACK_EXE=7z.exe
::指定打包存储文件夹
set RC_PATH=..\发布软件
::指定打包的程序
set BIN_PATH=.\
set BIN_NAME=%TARGET_NAME%
set BIN_EXE=%BIN_PATH%\%BIN_NAME%.exe
::获取应用程序版本号
for /f "delims=" %%i in ('powershell "(Get-Item -path '%BIN_EXE%').VersionInfo.ProductVersion"') do set version=%%i
@echo %version%
::指定临时文件夹
set OUTPUT=.\tmp
set BIN_OUT_PAK=%RC_PATH%\%BIN_NAME%_%version%.7z
@md %OUTPUT%

::for /f "delims=" %%A in ('dir /b *.exe') do (
::@windeployqt %%A --dir %OUTPUT% --release --no-angle --no-opengl-sw --no-translations 
::@copy %%A %OUTPUT% /Y
::)
@windeployqt %BIN_EXE% --dir %OUTPUT% --release --no-angle --no-opengl-sw --no-translations 
@copy %BIN_EXE% %OUTPUT% /Y
@del %OUTPUT%\styles %OUTPUT%\iconengines %OUTPUT%\imageformats %OUTPUT%\bearer %OUTPUT%\Qt5Svg.dll /S /Q
@rmdir %OUTPUT%\styles %OUTPUT%\iconengines %OUTPUT%\imageformats %OUTPUT%\bearer /S /Q 
::复制其他依赖文件
@del %BIN_OUT_PAK% /S /Q
::@for /f "tokens=*" %%i in (%BIN_PATH%\depend.list) do (@xcopy %BIN_PATH%\"%%i" "%OUTPUT%\%%i" /Y/S)
@for /f "tokens=*" %%i in (%BIN_PATH%\%BIN_NAME%.list) do (%PACK_EXE% a %BIN_OUT_PAK% "%BIN_PATH%\%%i")
@echo 打包至%BIN_OUT_PAK%...
%PACK_EXE% a %BIN_OUT_PAK% .\%OUTPUT%\*
@echo 清理文件%OUTPUT%...
@del %OUTPUT% /S /Q
@rmdir %OUTPUT% /S /Q
%PACK_EXE% x %BIN_OUT_PAK% -o%RC_PATH%\%BIN_NAME%\ -y
@pause