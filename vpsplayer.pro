TEMPLATE = app
CONFIG += qt warn_on release link_pkgconfig c++14
QT += widgets multimedia
PKGCONFIG += rubberband
DEFINES += VERSION_STRING=\\\"devel_1.0_pre\\\"
QMAKE_CXXFLAGS_RELEASE = "-O2 -march=native -fvisibility-inlines-hidden -pipe -fno-plt"
HEADERS = Audio_player.h \
          Player_window.h \
          Playing_progress.h
SOURCES = main.cpp \
          Audio_player.cpp \
          Player_window.cpp \
          Playing_progress.cpp
RESOURCES = icons.qrc
TARGET = vpsplayer

documentation.path = /usr/local/doc/VPSPlayer
documentation.files = COPYING README
INSTALLS += documentation

target.path = /usr/local/bin
INSTALLS += target
