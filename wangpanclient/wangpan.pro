#-------------------------------------------------
#
# Project created by QtCreator 2023-01-09T12:06:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wangpan
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    login.cpp \
    ckernel.cpp \
    cselectfile.cpp \
    csdownfile.cpp

HEADERS  += mainwindow.h \
    login.h \
    ckernel.h \
    cselectfile.h \
    csdownfile.h

FORMS    += mainwindow.ui \
    login.ui \
    cselectfile.ui \
    csdownfile.ui

include(./md5/md5.pri)
include(./netapi/netapi.pri)
INCLUDEPATH += ./md5/
INCLUDEPATH += ./netapi/

RESOURCES += \
    res.qrc
