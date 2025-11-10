include ($$_PRO_FILE_PWD_/../DP_Locations.pri)

QT += core gui widgets
CONFIG      += c++17 staticlib
TARGET      = DP_MainWindowView
TEMPLATE    = lib
DESTDIR = $$DP_DEPLOY/lib

HEADERS += class_salaramainwindowview.h
SOURCES += \
    class_salaramainwindowview.cpp

FORMS += form_salaramainwindowview.ui

