#pragma once

#include <mcpelauncher/path_helper.h>

class MinecraftUtils {

private:
    static void* loadLibM();

    static void setupHookApi();

public:
    static void workaroundLocaleBug();

    static void setMallocZero();

    static void setupHybris();

    static void* loadMinecraftLib(std::string const& path = PathHelper::findGameFile("lib/"
#ifdef __i386__
    "x86"
#else
    "armeabi-v7a"
#endif
    "/libminecraftpe.so"));

    static void* loadFMod();
    static void stubFMod();

    static void setupForHeadless();

    static unsigned int getLibraryBase(void* handle);

    static void initSymbolBindings(void* handle);

    static void workaroundShutdownCrash(void* handle);

    static void setupGLES2Symbols(void* (*resolver)(const char*));

};
