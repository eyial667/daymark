# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.21)

project(
    Daymark
    VERSION 0.2.1
    DESCRIPTION "A local-first daily command center"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui LinguistTools Network Quick QuickControls2 Sql)

qt_standard_project_setup(REQUIRES 6.5)

# Qt 6.8+ generates qmldir metadata for source subdirectories. Older supported
# Qt releases do not know this policy, so keep the guard for 6.5 compatibility.
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

add_library(daymark_core STATIC
    src/core/priorityengine.cpp
    src/core/priorityengine.h
    src/data/taskrepository.cpp
    src/data/taskrepository.h
    src/domain/category.h
    src/domain/dailynote.h
    src/domain/goal.h
    src/domain/task.h
    src/presentation/appsettings.cpp
    src/presentation/appsettings.h
    src/presentation/datatransferservice.cpp
    src/presentation/datatransferservice.h
    src/presentation/dailynotemodel.cpp
    src/presentation/dailynotemodel.h
    src/presentation/focussessionmodel.cpp
    src/presentation/focussessionmodel.h
    src/presentation/categorylistmodel.cpp
    src/presentation/categorylistmodel.h
    src/presentation/goallistmodel.cpp
    src/presentation/goallistmodel.h
    src/presentation/languagemanager.cpp
    src/presentation/languagemanager.h
    src/presentation/mentalmapmodel.cpp
    src/presentation/mentalmapmodel.h
    src/presentation/tasklistmodel.cpp
    src/presentation/tasklistmodel.h
    src/presentation/updateservice.cpp
    src/presentation/updateservice.h
)

target_include_directories(daymark_core PUBLIC src)
target_link_libraries(daymark_core PUBLIC Qt6::Core Qt6::Gui Qt6::Network Qt6::Sql)

if(MSVC)
    target_compile_options(daymark_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(daymark_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_add_executable(daymark
    src/app/main.cpp
)

qt_add_resources(daymark "daymark_icons"
    PREFIX "/"
    FILES assets/icons/org.daymark.dashboard.svg
)

qt_add_qml_module(daymark
    URI Daymark
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/AppButton.qml
        qml/CategoriesView.qml
        qml/DateField.qml
        qml/DayPlanningPrompt.qml
        qml/DailyNotePanel.qml
        qml/EmptyState.qml
        qml/FocusView.qml
        qml/GoalsView.qml
        qml/HybridView.qml
        qml/MetricCard.qml
        qml/MentalMapClouds.qml
        qml/MentalMapConstellation.qml
        qml/MidnightCommandView.qml
        qml/QuickAddDialog.qml
        qml/QuietFocusView.qml
        qml/ReviewView.qml
        qml/SettingsView.qml
        qml/Sidebar.qml
        qml/SpatialMapView.qml
        qml/TaskRow.qml
        qml/Theme.qml
        qml/TodoListView.qml
        qml/UpdateDialog.qml
)

target_link_libraries(daymark PRIVATE
    daymark_core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

target_compile_definitions(daymark PRIVATE DAYMARK_VERSION="${PROJECT_VERSION}")

set_target_properties(daymark PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Daymark"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.daymark.dashboard"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS daymark
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    LICENSE
    LICENSES/GPL-3.0-only.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/daymark
)

# The host-compatible Linux archive uses the distribution's complete Qt
# runtime. In particular, it must not install qt.conf or a partial plugin tree,
# because either would hide Wayland, GLX, and EGL plugins from the application.
# The later AppImage pipeline will bundle and validate its own complete runtime.
if(UNIX AND NOT APPLE)
    install(
        PROGRAMS packaging/linux/install.sh
        DESTINATION .
    )
    install(
        FILES packaging/linux/org.daymark.dashboard.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(
        FILES assets/icons/org.daymark.dashboard.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    )
endif()

if(NOT (UNIX AND NOT APPLE))
    qt_generate_deploy_qml_app_script(
        TARGET daymark
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        NO_TRANSLATIONS
    )
    install(SCRIPT ${deploy_script})
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(priorityengine_tests tests/priorityengine_test.cpp)
    target_link_libraries(priorityengine_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME priorityengine_tests COMMAND priorityengine_tests)

    qt_add_executable(taskrepository_tests tests/taskrepository_test.cpp)
    target_link_libraries(taskrepository_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME taskrepository_tests COMMAND taskrepository_tests)

    qt_add_executable(tasklistmodel_tests tests/tasklistmodel_test.cpp)
    target_link_libraries(tasklistmodel_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME tasklistmodel_tests COMMAND tasklistmodel_tests)

    qt_add_executable(categorylistmodel_tests tests/categorylistmodel_test.cpp)
    target_link_libraries(categorylistmodel_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME categorylistmodel_tests COMMAND categorylistmodel_tests)

    qt_add_executable(appsettings_tests tests/appsettings_test.cpp)
    target_link_libraries(appsettings_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME appsettings_tests COMMAND appsettings_tests)

    qt_add_executable(goallistmodel_tests tests/goallistmodel_test.cpp)
    target_link_libraries(goallistmodel_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME goallistmodel_tests COMMAND goallistmodel_tests)

    qt_add_executable(datatransfer_tests tests/datatransfer_test.cpp)
    target_link_libraries(datatransfer_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME datatransfer_tests COMMAND datatransfer_tests)

    qt_add_executable(focussessionmodel_tests tests/focussessionmodel_test.cpp)
    target_link_libraries(focussessionmodel_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME focussessionmodel_tests COMMAND focussessionmodel_tests)

    qt_add_executable(mentalmapmodel_tests tests/mentalmapmodel_test.cpp)
    target_link_libraries(mentalmapmodel_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME mentalmapmodel_tests COMMAND mentalmapmodel_tests)

    qt_add_executable(dailynotemodel_tests tests/dailynotemodel_test.cpp)
    target_link_libraries(dailynotemodel_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME dailynotemodel_tests COMMAND dailynotemodel_tests)

    qt_add_executable(updateservice_tests tests/updateservice_test.cpp)
    target_link_libraries(updateservice_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME updateservice_tests COMMAND updateservice_tests)

    qt_add_executable(languagemanager_tests tests/languagemanager_test.cpp)
    target_link_libraries(languagemanager_tests PRIVATE daymark_core Qt6::Test)
    add_test(NAME languagemanager_tests COMMAND languagemanager_tests)
endif()

set(daymark_translation_targets daymark)
if(BUILD_TESTING)
    list(APPEND daymark_translation_targets languagemanager_tests)
endif()

qt_add_translations(
    TARGETS ${daymark_translation_targets}
    SOURCE_TARGETS daymark daymark_core
    TS_FILES translations/daymark_fr.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_TARGET daymark_lupdate
    LRELEASE_TARGET daymark_lrelease
)

set(CPACK_PACKAGE_NAME "Daymark")
set(CPACK_PACKAGE_VENDOR "Daymark contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A local-first daily command center")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(WIN32)
    # Build a real per-user installer instead of a portable archive. Inno
    # Setup's lowest privilege level keeps the default destination under the
    # current user's local application data and avoids an elevation prompt.
    set(CPACK_GENERATOR "INNOSETUP")
    set(CPACK_PACKAGE_FILE_NAME
        "Daymark-${PROJECT_VERSION}-win64-setup"
    )
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Daymark")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "org.daymark.dashboard")
    set(CPACK_PACKAGE_EXECUTABLES "daymark" "Daymark")
    set(CPACK_CREATE_DESKTOP_LINKS "daymark")

    set(CPACK_INNOSETUP_INSTALL_ROOT "{localappdata}/Programs")
    set(CPACK_INNOSETUP_SETUP_PrivilegesRequired "lowest")
    set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)
    set(CPACK_INNOSETUP_PROGRAM_MENU_FOLDER "Daymark")
    set(CPACK_INNOSETUP_CREATE_UNINSTALL_LINK ON)
    set(CPACK_INNOSETUP_RUN_EXECUTABLES "daymark")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
