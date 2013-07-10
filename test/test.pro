include (../config.pri)

INCLUDEPATH += ../lib

QT += testlib network widgets
CONFIG += console

HEADERS += testutil.h

SOURCES += \
	changeset_apply_test.cpp \
	changeset_create_test.cpp \
	changeset_test.cpp \
	etherpadapi_test.cpp \
	main.cpp \
	testutil.cpp \
	log_test.cpp \

TARGET = unittest

initApp(ether)

