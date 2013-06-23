TEMPLATE = lib

QT += network

CONFIG += staticlib

INCLUDEPATH += .
TARGET = ether

HEADERS += \
    datatypes.h \
    etherpadapi.h \
    utils.h

SOURCES += \
    datatypes.cpp \
    etherpadapi.cpp
