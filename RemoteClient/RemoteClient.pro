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
    loginwidget.cpp \
    clientsocket.cpp \
    packet.cpp \
    ctesttool.cpp \
    watchdlg.cpp

HEADERS  += widget.h \
    cclientcontorler.h \
    loginwidget.h \
    clientsocket.h \
    packet.h \
    ctesttool.h \
    watchdlg.h

FORMS    += widget.ui \
    loginwidget.ui \
    cwatchdlg.ui

RESOURCES += \
    res.qrc
LIBS += -lws2_32  #qt 使用windowsAPI需要引入对应的库时需要再pro文件中引�