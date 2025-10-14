QT += widgets

CONFIG += c++11

SOURCES += src/main.cpp \
           src/alpsnapgui.cpp \
           src/settingsdialog.cpp

HEADERS += src/alpsnapgui.h \
           src/settingsdialog.h

DEFINES += ALPSNAP_SCRIPT_PATH=\\\"./alpsnap.sh\\\"
