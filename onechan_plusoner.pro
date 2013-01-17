QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = onechan_plusoner

TEMPLATE = app

SOURCES += main.cpp\
    sources/plusoner.cpp \
    sources/plusonerthread.cpp \
    sources/proxylist.cpp \
    sources/gui.cpp

HEADERS += \
    sources/plusoner.hpp \
    sources/plusonerthread.hpp \
    sources/proxylist.hpp \
    sources/gui.hpp

FORMS += \
    forms/gui.ui

RESOURCES += \
    resources/res.qrc

RC_FILE = resources//icon.rc
