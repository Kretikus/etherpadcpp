include(../config.pri)

TEMPLATE = lib

QT += network

CONFIG += staticlib

INCLUDEPATH += .
TARGET = ether

HEADERS += \
    changeset.h \
    datatypes.h \
    etherpadapi.h \
    log.h \
    utils.h \
    websocket.h \

initLib()

