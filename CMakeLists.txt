cmake_minimum_required(VERSION 3.5)


set(APP_NAME telemetry)
set(APP_DOMAIN it.unitn)
project(${APP_NAME} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QT-related
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)
set(USR_DIR ${EXTERNAL_DIR}/usr)
set(SRC_DIR ${EXTERNAL_DIR}/src)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(MAIN_DIR ${SOURCE_DIR}/main)

configure_file(
  ${SOURCE_DIR}/defines.h.in
  ${SOURCE_DIR}/defines.h
)

include_directories(${USR_DIR}/include ${SOURCE_DIR})
link_directories(${USR_DIR}/lib)

file(GLOB LIB_SOURCES ${SOURCE_DIR}/*(.h|.hpp|.cpp|.ui))
set(PROJECT_SOURCES ${LIB_SOURCES} ${MAIN_DIR}/main.cpp)

qt_add_executable(telemetry
    MANUAL_FINALIZATION
    ${LIB_SOURCES} ${MAIN_DIR}/main.cpp
)

target_link_libraries(telemetry PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER it.unitn.telemetry)

set_target_properties(telemetry PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTORCC TRUE
)

qt_finalize_executable(telemetry)

if(WIN32)
    set(LIB_LIST snappy.lib zmqpp.lib libzmq-static-mt-s-4_3_5.lib Ws2_32 Iphlpapi)
else()
    set(LIB_LIST libzmqpp-static.a libzmq.a libsnappy.a pthread)        # ${PTHREADS} for MAC OS
endif()

add_executable(pub ${MAIN_DIR}/pub.cpp)
target_link_libraries(pub ${LIB_LIST})

add_executable(sub ${MAIN_DIR}/sub.cpp)
target_link_libraries(sub ${LIB_LIST})

target_link_libraries(telemetry PRIVATE ${LIB_LIST})

set(TARGET_LIST pub sub telemetry)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/products)
#include(GNUInstallDirs)
install(TARGETS ${TARGET_LIST}
    BUNDLE DESTINATION bin
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# External library
if (NOT EXIST ${URS_DIR})
    message(STATUS "Compiling third party libraries")
    include(external.cmake)
    foreach(t IN ITEMS ${TARGET_LIST})
        add_dependencies(${t} libzmq zmqpp snappy)
    endforeach()
endif()

