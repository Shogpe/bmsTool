QMAKE_CFLAGS += "-rpath,\'\$$PWD\'"
QMAKE_CFLAGS += "-rpath,\'\$$PWD\lib'"
INCLUDEPATH += $$PWD/ $$PWD/inc
LIBS += -L$$PWD/Lib -L$$PWD/ -lJLinkARM_x64 -lSYS_x64 -lUTIL_x64
HEADERS += \
    $$PWD/JLinkManager.h
SOURCES += \
    $$PWD/JLinkManager.cpp
