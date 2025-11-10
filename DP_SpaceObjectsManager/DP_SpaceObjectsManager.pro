#-------------------------------------------------
#
# Project created by QtCreator 2018-11-22T10:40:22
#
#-------------------------------------------------

include ($$_PRO_FILE_PWD_/../DP_Locations.pri)
include(../DP_Dependencies.pri)
include(../DP_Application.pri)

QT       += core gui widgets

TARGET = DP_SpaceObjectsManager

CONFIG += c++17

SOURCES += \
    class_spaceobjectsmanagermainwindowcontroller.cpp \
    class_spaceobjectsmanagermainwindowview.cpp \
    main_SpaceObjectsManager.cpp \
    form_satellite.cpp \
    form_save.cpp

HEADERS += \
    class_spaceobjectsmanagermainwindowcontroller.h \
    class_spaceobjectsmanagermainwindowview.h \
    form_satellite.h \
    form_save.h

FORMS += \
        form_satellite.ui \
    form_save.ui \
    spaceobjectsmanagermainwindowview.ui

win32:
{
RC_ICONS = SpaceObjectsManager.ico
}

RESOURCES += \
    spaceobjectsmanager_resources.qrc

DISTFILES += \
    SpaceObjectsManager.ico
