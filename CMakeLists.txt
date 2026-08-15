cmake_minimum_required(VERSION 3.20)

project(
    funny_game
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

if(ANDROID)
    set(PLATFORM Android CACHE STRING "raylib target platform" FORCE)
endif()

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

# The prototype does not need raylib's examples or extra tools.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

set(GAME_SOURCES
    src/main.cpp
    src/game/Animation.cpp
    src/game/AssetLocator.cpp
    src/game/BackgroundMusic.cpp
    src/game/BambooVillageLevel.cpp
    src/game/CharacterAction.cpp
    src/game/CharacterFacing.cpp
    src/game/Chicken.cpp
    src/game/Dialogue.cpp
    src/game/Elder.cpp
    src/game/FaceLibrary.cpp
    src/game/FaceRenderer.cpp
    src/game/Game.cpp
    src/game/GameInput.cpp
    src/game/LevelManager.cpp
    src/game/Player.cpp
    src/game/Quest.cpp
    src/game/SoundEffects.cpp
    src/game/SystemDog.cpp
    src/game/TextureAsset.cpp
    src/game/Ui.cpp
)

if(ANDROID)
    add_library(funny-game SHARED ${GAME_SOURCES})
    set_target_properties(funny-game PROPERTIES OUTPUT_NAME main)
else()
    add_executable(funny-game ${GAME_SOURCES})
endif()

target_include_directories(
    funny-game
    PRIVATE
    include
)

target_link_libraries(
    funny-game
    PRIVATE
    raylib
)

target_compile_features(funny-game PRIVATE cxx_std_20)

option(HA_NHAN_MOBILE_UI_PREVIEW "Show and enable Android touch controls on desktop" OFF)
if(HA_NHAN_MOBILE_UI_PREVIEW)
    target_compile_definitions(funny-game PRIVATE HA_NHAN_MOBILE_UI_PREVIEW=1)
endif()

if(MSVC)
    target_compile_options(funny-game PRIVATE /utf-8)
endif()

if(ANDROID)
    # raylib compiles android_native_app_glue.c into libraylib.a, which defines
    # ANativeActivity_onCreate. Because no game source references it directly the
    # linker would strip the object; -u forces it to be kept in libmain.so.
    target_link_options(funny-game PRIVATE -u ANativeActivity_onCreate)
endif()

if(MINGW)
    # Keep the Windows demo portable: no MinGW runtime DLLs beside the exe.
    target_link_options(funny-game PRIVATE -static -static-libgcc -static-libstdc++ -mwindows)
endif()
