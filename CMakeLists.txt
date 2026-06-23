cmake_minimum_required(VERSION 3.19)
project(Car_GUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS SerialPort)

qt_standard_project_setup()

qt_add_executable(Car_GUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    serial.cpp
    serial.h
    debug.h
    comm.h
    comm.cpp
)

qt_add_translations(
    TARGETS Car_GUI
    TS_FILES Car_GUI_en_001.ts
)

target_link_libraries(Car_GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::SerialPort
)

install(TARGETS Car_GUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Car_GUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
