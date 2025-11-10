#-------------------------------------------------
#
# Project created by QtCreator 2019-03-21T09:20:15
#
#-------------------------------------------------

include ($$_PRO_FILE_PWD_/../DP_Locations.pri)

TARGET = LibJsonTableModel
DEFINES += LIBJSONTABLEMODEL_LIBRARY
DESTDIR = $$DP_DEPLOY/lib

QT -= gui
TEMPLATE = lib
#CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x070000

SOURCES += \
    sources/class_jsontablemodel.cpp 

HEADERS += \
    includes/class_jsontablemodel.h \
    includes/libjsontablemodel_global.h 
