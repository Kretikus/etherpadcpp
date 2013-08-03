include (../config.pri)

INCLUDEPATH += ../lib

QT += testlib network widgets
CONFIG += console

HEADERS += ../test/testutil.h
SOURCES += ../test/testutil.cpp

HEADERS += editor_test.h \

SOURCES += \
	editor_test.cpp \
	guitest.cpp \

TARGET = guitest

initApp(ether)

