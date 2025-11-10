include ($$_PRO_FILE_PWD_/../DP_Locations.pri)
include(../DP_Dependencies.pri)
include(../DP_Application.pri)

QT += core gui widgets
CONFIG += qwt c++17

TARGET = DP_FilterTool

HEADERS = \
    mainwindow.h \
    plot.h \
    qwt_slrplot_picker.h \
    errorplot.h

SOURCES = \
    main.cpp \
    mainwindow.cpp \
    plot.cpp \
    qwt_slrplot_picker.cpp \
    errorplot.cpp

FORMS += \
    mainwindow.ui

