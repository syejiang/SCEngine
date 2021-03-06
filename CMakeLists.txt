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

message("CMAKE_SYSTEM: " ${CMAKE_SYSTEM} )
message("Compilers in use : ")
message("CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )
message("CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )

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

if(INCLUDE_DISTRIB)
    add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)


include_directories(SYSTEM
    external/glfw-3.1.2/include/GLFW/
    external/glm-0.9.4.0/
    external/glew-1.9.0/include/
    external/rapidjson/
    external/stb/
    .
)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    GLEW_190    
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
aux_source_directory(${TARGET} APP_SOURCES)

file(GLOB TEMPLATES
    "templates/*.tpp"
)

file(GLOB RESSOURCES
    "${TARGET}/ressources/*.material"
    "${TARGET}/ressources/*/*.material"
    "${TARGET}/ressources/*/*/*.material"

    "${TARGET}/ressources/*.shader"
    "${TARGET}/ressources/*/*.shader"
    "${TARGET}/ressources/*/*/*.shader"

    "${TARGET}/ressources/*.texData"
    "${TARGET}/ressources/*/*.texData"
    "${TARGET}/ressources/*/*/*.texData"
)

file(GLOB SCE_ASSETS
    "${SCE_PATH}/SCE_Assets/*.material"
    "${SCE_PATH}/SCE_Assets/*/*.material"
    "${SCE_PATH}/SCE_Assets/*/*/*.material"

    "${SCE_PATH}/SCE_Assets/*.shader"
    "${SCE_PATH}/SCE_Assets/*/*.shader"
    "${SCE_PATH}/SCE_Assets/*/*/*.shader"

    "${SCE_PATH}/SCE_Assets/*.texData"
    "${SCE_PATH}/SCE_Assets/*/*.texData"
    "${SCE_PATH}/SCE_Assets/*/*/*.texData"
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
    ${TEMPLATES}
    ${RESSOURCES}
    ${SCE_ASSETS}
)
target_link_libraries(${TARGET}
    ${ALL_LIBS}
)

message(${ALL_LIBS})


if(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif(UNIX)
set(QMAKE_CXXFLAGS "${QMAKE_CXXFLAGS} -std=c++11 -Wall -pedantic")
set(QMAKE_LFLAGS "${QMAKE_LFLAGS} -std=c++11 -Wall -pedantic")

#debug and release flag for in-engine use
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSCE_DEBUG -DSCE_DEBUG_ENGINE")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSCE_DEBUG_ENGINE ")
SET(CMAKE_CXX_FLAGS_FINAL "${CMAKE_CXX_FLAGS_RELEASE} -DSCE_FINAL -O3")


file( COPY "${TARGET}/ressources" DESTINATION "." )
file( COPY "${SCE_PATH}/SCE_Assets" DESTINATION "." )
