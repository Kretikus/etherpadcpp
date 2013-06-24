include (../config.pri)

INCLUDEPATH += ../lib

QT += testlib network widgets
CONFIG += console

HEADERS += testutil.h

SOURCES += \
	main.cpp \
	testutil.cpp \
	log_test.cpp \

TARGET = unittest

initApp(ether)

