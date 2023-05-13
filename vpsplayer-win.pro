TEMPLATE = app
QMAKE_CXXFLAGS_RELEASE="-O2 -march=x86-64-v2 -fvisibility-inlines-hidden -pipe -fno-plt -g0 -flto"
QMAKE_LFLAGS_RELEASE="-O2 -flto"
CONFIG += qt warn_off release exceptions_off
QT += widgets multimedia
INCLUDEPATH += rubberband
DEFINES += VERSION_STRING=\\\"2.0\\\"
DEFINES += NO_EXCEPTIONS
MOC_DIR = build_tmp
OBJECTS_DIR = build_tmp
RCC_DIR = build_tmp
HEADERS = src/Audio_player.h \
          src/Player_window.h \
          src/Playing_progress.h \
          src/tools.h
SOURCES = src/main.cpp \
          src/Audio_player.cpp \
          src/Player_window.cpp \
          src/Playing_progress.cpp \
          src/tools.cpp \
          rubberband/single/RubberBandSingle.cpp
RESOURCES = icons.qrc
RC_FILE = vpsplayer-win.rc
TARGET = vpsplayer
