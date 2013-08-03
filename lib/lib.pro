include(../config.pri)

TEMPLATE = lib

QT += network widgets

CONFIG += staticlib

INCLUDEPATH += .
TARGET = ether

HEADERS += \
	changeset.h \
	datatypes.h \
	collabeditor.h \
	etherpadapi.h \
	log.h \
	utils.h \
	websocket.h \

initLib()
