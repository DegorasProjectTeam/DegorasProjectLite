include(DP_ExternalDependencies.pri)

LIBS += -L$$DP_DEPLOY/lib/ -lDP_Core

INCLUDEPATH += $$DP_ROOT/DP_Core/includes \
               $$DP_ROOT/DP_Core/resources/includes

