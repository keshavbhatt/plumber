cmake_minimum_required(VERSION 3.21)

project(plumber
    VERSION 3.1
    DESCRIPTION "Media trimmer, downloader and video-to-GIF creator"
    HOMEPAGE_URL "https://github.com/keshavbhatt/plumber"
    LANGUAGES CXX
)

set(APP_NAME "Plumber")
set(APP_EXECUTABLE_NAME "plumber")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plumber.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${APP_EXECUTABLE_NAME}.desktop"
    @ONLY
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Developed and shipped against Qt 6.11 (kde-qt6-core24-sdk / kf6-core24).
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Multimedia
    MultimediaWidgets
)

qt_standard_project_setup()

# Suppress qDebug output in non-debug builds.
add_compile_definitions(
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:QT_NO_DEBUG_OUTPUT>
)

# Plain add_executable (not qt_add_executable): the app has no QML, and
# skipping Qt target finalization avoids running the SDK snap's
# qmlimportscanner, which cannot start on non-Ubuntu dev hosts.
add_executable(${APP_EXECUTABLE_NAME}
    src/main.cpp
    src/RangeSlider.cpp
    src/RangeSlider.h
    src/controlbutton.cpp
    src/controlbutton.h
    src/elidedlabel.cpp
    src/elidedlabel.h
    src/engine.cpp
    src/engine.h
    src/error.cpp
    src/error.h
    src/error.ui
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/onlinesearchsuggestion.cpp
    src/onlinesearchsuggestion.h
    src/remotepixmaplabel.cpp
    src/remotepixmaplabel.h
    src/request.cpp
    src/request.h
    src/screenshot.cpp
    src/screenshot.h
    src/screenshot.ui
    src/searchprovider.cpp
    src/searchprovider.h
    src/searchprovider.ui
    src/seekslider.cpp
    src/seekslider.h
    src/settings.cpp
    src/settings.h
    src/settings.ui
    src/utils.cpp
    src/utils.h
    src/waitingspinnerwidget.cpp
    src/waitingspinnerwidget.h
    src/console.ui
    src/track.ui
    src/icons.qrc
    src/qbreeze.qrc
)

# uic-generated headers include promoted-widget headers by bare name.
target_include_directories(${APP_EXECUTABLE_NAME} PRIVATE src)

target_compile_definitions(${APP_EXECUTABLE_NAME} PRIVATE
    APPNAMESTR="${APP_NAME}"
    VERSIONSTR="${PROJECT_VERSION}"
)

target_link_libraries(${APP_EXECUTABLE_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

# Install rules
include(GNUInstallDirs)
install(TARGETS ${APP_EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

if(NOT WIN32)
    install(FILES app/icon-512.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
        RENAME ${APP_EXECUTABLE_NAME}.png
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${APP_EXECUTABLE_NAME}.desktop"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()
