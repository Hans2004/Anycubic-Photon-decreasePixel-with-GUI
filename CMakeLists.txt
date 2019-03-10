cmake_minimum_required(VERSION 3.1.0)

project(decreasePixel)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

#IF(CMAKE_BUILD_TYPE MATCHES Debug)
#  message("Debug build.")
#ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
#  message("Release build.")
#ELSE()
#  message("Some other build type.")
#ENDIF()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

# Populate a CMake variable with the sources
set(decreasePixel_SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
    graphicswidget.cpp
    photonfile.cpp
)
# Tell CMake to create the helloworld executable
add_executable(decreasePixel ${decreasePixel_SRCS})
# Use the Widgets module from Qt 5
target_link_libraries(decreasePixel Qt5::Widgets Qt5::Core Qt5::Gui)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin" COMPONENT applications)



# The cpack only works if the source tree does not have a parent directory with spaces in it. Therefore /home/hans/Documents/Qt Projects will not work.
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "decreasePixel")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Hansie")
set(CPACK_PACKAGE_VENDOR "My Company")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)



