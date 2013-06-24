include (../config.pri)

INCLUDEPATH += ../lib

QT += testlib network widgets
CONFIG += console

HEADERS += testutil.h

SOURCES += \
	main.cpp \
	testutil.cpp

TARGET = unittest

initApp(ether)

