#-------------------------------------------------
#
# Project created by QtCreator 2024-04-29T14:16:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RemoteClient
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    cclientcontorler.cpp \
    loginwidget.cpp

HEADERS  += widget.h \
    cclientcontorler.h \
    loginwidget.h

FORMS    += widget.ui \
    loginwidget.ui

RESOURCES += \
    res.qrc
