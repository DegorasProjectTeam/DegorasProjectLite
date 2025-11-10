TEMPLATE = subdirs

SUBDIRS  = \
# ==== Aux Libraries =============================================================================
        LibDPSLR \
        LibJsonTableModel \
# ==== DP Libraries =============================================================================
        DP_MainWindowView \
        DP_Core \
# ==== Main programs ============================================================================
        DP_SpaceObjectsManager \
# ==== Tools projects ============================================================================
        DP_FilterTool \
        DP_FilterTester \
# ==== Test project ==============================================================================

# ==== Main Dependencies =========================================================================
DP_Core.depends = LibDPSLR LibJsonTableModel
DP_SpaceObjectsManager.depends = DP_Core DP_MainWindowView
DP_PredictionsGenerator.depends = DP_Core DP_MainWindowView
DP_CPFManager.depends = DP_Core
DP_FilterTool.depends = DP_Core
DP_FilterTester.depends = DP_Core


RESOURCES += DP_Core/resources/common_resources.qrc \
             DP_MainWindowView/resources/mainwindowview_resources.qrc
