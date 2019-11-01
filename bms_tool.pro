#-------------------------------------------------
#
# Project created by QtCreator 2017-05-24T18:38:15
#
#-------------------------------------------------

QT       += core gui xml
# QT += quickwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = bms_tool
TEMPLATE    = app
#CONFIG(debug, debug|release){
DESTDIR     = $$PWD/output/release
#} else {
DESTDIR     = $$PWD/output/debug
#}
#MOC_DIR     = $$DESTDIR/temp/moc
#RCC_DIR     = $$DESTDIR/temp/rcc
#UI_DIR      = $$DESTDIR/temp/ui
#OBJECTS_DIR = $$DESTDIR/temp/obj

include(ThirdParty/ThirdParty.pri)
# include(ThirdParty/log4qt/log4qt.pri)
include(models/models.pri)
# include(ThirdParty/view/view.pri)
include(protocol/protocol.pri)

SOURCES     += main.cpp \
    MessageQueue.cpp \
    main_ui.cpp \
    stategroupbox.cpp \
    utils.cpp \
    widget.cpp
SOURCES     += iconhelper.cpp
SOURCES     += appinit.cpp

HEADERS     += iconhelper.h \
    MessageQueue.h \
    main_ui.h \
    myhelper.h \
    models/MyDoubleSpinBox/MyDoubleSpinBox.h \
    models/SOEModel/SOEModel.h \
    models/QFramelessWidget/qframelesswidget.h \
    stategroupbox.h \
    utils.h \
    version.h \
    widget.h
HEADERS     += appinit.h

FORMS       += \
    main_ui.ui \
    stategroupbox.ui \
    widget.ui

RESOURCES   += other\main.qrc
RESOURCES   += other\qss.qrc
# RESOURCES   += qml\dynamic.qml
CONFIG      += qt warn_off static
INCLUDEPATH += $$PWD
win32: LIBS += -LC:\Qt\5.12.3\mingw73_32\lib -lws2_32

#-------------------------------------------------
#
# Project Info created by bao 2019-09-25T09:51:37
#
#-------------------------------------------------
VERSION = 1.0.0.0
RC_ICONS = "other/main.ico"
QMAKE_TARGET_PRODUCT = "bms_tool"
QMAKE_TARGET_COMPANY = "Cubenergy"
QMAKE_TARGET_DESCRIPTION = "CMU Debug Tool"
QMAKE_TARGET_COPYRIGHT = "Copyright 2019 Ganing"
RC_LANG = 0x0804
RC_FILE += main.rc
