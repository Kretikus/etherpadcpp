
PROJECT_ROOT=$$PWD

exists($${PROJECT_ROOT}/local.pri) {
        include($${PROJECT_ROOT}/local.pri)
} else {
	 message("note: local config file (local.pri) not available")
}

CONFIG(debug, debug|release)|CONFIG(DebugBuild) {
        CONFIG -= debug release
        CONFIG += debug
        DEBUG_OR_RELEASE = debug
        DEBUG_TAG = _debug
} else {
        CONFIG -= debug release
        CONFIG += release
        DEBUG_OR_RELEASE = release
        DEBUG_TAG =
        DEFINES += NDEBUG
}

# generated libraries
LIB_DIR      = $${PROJECT_ROOT}/libs/$${DEBUG_OR_RELEASE}

# generated binaries
BIN_DIR      = $${PROJECT_ROOT}/bin/$${DEBUG_OR_RELEASE}

# complied objects
OBJECTS_DIR  = build/$${DEBUG_OR_RELEASE}

# generated files
MOC_DIR      = build/gen/moc
UI_DIR       = build/gen/ui
SER_DIR      = build/gen/ser
RCC_DIR      = build/gen/rcc

win32 {
        LIB_PREFIX  =
        LIB_POSTFIX = lib
        EXE_POSTFIX = .exe
} else {
        LIB_PREFIX  = lib
        LIB_POSTFIX = a
        EXE_POSTFIX =
}

QMAKE_LIBDIR += $${LIB_DIR} $${EXT_LIB_DIR}
DESTDIR       = $${BIN_DIR}

defineTest(initLib) {
        TEMPLATE = lib

        for(hFile, HEADERS) {
                cppFile = $$replace(hFile,\\.h,.cpp)
                exists($$cppFile):SOURCES *= $$cppFile
        }

        CONFIG += staticlib
        DESTDIR = $${LIB_DIR}

        export(TEMPLATE)
        export(SOURCES)
        export(CONFIG)
        export(DESTDIR)
}

defineTest(initApp) {
        TEMPLATE = app

        for(lib, 1) {
                LIBS += -l$${lib}
                POST_TARGETDEPS += $${LIB_DIR}/$${LIB_PREFIX}$${lib}.$${LIB_POSTFIX}
        }
        LIBS += $$EXT_LIBS

        # seems to be a bug in Qt 4.5.1
        contains(QT, gui) {
                win32:LIBS += -lgdi32 -lUser32
        }

        export(TEMPLATE)
        export(LIBS)
        export(POST_TARGETDEPS)

        !win32:release:!no_strip:console {
                QMAKE_POST_LINK += strip $${DESTDIR}/$${TARGET} ;
                export(QMAKE_POST_LINK)
        }

        macx:console {
                CONFIG -= app_bundle
                export(CONFIG)
        }
}
