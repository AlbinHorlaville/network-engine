cmake_minimum_required(VERSION 3.29.6)
project(MagnumBootstrap)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

add_subdirectory(externals)
find_package(Magnum REQUIRED
        GL
        GlfwApplication
        Primitives
        Shaders
        Trade
        MeshTools
        Primitives
        SceneGraph)
find_package(MagnumIntegration REQUIRED ImGui Bullet)
find_package(Bullet REQUIRED Dynamics)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

file(GLOB_RECURSE COMMON_SOURCES src/components/*.cpp src/entities/*.cpp src/systems/*.cpp)
message(STATUS "COMMON_SOURCES: ${COMMON_SOURCES}")

# --- Executable Serveur ---
add_executable(Serveur
        ${COMMON_SOURCES}
        src/server/server.cpp
)

# --- Executable Client ---
add_executable(Client
        ${COMMON_SOURCES}
        src/client/client.cpp
)

foreach(target IN ITEMS Serveur Client)
    target_include_directories(${target} PUBLIC inc)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)
    target_link_libraries(${target} PRIVATE
            Magnum::Application
            Magnum::GL
            Magnum::Magnum
            Magnum::MeshTools
            Magnum::Primitives
            Magnum::SceneGraph
            Magnum::Shaders
            Magnum::Trade
            MagnumIntegration::Bullet
            Bullet::Dynamics
            MagnumIntegration::ImGui
            BulletDynamics
            BulletCollision
            LinearMath
            Bullet3Common
            enet
    )
    target_include_directories(${target} PRIVATE externals/enet6/include)
endforeach()
