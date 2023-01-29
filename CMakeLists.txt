cmake_minimum_required(VERSION 3.5)

project(DiscountTracer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "..." FORCE)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dlls)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DiscountTracer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(DiscountTracer ${PROJECT_SOURCES})
endif()

target_link_libraries(DiscountTracer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(DiscountTracer PRIVATE Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(DiscountTracer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DiscountTracer)
endif()

######installation directly to build folder for proper debugging in VisualStudio####

####################################################################################
install(TARGETS DiscountTracer DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(TARGETS lamoda RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/dlls)
install(FILES testcfg/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
####################################################################################


######normal installation in a folder specified by CMAKE_INSTALL_PREFIX#############

####################################################################################
install(TARGETS DiscountTracer DESTINATION .)
install(TARGETS lamoda RUNTIME DESTINATION ./dlls)
install(FILES testcfg/config.json DESTINATION .)
####################################################################################