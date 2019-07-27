#-------------------------------------------------
#
# Project created by QtCreator 2018-01-05T22:07:21
#
#-------------------------------------------------

QT       += sql core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#QTPLUGIN += sqlitecipher

DEFINES -= UNICODE

TARGET = console
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

RC_FILE += $$PWD/console.rc

SOURCES += main.cpp \
           ini.c \
           readerini.cpp \
           dbbrowser.cpp
#           sqlitedb.cpp \
#           sqlitetablemodel.cpp \
#           sqlitetypes.cpp \
#           $$PWD/grammar/*.cpp


HEADERS += ini.h \
           readerini.h \
           dbbrowser.h
#           sqlitedb.h \
#           sqlitetablemodel.h \
#           sqlitetypes.h \
#           $$PWD/grammar/*.hpp

