CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT("TD2 OpenCL" VERSION 1.0.0 LANGUAGES CXX)

# Variable for the executable name & additional OpenCL file to be copied inside the build directory
SET(EXEC_NAME main)
SET(others MNT.txt MIN.txt)

# Pass variable from CMake to the code through a header file
SET(filename MIN.txt)
OPTION(VERBOSE "Print verbose messages" OFF)
IF (VERBOSE)
    MESSAGE(STATUS "Verbose messages enabled")
    ADD_COMPILE_OPTIONS(-DVERBOSE)
ENDIF()
CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
)
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

# Set C++ standard
SET(CMAKE_CXX_STANDARD 20)

# Add compile flags for warnings and optimization
IF(${CMAKE_BUILD_TYPE} MATCHES RELEASE)
    ADD_COMPILE_OPTIONS(-O3)
ELSE()
    ADD_COMPILE_OPTIONS(-g -O0 -Wall -Wextra -pedantic)
ENDIF()

# Use FindOpenCL through FIND_PACKAGE
FIND_PACKAGE(OpenCL REQUIRED)

# Add all cpp files to the "SOURCES" variable
FILE(GLOB SOURCES "*.cpp")

# Copy each file inside "others" array to the build directory
FOREACH(FILE IN LISTS others)
    CONFIGURE_FILE(
            ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
            ${CMAKE_CURRENT_BINARY_DIR} COPYONLY
    )
ENDFOREACH()

# Create the executable rule
ADD_EXECUTABLE(${EXEC_NAME} ${SOURCES})
# Link to OpenCL libraries through cmake 3.1 targets
TARGET_LINK_LIBRARIES(${EXEC_NAME} OpenCL::OpenCL)

ADD_CUSTOM_TARGET(
        Valgrind
        COMMAND valgrind --leak-check=full --track-origins=yes --show-error-list=yes ./${EXEC_NAME}
        DEPENDS ${EXEC_NAME}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
