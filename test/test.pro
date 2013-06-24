include (../config.pri)

INCLUDEPATH += ../lib

QT += testlib network widgets
CONFIG += console

SOURCES += main.cpp

TARGET = unittest

initApp(ether)

