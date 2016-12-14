cmake_minimum_required(VERSION 3.3)
project(weeaboo)

SET(TOPDIR ${weeaboo_SOURCE_DIR})
SET(CMAKE_LEGACY_CYGWIN_WIN32 0)

#----------------------
# External
#----------------------
OPTION(GLFW_BUILD_DOCS OFF)
OPTION(GLFW_BUILD_EXAMPLES OFF)
OPTION(GLFW_BUILD_TESTS OFF)
ADD_SUBDIRECTORY(${TOPDIR}/ext/glfw)

OPTION(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
OPTION(ASSIMP_BUILD_SAMPLES OFF)
OPTION(ASSIMP_BUILD_TESTS OFF)
ADD_SUBDIRECTORY(${TOPDIR}/ext/assimp)

INCLUDE_DIRECTORIES(${TOPDIR}/include
        ${TOPDIR}/ext/assimp/include
        ${TOPDIR}/ext/glad/include
        ${TOPDIR}/ext/glfw/include
        ${TOPDIR}/ext/glm
        ${TOPDIR}/ext/stb)

IF(${UNIX})
    ADD_DEFINITIONS(-D_UNIX_)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(GLAD_LIBRARIES dl)
ENDIF(${UNIX})

IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    ADD_DEFINITIONS(-D_LINUX_)
ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

IF(${WIN32})
    ADD_DEFINITIONS(-D_WIN32_)
ENDIF(${WIN32})

IF(${CYGWIN})
    ADD_DEFINITIONS(-D_CYGWIN_)
ENDIF(${CYGWIN})

SET(EXT_SRCLIST
        ${TOPDIR}/ext/glad/src/glad.c)

SET(WEEABOO_SRCLIST
        src/main.cpp
        src/material.cpp
        src/mesh.cpp
        src/shader_program.cpp)

SET(LIBRARIES
    assimp glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} pthread)

ADD_EXECUTABLE(weeaboo ${WEEABOO_SRCLIST} ${EXT_SRCLIST})
TARGET_LINK_LIBRARIES(weeaboo ${LIBRARIES})

