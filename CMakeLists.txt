cmake_minimum_required(VERSION 3.16)

project(ScreenProject VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appScreenProject
    main.cpp
)

qt_add_qml_module(appScreenProject
    URI ScreenProject
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES WeatherData.h
        SOURCES WeatherDataManager.h
        SOURCES WeatherDataManager.cpp
        SOURCES WeeklyWeatherModel.h
        SOURCES WeeklyWeatherModel.cpp
        RESOURCES source.qrc
        RESOURCES json.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appScreenProject PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appScreenProject
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appScreenProject
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appScreenProject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
