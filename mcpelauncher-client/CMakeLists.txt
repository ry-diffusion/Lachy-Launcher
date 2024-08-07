cmake_minimum_required (VERSION 2.6)

project(mcpelauncher-client LANGUAGES CXX ASM)

git_commit_hash(${CMAKE_CURRENT_SOURCE_DIR} CLIENT_GIT_COMMIT_HASH)
configure_file(src/build_info.h.in ${CMAKE_CURRENT_BINARY_DIR}/build_info/build_info.h)

add_executable(mcpelauncher-client src/main.cpp src/OpenSLESPatch.cpp src/OpenSLESPatch.h src/jni/HardwareInformation.cpp src/jni/Http.cpp src/jni/InputManager.cpp src/jni/MainActivity.cpp src/jni/Misc.cpp src/jni/Store.cpp src/jni/XboxLive.cpp src/window_callbacks.cpp src/window_callbacks.h src/xbox_live_helper.cpp src/xbox_live_helper.h src/minecraft_gamepad_mapping.h src/cll_upload_auth_step.cpp src/cll_upload_auth_step.h src/utf8_util.h src/xbox_live_game_interface.cpp src/xbox_live_game_interface.h src/hbui_patch.cpp src/hbui_patch.h)
target_link_libraries(mcpelauncher-client jnivm logger mcpelauncher-core gamewindow filepicker msa-daemon-client cll-telemetry argparser)
target_include_directories(mcpelauncher-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_BINARY_DIR}/build_info/)

if (APPLE)
    set_target_properties(mcpelauncher-client PROPERTIES MACOSX_RPATH TRUE INSTALL_RPATH @executable_path/../Frameworks)
endif()
if (IS_ARM_BUILD)
    target_link_libraries(mcpelauncher-client -Wl,${CMAKE_CURRENT_SOURCE_DIR}/../mcpelauncher-linux-bin/libs/native/libfmod.so.9.16)
    set(MCPELAUNCHER_RPATH \$ORIGIN/../share/mcpelauncher/libs/native/)
    if (ENABLE_DEV_PATHS)
        set(MCPELAUNCHER_RPATH ${NATIVES_PATH_DIR}/libs/native:${MCPELAUNCHER_RPATH})
    endif()
    set_target_properties(mcpelauncher-client PROPERTIES INSTALL_RPATH ${MCPELAUNCHER_RPATH})
endif()
if (IS_ARMHF_BUILD)
    target_sources(mcpelauncher-client PRIVATE src/armhf_support.cpp src/armhf_support.h)
    target_compile_definitions(mcpelauncher-client PRIVATE USE_ARMHF_SUPPORT)
else()
    target_sources(mcpelauncher-client PRIVATE src/cpuid.cpp src/cpuid.h)
endif()

install(TARGETS mcpelauncher-client RUNTIME COMPONENT mcpelauncher-client DESTINATION bin)
include(CPackSettings.cmake)

add_dependencies(mcpelauncher-client MoreInfos)
