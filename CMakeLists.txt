#***************************************#
#***************************************#
#** CMakeLists for Sand Castle Engine **#
#******** Author : Gwenn AUBERT ********#
#***************************************#
#***************************************#

#CMake Debug Line
#-DCMAKE_BUILD_TYPE=Debug



# CMake entry point
cmake_minimum_required (VERSION 2.8)
project (SCEngine)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()


# Compile external dependencies (needs a cMakeList)
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-1c73e35")
#include(CreateLaunchers)
#include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
    add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)


include_directories(SYSTEM
#    external/AntTweakBar-1.16/include/
    external/glfw-3.0.3/include/GLFW/
    external/glm-0.9.4.0/
    external/glew-1.9.0/include/
#    external/assimp-3.0.1270/include/
    external/bullet-2.81-rev2613/src/
    external/rapidjson/
#    ${CMAKE_CURRENT_SOURCE_DIR}/external/SOIL/
    .
)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    GLFW_303
    GLEW_190
    #SOIL
)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

set( TARGET playground)
set( SCE_PATH .)

file(GLOB HEADERS
    "headers/*.hpp"
)

aux_source_directory(./external/glm-0.9.4.0/ EXT)
aux_source_directory(./sources SOURCES)
aux_source_directory(./common COMMONS)
aux_source_directory(${TARGET} APP_SOURCES)

file(GLOB TEMPLATES
    "templates/*.tpp"
)

file(GLOB RESSOURCES
    "${TARGET}/ressources/*.shader"
    "${TARGET}/ressources/*.material"
    "${TARGET}/ressources/*.texData"
)

file(GLOB SCE_ASSETS
    "${SCE_PATH}/SCE_Assets/*"
)

file(GLOB GLM_HEADERS
    "${SCE_PATH}/external/glm-0.9.4.0/glm/*"
)



# User playground
add_executable(${TARGET}
    ${APP_SOURCES}
    ${HEADERS}
    ${SOURCES}
    ${GLM_HEADERS}
    ${COMMONS}
    ${TEMPLATES}
    ${RESSOURCES}
    ${SCE_ASSETS}
)
target_link_libraries(${TARGET}
    ${ALL_LIBS}
)

target_link_libraries(${TARGET}
#    ${CMAKE_SOURCE_DIR}/external/SOIL/libSOIL.a
    SOIL
)

message("all libs : ")
message(${ALL_LIBS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wno-variadic-macros -Wunused-local-typedefs -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-long-long -Wno-variadic-macros -pedantic")
set(QMAKE_CXXFLAGS "${QMAKE_CXXFLAGS} -std=c++11")
set(QMAKE_LFLAGS "${QMAKE_LFLAGS} -std=c++11")

# Xcode and Visual working directories
#set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/")
#create_target_launcher(${TARGET} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/")

#message("${CMAKE_CURRENT_SOURCE_DIR}/playground/")


#SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
#SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader*" )


#if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
#add_custom_command(
#   TARGET ${TARGET} POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${TARGET}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/"
#)


#elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

#endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

file( COPY "${TARGET}/ressources" DESTINATION "." )
file( COPY "${SCE_PATH}/SCE_Assets" DESTINATION "." )