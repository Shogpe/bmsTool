#-------------------------------------------------
#
# Project created by QtCreator 2017-05-24T18:38:15
#
#-------------------------------------------------

QT       += core gui xml network
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

RESOURCES   += other/main.qrc
RESOURCES   += other/qss.qrc
# RESOURCES   += qml/dynamic.qml
CONFIG += qt warn_off c++11 utf8_source
INCLUDEPATH += $$PWD
win32: LIBS += -lws2_32
macx {
# mac only
}
unix:!macx{
# linux only
LIBS += -licudata
}
RC_FILE += main.rc
