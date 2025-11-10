include ($$_PRO_FILE_PWD_/../DP_Locations.pri)
include(../DP_Dependencies.pri)
include(../DP_Application.pri)

TARGET = DP_FilterTester

QT += core gui widgets

CONFIG += c++17 qwt

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x070000
DEFINES += APP_NAME=ALGORITHMSTESTER

HEADERS += \
    class_mainwindow.h \
    class_staticresidualsplot.h

SOURCES += \
    class_mainwindow.cpp \
    class_staticresidualsplot.cpp \
    main.cpp

FORMS += \
    form_mainwindow.ui

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp
