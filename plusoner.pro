QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = plusoner

TEMPLATE = app

SOURCES += src/main.cpp\
    src/plusoner.cpp \
    src/plusonerthread.cpp \
    src/proxylist.cpp \
    src/gui.cpp \
    src/yobadb.cpp

HEADERS += \
    src/plusoner.hpp \
    src/plusonerthread.hpp \
    src/proxylist.hpp \
    src/gui.hpp \
    src/yobadb.hpp

FORMS += \
    forms/gui.ui

RESOURCES += \
    res/res.qrc

RC_FILE = res/icon.rc
