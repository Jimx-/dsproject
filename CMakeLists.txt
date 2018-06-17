cmake_minimum_required(VERSION 3.3)
project(dsproject)

SET(TOPDIR ${dsproject_SOURCE_DIR})
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

OPTION(BUILD_BULLET2_DEMOS OFF)
OPTION(BUILD_CPU_DEMOS OFF)
OPTION(BUILD_EXTRAS OFF)
OPTION(BUILD_OPENGL3_DEMOS OFF)
OPTION(BUILD_UNIT_TESTS OFF)
ADD_SUBDIRECTORY(${TOPDIR}/ext/bullet)

EXECUTE_PROCESS(COMMAND python amalgamate.py WORKING_DIRECTORY ${TOPDIR}/ext/jsoncpp)

FIND_PACKAGE(Freetype REQUIRED)

INCLUDE_DIRECTORIES(${TOPDIR}/include
        ${TOPDIR}/ext/assimp/include
        ${TOPDIR}/ext/glad/include
        ${TOPDIR}/ext/glfw/include
        ${TOPDIR}/ext/bullet/src
        ${TOPDIR}/ext/glm
        ${TOPDIR}/ext/stb
        ${TOPDIR}/ext/jsoncpp/dist
        ${FREETYPE_INCLUDE_DIRS})

IF(${UNIX})
    ADD_DEFINITIONS(-D_UNIX_)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(GLAD_LIBRARIES dl pthread)
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
        ${TOPDIR}/ext/glad/src/glad.c
        ${TOPDIR}/ext/jsoncpp/dist/jsoncpp.cpp)

SET(DSPROJECT_SRCLIST
        src/log.cpp
        src/log_manager.cpp
        src/main.cpp
        src/material.cpp
        src/mesh.cpp
        src/shader_program.cpp
        src/renderer.cpp
        src/intern_string.cpp
        src/config.cpp
        src/string_utils.cpp
        src/exception.cpp
        src/animation.cpp
        src/animation_manager.cpp
        src/map.cpp
        src/map_generator.cpp
        src/random_utils.cpp
        src/camera.cpp
        src/characters.cpp
        src/character_manager.cpp
		src/particle.cpp
		src/particle_system.cpp
        src/simulation.cpp
        src/motion_state.cpp
        src/text_overlay.cpp
        src/controllers.cpp
        src/gui.cpp)

SET(LIBRARIES
    assimp glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} BulletDynamics BulletCollision LinearMath ${FREETYPE_LIBRARIES})

ADD_EXECUTABLE(dsproject ${DSPROJECT_SRCLIST} ${EXT_SRCLIST})
TARGET_LINK_LIBRARIES(dsproject ${LIBRARIES})

