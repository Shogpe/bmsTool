#-------------------------------------------------
#
# Project created by QtCreator 2017-05-24T18:38:15
#
#-------------------------------------------------

QT       += core gui xml network sql
# QT += quickwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
win32 {
}
#TARGET      = bms_tool
TEMPLATE    = app
#CONFIG(debug, debug|release){
DESTDIR     = $$PWD/output/
#} else {
#DESTDIR     = $$PWD/output/debug
#}
#MOC_DIR     = $$DESTDIR/temp/moc
#RCC_DIR     = $$DESTDIR/temp/rcc
#UI_DIR      = $$DESTDIR/temp/ui
#OBJECTS_DIR = $$DESTDIR/temp/obj

#include(ThirdParty/ThirdParty.pri)
# include(ThirdParty/log4qt/log4qt.pri)
include(models/models.pri)
include(3rdParty.pri)
include(protocol/protocol.pri)
include(views/views.pri)

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
    models/qlineeditext.h \
    tableModel.h \
    models/QFramelessWidget/qframelesswidget.h \
    stategroupbox.h \
    utils.h \
    version.h \
    widget.h
HEADERS     += appinit.h cmu4u.h
HEADERS     += socImporter.h db_manager.h downloadmanager.h
SOURCES     += socImporter.cpp db_manager.cpp cmu4u.cpp  downloadmanager.cpp
FORMS       += cmu4u.ui \
    main_ui.ui \
    stategroupbox.ui \
    widget.ui

RESOURCES   += other/main.qrc
RESOURCES   += other/qss.qrc
# RESOURCES   += qml/dynamic.qml
CONFIG += qt warn_off c++11 utf8_source
INCLUDEPATH += $$PWD
win32 {
LIBS += -lws2_32
}
macx {
# mac only
}
unix:!macx{
# linux only
LIBS += -licudata
}
RC_FILE += main.rc
OTHER_FILES += main.rc touch.bat

DEFINES += QT_NO_DEBUG_OUTPUT #禁止qDebug()输出

