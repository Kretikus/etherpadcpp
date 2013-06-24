include(../config.pri)

TEMPLATE = lib

QT += network

CONFIG += staticlib

INCLUDEPATH += .
TARGET = ether

HEADERS += \
    datatypes.h \
    etherpadapi.h \
    log.h \
    utils.h

initLib()

