cmake_minimum_required(VERSION 3.10)

project(GiftFromOtherPlanet VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/AudioSystem.cpp
    src/GameLogic.cpp
    src/keyjob.cpp
    src/PhysicsSystem.cpp
    src/Render.cpp
    src/Simulation.cpp
)

set(HEADERS
    src/AudioSystem.h
    src/GameConfig.h
    src/GameLogic.h
    src/keyjob.h
    src/PhysicsSystem.h
    src/Render.h
    src/Simulation.h
    src/SpatialGrid.h
    src/ThreadPool.h
    src/miniaudio.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if(WIN32)
    set(SDL2_DIR "${CMAKE_SOURCE_DIR}/libs/SDL2")
    
    target_include_directories(${PROJECT_NAME} PRIVATE 
        "${SDL2_DIR}/include"
        "${CMAKE_SOURCE_DIR}/src"
    )
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2_LIB_DIR "${SDL2_DIR}/lib/x64")
    else()
        set(SDL2_LIB_DIR "${SDL2_DIR}/lib/x86")
    endif()
    
    target_link_libraries(${PROJECT_NAME} 
        "${SDL2_LIB_DIR}/SDL2.lib"
        "${SDL2_LIB_DIR}/SDL2main.lib"
    )
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_LIB_DIR}/SDL2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/Stellardrone - Eternity.mp3"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${SDL2_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/src"
    )
    
    target_link_libraries(${PROJECT_NAME} 
        ${SDL2_LIBRARIES}
        pthread
    )
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/Stellardrone - Eternity.mp3"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()