TEMPLATE = app
QMAKE_CXXFLAGS_RELEASE = "-O2 -march=x86-64-v2 -fvisibility-inlines-hidden -pipe -fno-plt -g0 -flto"
QMAKE_LFLAGS_RELEASE += "-O2 -march=x86-64-v2 -fvisibility-inlines-hidden -g0 -flto"
CONFIG += qt warn_on release exceptions_off
QT += widgets multimedia
INCLUDEPATH += rubberband
DEFINES += VERSION_STRING=\\\"2.1\\\"
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
TARGET = vpsplayer

target.path = /usr/bin

icon64.path = /usr/share/icons/hicolor/64x64/apps
icon64.files = icons/64x64/com.github.fcrollet.vpsplayer.png

icon128.path = /usr/share/icons/hicolor/128x128/apps
icon128.files = icons/128x128/com.github.fcrollet.vpsplayer.png

desktop.path = /usr/share/applications
desktop.files = com.github.fcrollet.vpsplayer.desktop

INSTALLS += target icon64 icon128 desktop
