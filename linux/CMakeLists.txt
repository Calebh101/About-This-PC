cmake_minimum_required(VERSION 3.16)

project(AboutThisPC VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg Network)
find_package(Vulkan REQUIRED)
find_package(X11 REQUIRED)

find_path(XRANDR_INCLUDE_DIR X11/extensions/Xrandr.h)
find_library(XRANDR_LIBRARY Xrandr)

if (NOT XRANDR_INCLUDE_DIR OR NOT XRANDR_LIBRARY)
    message(FATAL_ERROR "XRandR not found")
endif()

include_directories(${X11_INCLUDE_DIR} ${XRANDR_INCLUDE_DIR})

set(PROJECT_SOURCES

        mainwindow.cpp
        mainwindow.h
        logger.cpp
        logger.h
        tabpage.cpp
        tabpage.h
        json.hpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AboutThisPC
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        logger.h logger.cpp
        tabpage.h tabpage.cpp
        images/default-linux-icon.png
        default-linux-icon.qrc
        application-icon.qrc
        main.cpp
        displays.h displays.cpp
        classicpage.h classicpage.cpp
        global.h global.cpp
        computers.qrc
        supportpage.h supportpage.cpp
        icons.qrc
        cicon.h cicon.cpp
        themelistener.h themelistener.cpp
        storage.h storage.cpp
        binaries.qrc
        data.h data.cpp
        settings.hpp settings.cpp
        updatemanager.h updatemanager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AboutThisPC APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AboutThisPC SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AboutThisPC
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AboutThisPC PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Vulkan::Vulkan ${X11_LIBRARIES} ${XRANDR_LIBRARY})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.calebh101.AboutThisPC)
endif()
set_target_properties(AboutThisPC PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AboutThisPC
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AboutThisPC)
endif()
