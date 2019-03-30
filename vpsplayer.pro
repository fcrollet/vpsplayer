TEMPLATE = app
CONFIG += qt warn_on release link_pkgconfig c++14
QT += widgets multimedia
PKGCONFIG += rubberband
DEFINES += VERSION_STRING=\\\"devel_1.0_pre\\\"
QMAKE_CXXFLAGS_RELEASE = "-O2 -march=native -fvisibility-inlines-hidden -pipe -fno-plt"
MOC_DIR = build_tmp
OBJECTS_DIR = build_tmp
RCC_DIR = build_tmp
HEADERS = src/Audio_player.h \
          src/Player_window.h \
          src/Playing_progress.h
SOURCES = src/main.cpp \
          src/Audio_player.cpp \
          src/Player_window.cpp \
          src/Playing_progress.cpp
RESOURCES = icons.qrc
TARGET = vpsplayer

documentation.path = /usr/local/share/doc/VPSPlayer
documentation.files = COPYING README.md

target.path = /usr/local/bin

icon.path = /usr/share/pixmaps
icon.files = icons/vps-64.png

desktop.path = /usr/share/applications
desktop.files = vpsplayer.desktop

INSTALLS += target documentation icon desktop
