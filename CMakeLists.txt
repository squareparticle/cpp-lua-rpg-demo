cmake_minimum_required(VERSION 3.24)

project(RPG2002-Ship LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)

# The game only needs the yaml-cpp library. Its optional command-line tools
# otherwise appear in Visual Studio as runnable targets such as parse.exe.
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Build yaml-cpp command-line tools" FORCE)

# yaml-cpp 0.8.0 predates CMake 4, which requires an explicit policy floor
# for dependencies whose cmake_minimum_required version is older than 3.5.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(yaml-cpp)
unset(CMAKE_POLICY_VERSION_MINIMUM)

FetchContent_Declare(
    lua
    URL https://www.lua.org/ftp/lua-5.5.1.tar.gz
    URL_HASH SHA256=1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce
    SOURCE_SUBDIR no-cmake-project
)
FetchContent_MakeAvailable(lua)

file(GLOB lua_sources CONFIGURE_DEPENDS "${lua_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM lua_sources
    "${lua_SOURCE_DIR}/src/lua.c"
    "${lua_SOURCE_DIR}/src/luac.c"
    "${lua_SOURCE_DIR}/src/onelua.c"
)
add_library(lua STATIC ${lua_sources})
set_source_files_properties(${lua_sources} PROPERTIES LANGUAGE CXX)
target_include_directories(lua PUBLIC "${lua_SOURCE_DIR}/src")
target_compile_definitions(lua PRIVATE LUA_USE_WINDOWS)
target_compile_features(lua PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(lua PRIVATE /W0)
endif()

add_executable(RPG2002-Ship WIN32
    src/Application/main.cpp
    src/Application/AppConfig.cpp
    src/Application/AppConfig.h
    src/Application/Application.cpp
    src/Application/Application.h
    src/Platform/Input/DeviceInput.cpp
    src/Platform/Input/DeviceInput.h
    src/Game/GameController.cpp
    src/Game/GameController.h
    src/Game/Input/InputActions.h
    src/Game/Input/InputState.h
    src/Game/Effects/WeatherSystem.cpp
    src/Game/Effects/WeatherSystem.h
    src/Framework/Rendering/Camera.cpp
    src/Framework/Rendering/Camera.h
    src/Platform/DebugLog.cpp
    src/Platform/DebugLog.h
    src/Framework/Objects/BaseGameObject.cpp
    src/Framework/Objects/BaseGameObject.h
    src/Framework/Layer/BaseLayer.cpp
    src/Framework/Layer/BaseLayer.h
    src/Framework/Scenes/BaseScene.cpp
    src/Framework/Scenes/BaseScene.h
    src/Platform/Graphics.cpp
    src/Platform/Graphics.h
    src/Platform/Audio/Audio.cpp
    src/Platform/Audio/Audio.h
    src/Platform/SoundAsset.cpp
    src/Platform/SoundAsset.h
    src/Game/World/GroundRegionMap.cpp
    src/Game/World/GroundRegionMap.h
    src/Framework/Collision/CollisionMask.cpp
    src/Framework/Collision/CollisionMask.h
    src/Platform/ImageAsset.cpp
    src/Platform/ImageAsset.h
    src/Framework/Rendering/SpriteAnimator.cpp
    src/Framework/Rendering/SpriteAnimator.h
    src/Platform/SpriteAsset.cpp
    src/Platform/SpriteAsset.h
    src/Framework/Resources/ResourceManager.cpp
    src/Framework/Resources/ResourceManager.h
    src/Game/Scenes/SceneBuilder.cpp
    src/Game/Scenes/SceneBuilder.h
    src/Game/Objects/Player.h
    src/Game/Objects/Player.cpp
    src/Game/Objects/PlayerLoad.cpp
    src/Game/Objects/BattleEnemy.cpp
    src/Game/Objects/BattleEnemy.h
    src/Game/Objects/BattleEnemyLoad.cpp
    src/Game/Objects/BattleObjectLoad.cpp
    src/Game/Objects/BattleObjectLoad.h
    src/Game/Battle/BattleStats.h
    src/Game/Objects/PlayerBattle.cpp
    src/Game/Objects/PlayerBattle.h
    src/Game/Objects/PlayerBattleLoad.cpp
    src/Game/Objects/GameObjectLoad.h
    src/Game/Objects/SpriteObject.cpp
    src/Game/Objects/SpriteObject.h
    src/Game/Objects/SpriteObjectLoad.cpp
    src/Game/Objects/InteractableObject.cpp
    src/Game/Objects/InteractableObject.h
    src/Game/Objects/InteractableObjectLoad.cpp
    src/Game/Objects/TriggerRegion.cpp
    src/Game/Objects/TriggerRegion.h
    src/Game/Objects/ScrollingSurface.cpp
    src/Game/Objects/ScrollingSurface.h
    src/Game/Objects/ScrollingSurfaceLoad.cpp
    src/Framework/Layer/ImageLayer.cpp
    src/Framework/Layer/ImageLayer.h
    src/Framework/Layer/ObjectLayer.cpp
    src/Framework/Layer/ObjectLayer.h
    src/Game/Scenes/BoatScene.cpp
    src/Game/Scenes/BoatScene.h
    src/Game/Scenes/BattleArenaScene.cpp
    src/Game/Scenes/BattleArenaScene.h
    src/Game/Scenes/StorageRoomScene.cpp
    src/Game/Scenes/StorageRoomScene.h
    src/Game/Scenes/WorldScene.cpp
    src/Game/Scenes/WorldScene.h
    src/Game/Scenes/WorldSceneCutscene.cpp
    src/Game/Scenes/EndingScene.cpp
    src/Game/Scenes/EndingScene.h
    src/Game/State/GlobalState.cpp
    src/Game/State/GlobalState.h
    src/Game/State/Inventory.cpp
    src/Game/State/Inventory.h
    src/Game/Items/ItemDefinition.h
    src/Game/Items/ItemDefinitionLoad.cpp
    src/Game/Items/ItemDefinitionLoad.h
    src/Game/State/PlayerState.cpp
    src/Game/State/PlayerState.h
    src/Game/State/SaveGame.cpp
    src/Game/State/SaveGame.h
    src/Game/Battle/BattleDecision.h
    src/Game/Battle/BattleRequest.h
    src/Game/Battle/BattleMove.h
    src/Game/Battle/BattleMoveLoad.cpp
    src/Game/Battle/BattleMoveLoad.h
    src/Game/Battle/BattleRules.cpp
    src/Game/Battle/BattleRules.h
    src/Framework/Scenes/SceneChangeRequest.h
    src/Game/World/GroundRegionSceneChange.cpp
    src/Game/World/GroundRegionSceneChange.h
    src/Game/Scripting/LuaSystem.cpp
    src/Game/Scripting/LuaSystem.h
    src/Framework/Objects/ObjectHook.cpp
    src/Framework/Objects/ObjectHook.h
    src/Game/Scripting/RuleTable.cpp
    src/Game/Scripting/RuleTable.h
    src/Game/UI/DialogueWindow.cpp
    src/Game/UI/DialogueWindow.h
    src/Game/UI/DialogueWindowRender.cpp
    src/Game/UI/InventoryMenu.cpp
    src/Game/UI/InventoryMenu.h
    src/Game/UI/InventoryMenuRender.cpp
    src/Game/UI/MainMenu.cpp
    src/Game/UI/MainMenu.h
    src/Game/UI/MainMenuRender.cpp
    src/Game/UI/StatusMenu.cpp
    src/Game/UI/StatusMenu.h
    src/Game/UI/StatusMenuRender.cpp
    src/Game/UI/BattleMenu.cpp
    src/Game/UI/BattleMenu.h
    src/Game/UI/BattleMenuRender.cpp
    src/Game/UI/RpgMenuStyle.cpp
    src/Game/UI/RpgMenuStyle.h
)

target_compile_features(RPG2002-Ship PRIVATE cxx_std_20)
target_compile_definitions(RPG2002-Ship PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MSVC)
    target_compile_options(RPG2002-Ship PRIVATE /W4 /permissive-)
endif()

target_link_libraries(RPG2002-Ship
    PRIVATE
        d2d1
        dwrite
        windowscodecs
        xaudio2
        lua
        yaml-cpp::yaml-cpp
)

include(CTest)
if(BUILD_TESTING)
    # Exercise actual scene updates and rendering with an in-memory WIC target;
    # this test never creates a window or initializes audio playback.
    get_target_property(world_cutscene_sources RPG2002-Ship SOURCES)
    list(FILTER world_cutscene_sources EXCLUDE REGEX "^src/Application/")
    add_executable(WorldCutsceneTests tests/WorldCutsceneTests.cpp
        ${world_cutscene_sources})
    target_compile_features(WorldCutsceneTests PRIVATE cxx_std_20)
    target_compile_definitions(WorldCutsceneTests PRIVATE
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    if(MSVC)
        target_compile_options(WorldCutsceneTests PRIVATE /W4 /permissive-)
    endif()
    target_link_libraries(WorldCutsceneTests PRIVATE
        d2d1 dwrite windowscodecs xaudio2 ole32 lua yaml-cpp::yaml-cpp)
    add_test(NAME WorldCutsceneTests COMMAND WorldCutsceneTests)
    set_tests_properties(WorldCutsceneTests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" SKIP_RETURN_CODE 77)

    add_executable(SoundAssetTests
        tests/SoundAssetTests.cpp
        src/Platform/SoundAsset.cpp
        src/Platform/SoundAsset.h
    )
    target_compile_features(SoundAssetTests PRIVATE cxx_std_20)
    target_compile_definitions(SoundAssetTests PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    if(MSVC)
        target_compile_options(SoundAssetTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME SoundAssetTests COMMAND SoundAssetTests)

    add_executable(DeviceInputTests
        tests/DeviceInputTests.cpp
        src/Platform/Input/DeviceInput.cpp
        src/Platform/Input/DeviceInput.h
    )
    target_compile_features(DeviceInputTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(DeviceInputTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME DeviceInputTests COMMAND DeviceInputTests)

    add_executable(AppConfigTests
        tests/AppConfigTests.cpp
        src/Application/AppConfig.cpp
        src/Application/AppConfig.h
    )
    target_compile_features(AppConfigTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(AppConfigTests PRIVATE /W4 /permissive-)
    endif()
    target_link_libraries(AppConfigTests PRIVATE yaml-cpp::yaml-cpp)
    add_test(NAME AppConfigTests COMMAND AppConfigTests)

    add_executable(RuleTableTests
        tests/RuleTableTests.cpp
        src/Platform/DebugLog.cpp
        src/Platform/DebugLog.h
        src/Game/World/GroundRegionMap.cpp
        src/Game/World/GroundRegionMap.h
        src/Game/State/GlobalState.cpp
        src/Game/State/GlobalState.h
        src/Game/State/Inventory.cpp
        src/Game/State/Inventory.h
        src/Game/Scripting/RuleTable.cpp
        src/Game/Scripting/RuleTable.h
        src/Game/Scripting/LuaSystem.cpp
        src/Game/Scripting/LuaSystem.h
        src/Game/UI/DialogueWindow.cpp
        src/Game/UI/DialogueWindow.h
    )
    target_compile_features(RuleTableTests PRIVATE cxx_std_20)
    target_compile_definitions(RuleTableTests PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    if(MSVC)
        target_compile_options(RuleTableTests PRIVATE /W4 /permissive-)
    endif()
    target_link_libraries(RuleTableTests PRIVATE lua yaml-cpp::yaml-cpp)
    add_test(NAME RuleTableTests COMMAND RuleTableTests)
    set_tests_properties(RuleTableTests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    add_executable(CameraTests
        tests/CameraTests.cpp
        src/Framework/Rendering/Camera.cpp
        src/Framework/Rendering/Camera.h
    )
    target_compile_features(CameraTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(CameraTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME CameraTests COMMAND CameraTests)

    add_executable(DialogueWindowTests
        tests/DialogueWindowTests.cpp
        src/Game/UI/DialogueWindow.cpp
        src/Game/UI/DialogueWindow.h
    )
    target_compile_features(DialogueWindowTests PRIVATE cxx_std_20)
    target_compile_definitions(DialogueWindowTests PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    if(MSVC)
        target_compile_options(DialogueWindowTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME DialogueWindowTests COMMAND DialogueWindowTests)

    add_executable(MainMenuTests
        tests/MainMenuTests.cpp
        src/Game/State/Inventory.cpp
        src/Game/State/Inventory.h
        src/Game/Items/ItemDefinition.h
        src/Game/State/PlayerState.cpp
        src/Game/State/PlayerState.h
        src/Game/Input/InputState.h
        src/Game/UI/InventoryMenu.cpp
        src/Game/UI/InventoryMenu.h
        src/Game/UI/MainMenu.cpp
        src/Game/UI/MainMenu.h
        src/Game/UI/StatusMenu.cpp
        src/Game/UI/StatusMenu.h
    )
    target_compile_features(MainMenuTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(MainMenuTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME MainMenuTests COMMAND MainMenuTests)

    add_executable(SaveGameTests
        tests/SaveGameTests.cpp
        src/Game/State/GlobalState.cpp
        src/Game/State/GlobalState.h
        src/Game/State/Inventory.cpp
        src/Game/State/Inventory.h
        src/Game/State/PlayerState.cpp
        src/Game/State/PlayerState.h
        src/Game/Battle/BattleStats.h
        src/Game/State/SaveGame.cpp
        src/Game/State/SaveGame.h
    )
    target_compile_features(SaveGameTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(SaveGameTests PRIVATE /W4 /permissive-)
    endif()
    target_link_libraries(SaveGameTests PRIVATE yaml-cpp::yaml-cpp)
    add_test(NAME SaveGameTests COMMAND SaveGameTests)

    add_executable(BattleMenuTests
        tests/BattleMenuTests.cpp
        src/Game/UI/BattleMenu.cpp
        src/Game/UI/BattleMenu.h
        src/Game/Battle/BattleStats.h
        src/Game/Input/InputState.h
    )
    target_compile_features(BattleMenuTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(BattleMenuTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME BattleMenuTests COMMAND BattleMenuTests)

    add_executable(BattleRulesTests
        tests/BattleRulesTests.cpp
        src/Platform/DebugLog.cpp
        src/Platform/DebugLog.h
        src/Game/Battle/BattleMove.h
        src/Game/Battle/BattleMoveLoad.cpp
        src/Game/Battle/BattleMoveLoad.h
        src/Game/Battle/BattleRules.cpp
        src/Game/Battle/BattleRules.h
        src/Game/Items/ItemDefinition.h
        src/Game/Items/ItemDefinitionLoad.cpp
        src/Game/Items/ItemDefinitionLoad.h
        src/Game/State/PlayerState.cpp
        src/Game/State/PlayerState.h
        src/Game/Objects/BattleObjectLoad.cpp
        src/Game/Objects/BattleObjectLoad.h
        src/Game/Battle/BattleStats.h
    )
    target_compile_features(BattleRulesTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(BattleRulesTests PRIVATE /W4 /permissive-)
    endif()
    target_link_libraries(BattleRulesTests PRIVATE yaml-cpp::yaml-cpp)
    add_test(NAME BattleRulesTests COMMAND BattleRulesTests)
    set_tests_properties(BattleRulesTests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    add_executable(TriggerRegionTests
        tests/TriggerRegionTests.cpp
        src/Game/Objects/TriggerRegion.cpp
        src/Game/Objects/TriggerRegion.h
    )
    target_compile_features(TriggerRegionTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(TriggerRegionTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME TriggerRegionTests COMMAND TriggerRegionTests)

    add_executable(GroundRegionMapTests
        tests/GroundRegionMapTests.cpp
        src/Game/World/GroundRegionMap.cpp
        src/Game/World/GroundRegionMap.h
    )
    target_compile_features(GroundRegionMapTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(GroundRegionMapTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME GroundRegionMapTests COMMAND GroundRegionMapTests)

    add_executable(SceneChangeRequestTests
        tests/SceneChangeRequestTests.cpp
        src/Framework/Scenes/SceneChangeRequest.h
        src/Game/World/GroundRegionSceneChange.cpp
        src/Game/World/GroundRegionSceneChange.h
        src/Game/World/GroundRegionMap.h
    )
    target_compile_features(SceneChangeRequestTests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(
            SceneChangeRequestTests PRIVATE /W4 /permissive-)
    endif()
    add_test(
        NAME SceneChangeRequestTests
        COMMAND SceneChangeRequestTests)
endif()

add_custom_target(RPG2002-Ship-copy-assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:RPG2002-Ship>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:RPG2002-Ship>/assets"
    VERBATIM
)

add_custom_target(RPG2002-Ship-copy-config ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:RPG2002-Ship>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.yaml"
        "$<TARGET_FILE_DIR:RPG2002-Ship>/config.yaml"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/game.yaml"
        "$<TARGET_FILE_DIR:RPG2002-Ship>/game.yaml"
    DEPENDS
        "${CMAKE_SOURCE_DIR}/config.yaml"
        "${CMAKE_SOURCE_DIR}/game.yaml"
    VERBATIM
)
