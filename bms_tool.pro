#-------------------------------------------------
#
# Project created by QtCreator 2017-05-24T18:38:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = bms_tool
TEMPLATE    = app
MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj
# DESTDIR     = $$PWD/../bin

SOURCES     += main.cpp \
    MessageQueue.cpp \
    main_ui.cpp \
    mb_cmu.cpp \
    modbus-data.c \
    modbus-rtu.c \
    modbus-tcp.c \
    modbus.c \
    utils.cpp \
    widget.cpp
SOURCES     += iconhelper.cpp
SOURCES     += appinit.cpp
SOURCES     +=

HEADERS     += iconhelper.h \
    MessageQueue.h \
    config.h \
    main_ui.h \
    mb_cmu.h \
    modbus-private.h \
    modbus-rtu-private.h \
    modbus-rtu.h \
    modbus-tcp-private.h \
    modbus-tcp.h \
    modbus-version.h \
    modbus.h \
    utils.h \
    version.h \
    widget.h
HEADERS     += appinit.h
HEADERS     +=

FORMS       += \
    main_ui.ui \
    widget.ui

RESOURCES   += main.qrc
RESOURCES   += qss.qrc
CONFIG      += qt warn_off static
INCLUDEPATH += $$PWD
win32: LIBS += -LC:\Qt\5.12.3\mingw73_32\lib -lws2_32

