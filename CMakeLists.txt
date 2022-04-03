CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT("TD2 OpenCL" VERSION 1.0.0 LANGUAGES CXX)

# Variable for the executable name & additional OpenCL file to be copied inside the build directory
SET(VANILLA_NAME main)
SET(OPENCL_NAME opencl)
SET(others MNT.txt MIN.txt kernel.cl)

# Pass variable from CMake to the code through a header file
SET(filename MNT.txt)
SET(program kernel.cl)

# Add compile flags and VERBOSE option depending on the compilation type
IF(${CMAKE_BUILD_TYPE} MATCHES Debug)
    OPTION(VERBOSE "Print verbose messages" ON)
    ADD_COMPILE_OPTIONS(-g -O0 -Wall -Wextra -pedantic)
ELSE()
    OPTION(VERBOSE "Print verbose messages" OFF)
    ADD_COMPILE_OPTIONS(-O3)
ENDIF()

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
SET(CMAKE_CXX_STANDARD 11)

# Use FindOpenCL through FIND_PACKAGE
FIND_PACKAGE(OpenCL REQUIRED)

# Add all cpp files to the "SOURCES" variable
FILE(GLOB VANILLA_SOURCES main.cpp)
FILE(GLOB OPENCL_SOURCES opencl.cpp)

# Copy each file inside "others" array to the build directory
FOREACH(FILE IN LISTS others)
    CONFIGURE_FILE(
            ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
            ${CMAKE_CURRENT_BINARY_DIR} COPYONLY
    )
ENDFOREACH()

# Create the executable rule
ADD_EXECUTABLE(${VANILLA_NAME} ${VANILLA_SOURCES})

# Create the executable rule
ADD_EXECUTABLE(${OPENCL_NAME} ${OPENCL_SOURCES})
# Link to OpenCL libraries through cmake 3.1 targets
TARGET_LINK_LIBRARIES(${OPENCL_NAME} OpenCL::OpenCL)

ADD_CUSTOM_TARGET(
        Valgrind
        COMMAND valgrind --leak-check=full --track-origins=yes --show-error-list=yes ./${OPENCL_NAME}
        DEPENDS ${OPENCL_NAME}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
