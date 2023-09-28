:: shold set deploy PATH first
:: PATH+=D:\Program\7zip;%{ActiveProject:QT_INSTALL_BINS}
@echo off
@pushd %~dp0\deploy
@call deploy_x64.bat
@popd