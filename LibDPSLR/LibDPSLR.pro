#-------------------------------------------------
#
# Project created by QtCreator 2019-03-21T09:20:15
#
#-------------------------------------------------

include ($$_PRO_FILE_PWD_/../DP_Locations.pri)

TARGET = LibDPSLR
DEFINES += LIBDPSLR_LIBRARY
DESTDIR = $$DP_DEPLOY/lib
TEMPLATE = lib

CONFIG -= qt

SOURCES += \
    sources/algorithms.cpp \
    sources/astronomy.cpp \
    sources/class_cpf.cpp \
    sources/class_crd.cpp \
    sources/class_tle.cpp \
    sources/common.cpp \
    sources/cpfutils.cpp \
    sources/crdutils.cpp \
    sources/geo.cpp \
    sources/helpers.cpp \
    sources/math.cpp \
    sources/utils.cpp

HEADERS += \
    includes/algorithms.h \
    includes/astronomy.h \
    includes/class_cpf.h \
    includes/class_crd.h \
    includes/class_matrix.h \
    includes/class_tle.h \
    includes/common.h \
    includes/cpfutils.h \
    includes/crdutils.h \
    includes/dpslr_math.h \
    includes/dpslr_interval.h \
    includes/geo.h \
    includes/helpers.h \
    includes/helpers.tpp \
    includes/libdpslr_global.h \
    includes/sun.h \
    includes/math.tpp \
    includes/math_definitions.h \
    includes/math_operators.h \
    includes/math_operators.tpp \
    includes/utils.h

DISTFILES += \
    README \
    LICENSE

CONFIG += c++14
LIBS += -fopenmp
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
