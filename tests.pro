QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase utf8_source
CONFIG -= app_bundle

TEMPLATE = app
include($$PWD/../3rdParty/lua/lua.pri)
SOURCES +=  $$PWD/tests/autotest.cpp
