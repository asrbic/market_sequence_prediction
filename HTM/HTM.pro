#-------------------------------------------------
#
# Project created by QtCreator 2015-10-17T21:26:23
#
#-------------------------------------------------

QT       += core gui
QMAKE_CFLAGS += -std=c99 -g3 -Wall -O0
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTM
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tp.c \
    graphics.c \
    htmwidget.cpp \
    btf.cpp

HEADERS  += mainwindow.h \
    tp.h \
    graphics.h \
    htmwidget.h \
    btf.h

FORMS    += mainwindow.ui

LIBS += -lboost_system -lboost_filesystem
