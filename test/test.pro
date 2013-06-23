TEMPLATE = app

INCLUDEPATH += ../lib

QT += testlib network widgets

TARGET = unittest

SOURCES += main.cpp

LIBS += -L../lib/debug -lether
