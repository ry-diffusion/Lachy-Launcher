#include <argparser.h>
#include <dlfcn.h>
#include <game_window_manager.h>
#include <hybris/dlfcn.h>
#include <hybris/hook.h>
#include <imgui.h>
#include <log.h>
#include <mcpelauncher/crash_handler.h>
#include <mcpelauncher/minecraft_utils.h>
#include <mcpelauncher/minecraft_version.h>
#include <mcpelauncher/mod_loader.h>
#include <mcpelauncher/patch_utils.h>
#include <mcpelauncher/path_helper.h>
#include <minecraft/ClientInstance.h>
#include <minecraft/Common.h>
#include <minecraft/MinecraftGame.h>

#include <functional>
#include <iostream>
#include <unordered_map>

#include "mcpelauncher/app_platform.h"
#include "mcpelauncher/core_mod_loader.h"
static bool isModern = false;
#include "hbui_patch.h"
#include "minecraft/MinecraftClient.h"
#include "window_callbacks.h"
#include "xbox_live_helper.h"
#ifdef USE_ARMHF_SUPPORT
#include "armhf_support.h"
#endif
#ifdef __i386__
#include "cpuid.h"
#endif
#include <build_info.h>
#include <dirent.h>
#include <fcntl.h>
#include <hybris/hook.h>
#include <jnivm.h>
#include <minecraft/GenericMinecraft.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <mutex>

#include "JNIBinding.h"
#include "OpenSLESPatch.h"
#include "config.h"

// Replacement for deprecated ftime() using gettimeofday()
struct timeb
{
  time_t time;
  unsigned short millitm;
  short timezone;
  short dstflag;
};

static int ftime_replacement(struct timeb* tb)
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  tb->time = tv.tv_sec;
  tb->millitm = tv.tv_usec / 1000;
  tb->timezone = 0;
  tb->dstflag = 0;
  return 0;
}
#include "native_activity.h"

// Log verbosity level
static int g_logVerbosity = 1;  // 0=quiet, 1=normal, 2=verbose

// Asset cache configuration
static constexpr size_t ASSET_CACHE_MAX_ENTRIES = 64;
static constexpr size_t ASSET_CACHE_MAX_FILE_SIZE =
    1024 * 1024;  // 1MB per file

// Simple bounded cache (no LRU tracking to avoid hybris compatibility issues)
static std::unordered_map<std::string, std::string> g_assetCache;
static std::mutex g_assetCacheMutex;
static bool g_enableAssetCache = true;
static size_t g_assetCacheHits = 0;
static size_t g_assetCacheMisses = 0;

/**
 * Clear oldest entries if cache exceeds max size
 */
static void trimAssetCache()
{
  if (g_assetCache.size() > ASSET_CACHE_MAX_ENTRIES)
  {
    // Simple strategy: clear half the cache when full
    size_t toRemove = g_assetCache.size() / 2;
    auto it = g_assetCache.begin();
    while (toRemove > 0 && it != g_assetCache.end())
    {
      it = g_assetCache.erase(it);
      toRemove--;
    }
  }
}

#define EGL_NONE 0x3038
#define EGL_TRUE 1
#define EGL_FALSE 0
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3056
using EGLint = int;
using EGLDisplay = void*;
using EGLSurface = void*;
using EGLContext = void*;
using EGLConfig = void*;
using NativeWindowType = void*;
using NativeDisplayType = void*;

GenericMinecraft* minecraftClient = nullptr;
JNIEnv* jnienv = nullptr;

void printVersionInfo();

mcpe::string getEdition()
{
  return "win10";
}

int getPlatformUIScalingRules()
{
  return 0;
}

int getScreenType()
{
  return 0;
}

int getPlatformType()
{
  return 0;
}

bool useCenteredGUI()
{
  return true;
}

mcpe::string getSetttingsPath()
{
  return PathHelper::getPrimaryDataDirectory();
}

/**
 * Thanks for D4yvid and BlockLauncher
 */
void patchDesktopUi(PatchUtils::VtableReplaceHelper vtr)
{
  Log::info("DesktopUI", "Using Windows 10 Edition UI...");

  vtr.replace("_ZNK11AppPlatform15getPlatformTypeEv", &getPlatformType);
  vtr.replace("_ZNK11AppPlatform14useCenteredGUIEv", &useCenteredGUI);
  vtr.replace("_ZNK19AppPlatform_android25getPlatformUIScalingRulesEv",
              &getPlatformUIScalingRules);
  vtr.replace("_ZNK19AppPlatform_android13getScreenTypeEv", &getScreenType);
  vtr.replace("_ZNK19AppPlatform_android10getEditionEv", &getEdition);
}

/**
 * Thx Blocklauncher
 */
void patchFixSettingsPath(PatchUtils::VtableReplaceHelper vtr)
{
  vtr.replace("_ZN11AppPlatform15getSettingsPathEv", &getSetttingsPath);
  vtr.replace("_ZN19AppPlatform_android15getSettingsPathEv", &getSetttingsPath);
}

/**
 * Read asset file for legacy versions (pre-0.16)
 * The game passes a relative path, we prepend the assets directory
 * Implements simple bounded caching for frequently accessed files
 */
mcpe::string readAssetFileLegacy(void* self, mcpe::string const& path)
{
  // Cache the assets directory to avoid repeated lookups
  static std::string cachedAssetsDir;
  if (cachedAssetsDir.empty())
  {
    cachedAssetsDir = PathHelper::getGameDir() + "assets/";
  }

  std::string pathStr = path.std();

  // Check cache first. Minecraft may call readAssetFile from multiple worker
  // threads, so all access to the shared unordered_map must be serialized.
  if (g_enableAssetCache)
  {
    std::lock_guard<std::mutex> lock(g_assetCacheMutex);

    auto it = g_assetCache.find(pathStr);
    if (it != g_assetCache.end())
    {
      g_assetCacheHits++;
      return mcpe::string(it->second.c_str(), it->second.size());
    }
    g_assetCacheMisses++;
  }

  std::string fullPath = cachedAssetsDir + pathStr;
  int fd = open(fullPath.c_str(), O_RDONLY);
  if (fd < 0)
  {
    if (g_logVerbosity >= 2)
      Log::error("readAssetFileLegacy", "not found: %s", fullPath.c_str());
    return mcpe::string();
  }
  struct stat sr;
  if (fstat(fd, &sr) < 0 || (sr.st_mode & S_IFDIR))
  {
    close(fd);
    return mcpe::string();
  }
  off_t size = sr.st_size;

  std::string buffer;
  buffer.resize((size_t)size);

  ssize_t bytesRead = read(fd, &buffer[0], size);
  if (bytesRead < 0)
  {
    Log::error("readAssetFileLegacy", "read error");
    close(fd);
    return mcpe::string();
  }

  while (bytesRead < size)
  {
    ssize_t res = read(fd, &buffer[0] + bytesRead, size - bytesRead);
    if (res <= 0)
    {
      Log::error("readAssetFileLegacy", "read error");
      close(fd);
      return mcpe::string();
    }
    bytesRead += res;
  }
  close(fd);

  // Cache small files with simple trimming
  if (g_enableAssetCache &&
      static_cast<size_t>(size) < ASSET_CACHE_MAX_FILE_SIZE)
  {
    std::lock_guard<std::mutex> lock(g_assetCacheMutex);

    trimAssetCache();
    g_assetCache[pathStr] = buffer;
  }

  return mcpe::string(buffer.c_str(), buffer.size());
}

mcpe::string getImagePathLegacy(void* self, mcpe::string const& s, int loc)
{
  // Cache the assets directory
  static std::string cachedAssetsDir;
  if (cachedAssetsDir.empty())
  {
    cachedAssetsDir = PathHelper::getGameDir() + "assets/";
  }

  if (loc == 0)
    return cachedAssetsDir + "images/" + s.std();
  else
    return cachedAssetsDir + s.std();
}

void patchReadAssetFileLegacy(PatchUtils::VtableReplaceHelper vtr, void* handle)
{
  // Read version directly from the library since initSymbolBindings hasn't been
  // called yet
  int* majorVersion =
      (int*)hybris_dlsym(handle, "_ZN15SharedConstants12MajorVersionE");
  int* minorVersion =
      (int*)hybris_dlsym(handle, "_ZN15SharedConstants12MinorVersionE");
  // Legacy versions are 0.x where x < 16 (i.e., 0.15.x and earlier)
  // Version 1.x and above are NOT legacy
  bool isLegacy = !majorVersion || !minorVersion ||
                  (*majorVersion == 0 && *minorVersion < 16);

  Log::trace("readAssetFileLegacy",
             "Version check: major=%d minor=%d isLegacy=%d",
             majorVersion ? *majorVersion : -1,
             minorVersion ? *minorVersion : -1, isLegacy);

  if (isLegacy)
  {
    Log::info("readAssetFileLegacy",
              "Patching readAssetFile for legacy version");
    vtr.replace("_ZN19AppPlatform_android13readAssetFileERKSs",
                &readAssetFileLegacy);
    vtr.replace("_ZN19AppPlatform_android12getImagePathERKSs15TextureLocation",
                &getImagePathLegacy);
  }
}

#ifdef JNI_DEBUG
void dump()
{
  std::ofstream os("../binding.cpp");
  os << jnivm::GeneratePreDeclaration(jnienv);
  os << jnivm::GenerateHeader(jnienv);
  os << jnivm::GenerateStubs(jnienv);
  os << jnivm::GenerateJNIBinding(jnienv);
}
#endif

#ifdef __arm__
namespace FMOD
{
  struct ChannelControl
  {
    int setVolume(float);
    int setPitch(float);
    int addFadePoint(unsigned long long, float);
  };
  struct Sound
  {
    int set3DMinMaxDistance(float, float);
  };
  struct System
  {
    int set3DSettings(float, float, float);
  };
}  // namespace FMOD

// Translate arm softfp to armhf
int32_t __attribute__((pcs("aapcs"))) FMOD_ChannelControl_setVolume(
    FMOD::ChannelControl* ch, float f)
{
  return ch->setVolume(f);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_ChannelControl_setPitch(
    FMOD::ChannelControl* ch, float p)
{
  return ch->setPitch(p);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_System_set3DSettings(
    FMOD::System* sys, float x, float y, float z)
{
  return sys->set3DSettings(x, y, z);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_Sound_set3DMinMaxDistance(
    FMOD::Sound* s, float m, float M)
{
  return s->set3DMinMaxDistance(m, M);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_ChannelControl_addFadePoint(
    FMOD::ChannelControl* ch, unsigned long long i, float f)
{
  return ch->addFadePoint(i, f);
}
#endif

static char initBackup[5] = {};
static char clientInstanceBackup[5] = {};
static void* clientInitSym = nullptr;
static void* clientInstanceUpdate = nullptr;

void setupInitHooks(void* handle)
{
  assert(nullptr != handle);

  if (*SharedConstants::MajorVersion >= 1 &&
      *SharedConstants::MinorVersion >= 1)
  {
    Log::info("Launcher", "Using Modern Game Init");
    isModern = true;
    clientInitSym = hybris_dlsym(handle, "_ZN13MinecraftGame4initEv");
  }
  else
  {
    Log::info("Launcher", "Using Legacy Client Init");
    clientInitSym = hybris_dlsym(handle, "_ZN15MinecraftClient4initEv");
  }

  assert(clientInitSym != nullptr);

  if (clientInitSym)
  {
    PatchUtils::patchCallInstruction(
        clientInitSym,
        (void*)+[](void* clazz)
        {
          const auto client = static_cast<MinecraftClient*>(clazz);
          const auto game = static_cast<MinecraftGame*>(clazz);

          if (isModern)
            minecraftClient = new GenericMinecraft(game, nullptr);
          else
            minecraftClient = new GenericMinecraft(nullptr, client);

          memcpy(clientInitSym, initBackup, 5);
          const auto init = reinterpret_cast<void (*)(void*)>(clientInitSym);

          Log::trace("Launcher", "Collecting client as %p init is: %p", clazz,
                     init);
          init(clazz);
          if (!isModern)
          {
            minecraftClient->font = client->getFont();
            CoreModLoader::getInstance()->onStart(minecraftClient);
          }
        },
        true, initBackup);
  }

  clientInstanceUpdate = hybris_dlsym(handle, "_ZN14ClientInstance6updateEv");

  if (clientInstanceUpdate)
  {
    Log::info("Launcher", "Patching CI Instance, %p", clientInstanceUpdate);
    PatchUtils::patchCallInstruction(
        clientInstanceUpdate,
        (void*)+[](void* clazz)
        {
          const auto instance = static_cast<ClientInstance*>(clazz);
          memcpy(clientInstanceUpdate, clientInstanceBackup, 5);
          const auto update =
              reinterpret_cast<void (*)(void*)>(clientInstanceUpdate);

          Log::trace("Launcher",
                     "Collecting ClientInstance as %p and update is %p", clazz,
                     update);
          update(instance);
          minecraftClient->font = instance->getFont();
          CoreModLoader::getInstance()->onStart(minecraftClient);
        },
        true, clientInstanceBackup);
  }
}

int main(int argc, char* argv[])
{
  static auto windowManager = GameWindowManager::getManager();

  CrashHandler::registerCrashHandler();
  MinecraftUtils::workaroundLocaleBug();

  // Load configuration file
  LauncherConfig config;
  bool configLoaded = config.load();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  ImGui::StyleColorsLight();

  argparser::arg_parser p;

  // Basic options
  argparser::arg<bool> printVersion(p, "--version", "-v",
                                    "Prints version info");
  argparser::arg<bool> saveConfig(p, "--save-config", "-sc",
                                  "Save current settings to config file");

  // Directory options
  argparser::arg<std::string> gameDir(p, "--game-dir", "-dg",
                                      "Directory with the game and assets");
  argparser::arg<std::string> dataDir(p, "--data-dir", "-dd",
                                      "Directory to use for the data");
  argparser::arg<std::string> cacheDir(p, "--cache-dir", "-dc",
                                       "Directory to use for cache");

  // Window options - use config defaults
  argparser::arg<int> windowWidth(p, "--width", "-ww", "Window width",
                                  config.getWindowWidth());
  argparser::arg<int> windowHeight(p, "--height", "-wh", "Window height",
                                   config.getWindowHeight());
  argparser::arg<float> pixelScale(p, "--scale", "-s", "Pixel Scale",
                                   config.getPixelScale());

  // Audio options
  argparser::arg<bool> disableFmod(p, "--disable-fmod", "-df",
                                   "Disables usage of the FMod audio library");

  // Performance options
  argparser::arg<bool> disableCache(
      p, "--disable-cache", "-nc",
      "Disable asset caching (uses more disk I/O)");
  argparser::arg<bool> mallocZero(p, "--malloc-zero", "-mz",
                                  "Patch malloc to always zero initialize "
                                  "memory, this may help workaround MCPE bugs");

  // Debug/Developer options
  argparser::arg<bool> dryRun(p, "--dry-run", "-n",
                              "Validate setup and exit without launching game");
  argparser::arg<bool> verbose(p, "--verbose", "-V",
                               "Enable verbose logging output");
  argparser::arg<bool> quiet(p, "--quiet", "-q", "Suppress most log output");

  if (!p.parse(argc, const_cast<const char**>(argv))) return 0;

  // Determine effective settings (CLI overrides config)
  bool effectiveDisableFmod = disableFmod || config.isFmodDisabled();
  bool effectiveDisableCache = disableCache || config.isAssetCacheDisabled();

  // Handle verbosity settings
  if (quiet)
  {
    g_logVerbosity = 0;
    Log::setMinLevel(LogLevel::LOG_ERROR);
  }
  else if (verbose)
  {
    g_logVerbosity = 2;
    Log::setMinLevel(LogLevel::LOG_TRACE);
  }
  else if (config.getLogLevel() == 0)
  {
    g_logVerbosity = 0;
    Log::setMinLevel(LogLevel::LOG_ERROR);
  }
  else if (config.getLogLevel() == 2)
  {
    g_logVerbosity = 2;
    Log::setMinLevel(LogLevel::LOG_TRACE);
  }

  if (effectiveDisableCache) g_enableAssetCache = false;

  if (printVersion)
  {
    printVersionInfo();
    return 0;
  }

  // Use last game dir from config if not specified
  if (gameDir.get().empty() && !config.getLastGameDir().empty())
  {
    PathHelper::setGameDir(config.getLastGameDir());
  }
  else if (!gameDir.get().empty())
  {
    PathHelper::setGameDir(gameDir);
  }

  if (!dataDir.get().empty()) PathHelper::setDataDir(dataDir);
  if (!cacheDir.get().empty()) PathHelper::setCacheDir(cacheDir);
  if (mallocZero) MinecraftUtils::setMallocZero();

  if (PathHelper::getGameDir() == PathHelper::getPrimaryDataDirectory())
  {
    std::cerr << "ERROR: NO GAME DIRECTORY FOUND.\n";
    std::cerr << "PLEASE PROVIDE A GAME DIRECTORY WITH --game-dir\n";
    if (!config.getLastGameDir().empty())
    {
      std::cerr << "Last used: " << config.getLastGameDir() << "\n";
    }
    std::cerr << "Bye.\n";
    abort();
  }

  // Save config if requested
  if (saveConfig)
  {
    config.setWindowWidth(windowWidth);
    config.setWindowHeight(windowHeight);
    config.setPixelScale(pixelScale);
    config.setDisableFmod(effectiveDisableFmod);
    config.setDisableAssetCache(effectiveDisableCache);
    config.setLogLevel(g_logVerbosity);
    config.setLastGameDir(PathHelper::getGameDir());
    if (config.save())
    {
      std::cout << "Configuration saved to: " << config.getConfigPath() << "\n";
    }
    else
    {
      std::cerr << "Failed to save configuration\n";
    }
    return 0;
  }

  // Update last game dir in config
  config.setLastGameDir(PathHelper::getGameDir());
  config.save();

  // Dry-run mode: validate setup and exit
  if (dryRun)
  {
    std::cout << "=== Lachy-Launcher Dry Run ===\n";
    std::cout << "Client version: " << CLIENT_GIT_COMMIT_HASH << "\n";
    std::cout << "Manifest version: " << MANIFEST_GIT_COMMIT_HASH << "\n";
    std::cout << "Game directory: " << PathHelper::getGameDir() << "\n";
    std::cout << "Data directory: " << PathHelper::getPrimaryDataDirectory()
              << "\n";
    std::cout << "Cache directory: " << PathHelper::getCacheDirectory() << "\n";
    std::cout << "Config file: " << config.getConfigPath()
              << (configLoaded ? " (loaded)" : " (not found)") << "\n";

    // Check for required files
    std::string libPath =
        PathHelper::getGameDir() + "lib/x86/libminecraftpe.so";
    struct stat st;
    if (stat(libPath.c_str(), &st) == 0)
    {
      std::cout << "Minecraft library: FOUND (" << (st.st_size / 1024 / 1024)
                << " MB)\n";
    }
    else
    {
      std::cout << "Minecraft library: NOT FOUND at " << libPath << "\n";
      return 1;
    }

    std::string assetsPath = PathHelper::getGameDir() + "assets/";
    if (stat(assetsPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    {
      std::cout << "Assets directory: FOUND\n";
    }
    else
    {
      std::cout << "Assets directory: NOT FOUND at " << assetsPath << "\n";
      return 1;
    }

    std::cout << "\nAll checks passed. Ready to launch.\n";
    return 0;
  }

  if (g_logVerbosity >= 1)
  {
    Log::info("Launcher", "Version: client %s / manifest %s",
              CLIENT_GIT_COMMIT_HASH, MANIFEST_GIT_COMMIT_HASH);
  }
#ifdef __i386__
  if (g_logVerbosity >= 1)
  {
    CpuId cpuid;
    Log::info("Launcher", "CPU: %s %s", cpuid.getManufacturer(),
              cpuid.getBrandString());
    Log::info("Launcher", "CPU supports SSSE3: %s",
              cpuid.queryFeatureFlag(CpuId::FeatureFlag::SSSE3) ? "YES" : "NO");
  }
#endif

  GraphicsApi graphicsApi = GraphicsApi::OPENGL_ES2;

  if (g_logVerbosity >= 2) Log::trace("Launcher", "Loading hybris libraries");
  if (!effectiveDisableFmod)
  {
    // Store asset directory path in static for use in FMOD createSound hook
    static std::string g_assetDir = std::string(gameDir) + "/assets";

    MinecraftUtils::loadFMod();

    // Hook FMOD::System::createSound to redirect android_asset paths to real
    // files Legacy Minecraft versions use file:///android_asset/ URLs which
    // don't exist on Linux
    static auto orig_createSound =
        (int (*)(void*, const char*, unsigned int, void*, void**))
            get_hooked_symbol(
                "_ZN4FMOD6System11createSoundEPKcjP22FMOD_"
                "CREATESOUNDEXINFOPPNS_5SoundE");
    if (orig_createSound)
    {
      hybris_hook(
          "_ZN4FMOD6System11createSoundEPKcjP22FMOD_CREATESOUNDEXINFOPPNS_"
          "5SoundE",
          (void*)+[](void* sys, const char* name, unsigned int mode,
                     void* exinfo, void** sound) -> int
          {
            std::string actualPath;
            const char* pathToUse = name;

            // Redirect file:///android_asset/ paths to actual assets directory
            if (name && strncmp(name, "file:///android_asset/", 22) == 0)
            {
              actualPath = g_assetDir + "/" + (name + 22);
              pathToUse = actualPath.c_str();
            }

            return orig_createSound(sys, pathToUse, mode, exinfo, sound);
          });
    }

    // Hook FMOD::System::createStream for streaming audio (music)
    // Same path redirection as createSound
    static auto orig_createStream =
        (int (*)(void*, const char*, unsigned int, void*, void**))
            get_hooked_symbol(
                "_ZN4FMOD6System12createStreamEPKcjP22FMOD_"
                "CREATESOUNDEXINFOPPNS_5SoundE");
    if (orig_createStream)
    {
      hybris_hook(
          "_ZN4FMOD6System12createStreamEPKcjP22FMOD_CREATESOUNDEXINFOPPNS_"
          "5SoundE",
          (void*)+[](void* sys, const char* name, unsigned int mode,
                     void* exinfo, void** sound) -> int
          {
            std::string actualPath;
            const char* pathToUse = name;

            if (name && strncmp(name, "file:///android_asset/", 22) == 0)
            {
              actualPath = g_assetDir + "/" + (name + 22);
              pathToUse = actualPath.c_str();
            }

            return orig_createStream(sys, pathToUse, mode, exinfo, sound);
          });
    }

#ifdef __arm__
    hybris_hook("_ZN4FMOD14ChannelControl9setVolumeEf",
                (void*)&FMOD_ChannelControl_setVolume);
    hybris_hook("_ZN4FMOD14ChannelControl8setPitchEf",
                (void*)&FMOD_ChannelControl_setPitch);
    hybris_hook("_ZN4FMOD6System13set3DSettingsEfff",
                (void*)&FMOD_System_set3DSettings);
    hybris_hook("_ZN4FMOD5Sound19set3DMinMaxDistanceEff",
                (void*)&FMOD_Sound_set3DMinMaxDistance);
    hybris_hook("_ZN4FMOD14ChannelControl12addFadePointEyf",
                (void*)&FMOD_ChannelControl_addFadePoint);
#endif
  }
  else
    MinecraftUtils::stubFMod();
  // Get rid of defining OPENSSL_armcap
  hybris_hook("OPENSSL_cpuid_setup", (void*)+[]() -> void {});

  MinecraftUtils::setupHybris();

  Log::info("Launcher", "Creating window");
  WindowCallbacks::loadGamepadMappings();
  static auto window = windowManager->createWindow("Lachy", windowWidth,
                                                   windowHeight, graphicsApi);

  window->setIcon(PathHelper::getIconPath());
  window->show();
  hybris_hook(
      "ANativeActivity_finish",
      (void*)+[](ANativeActivity* activity)
      {
        Log::warn("Launcher", "Android stub %s called",
                  "ANativeActivity_finish");
        std::thread(
            [=]()
            {
              // Saves nothing (returns every time null)
              // size_t outSize;
              // void * data =
              // activity->callbacks->onSaveInstanceState(activity, &outSize);
              ((void (*)(JNIEnv* env, void*))hybris_dlsym(
                  jnienv->functions->reserved3,
                  "Java_com_mojang_minecraftpe_MainActivity_"
                  "nativeUnregisterThis"))(jnienv, nullptr);
              ((void (*)(JNIEnv* env, void*))hybris_dlsym(
                  jnienv->functions->reserved3,
                  "Java_com_mojang_minecraftpe_MainActivity_nativeSuspend"))(
                  jnienv, nullptr);
              ((void (*)(JNIEnv* env, void*))hybris_dlsym(
                  jnienv->functions->reserved3,
                  "Java_com_mojang_minecraftpe_MainActivity_nativeShutdown"))(
                  jnienv, nullptr);
              activity->callbacks->onStop(activity);
            })
            .detach();
        // With Xboxlive it usually don't close the Game with the main function
        // correctly Force exit with code 0 (Maybe Android related)
        _Exit(0);
      });
  hybris_hook(
      "eglChooseConfig",
      (void*)+[](EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs,
                 EGLint config_size, EGLint* num_config)
      {
        *num_config = 1;
        return EGL_TRUE;
      });
  hybris_hook("eglGetError", (void*)(void (*)()) []() {});
  hybris_hook(
      "eglGetCurrentDisplay",
      (void*)+[]() -> EGLDisplay
      {
        return (EGLDisplay)1;
      });
  hybris_hook(
      "eglCreateWindowSurface",
      (void*)+[](EGLDisplay display, EGLConfig config,
                 NativeWindowType native_window, EGLint const* attrib_list)
      {
        return native_window;
      });
  hybris_hook(
      "eglGetConfigAttrib",
      (void*)+[](EGLDisplay display, EGLConfig config, EGLint attribute,
                 EGLint* value)
      {
        return EGL_TRUE;
      });
  hybris_hook(
      "eglCreateContext",
      (void*)+[](EGLDisplay display, EGLConfig config, EGLContext share_context,
                 EGLint const* attrib_list)
      {
        return 1;
      });
  hybris_hook("eglDestroySurface", (void*)(void (*)()) []() {});
  hybris_hook(
      "eglSwapBuffers",
      (void*)+[](EGLDisplay* display, EGLSurface surface)
      {
        window->swapBuffers();
      });
  hybris_hook(
      "eglMakeCurrent",
      (void*)+[](EGLDisplay display, EGLSurface draw, EGLSurface read,
                 EGLContext context)
      {
        Log::warn("Launcher", "EGL stub %s called", "eglMakeCurrent");
        return EGL_TRUE;
      });
  hybris_hook("eglDestroyContext", (void*)(void (*)()) []() {});
  hybris_hook("eglTerminate", (void*)(void (*)()) []() {});
  hybris_hook(
      "eglGetDisplay",
      (void*)+[](NativeDisplayType native_display)
      {
        return 1;
      });
  hybris_hook(
      "eglInitialize",
      (void*)+[](void* display, uint32_t* major, uint32_t* minor)
      {
        return EGL_TRUE;
      });
  hybris_hook(
      "eglQuerySurface",
      (void*)+[](void* dpy, EGLSurface surface, EGLint attribute, EGLint* value)
      {
        int dummy;
        switch (attribute)
        {
          case EGL_WIDTH:
            window->getWindowSize(*value, dummy);
            break;
          case EGL_HEIGHT:
            window->getWindowSize(dummy, *value);
            break;
          default:
            return EGL_FALSE;
        }
        return EGL_TRUE;
      });
  hybris_hook(
      "eglSwapInterval",
      (void*)+[](EGLDisplay display, EGLint interval)
      {
        window->swapInterval(interval);
        return EGL_TRUE;
      });
  hybris_hook(
      "eglQueryString",
      (void*)+[](void* display, int32_t name)
      {
        return 0;
      });
  hybris_hook(
      "eglGetProcAddress",
      (void*)+[](char* ch) -> void*
      {
        static std::map<std::string, void*> eglfuncs = {
          { "glInvalidateFramebuffer", (void*)+[]() {} }
        };
        auto hook = eglfuncs[ch];
        return hook
                   ? hook
                   : ((void* (*)(const char*))windowManager->getProcAddrFunc())(
                         ch);
      });
  hybris_hook(
      "eglGetCurrentContext",
      (void*)+[]() -> int
      {
        return 0;
      });
  MinecraftUtils::setupGLES2Symbols(
      (void* (*)(const char*))windowManager->getProcAddrFunc());
#ifdef USE_ARMHF_SUPPORT
  ArmhfSupport::install();
#endif

  struct Looper
  {
    int fd;
    int indent;
    void* data;
    int indent2;
    void* data2;
  };
  static Looper looper;
  hybris_hook(
      "ALooper_pollAll",
      (void*)+[](int timeoutMillis, int* outFd, int* outEvents, void** outData)
      {
        fd_set rfds;
        struct timeval tv;
        int retval;

        /* Watch stdin (fd 0) to see when it has input. */

        FD_ZERO(&rfds);
        FD_SET(looper.fd, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        retval = select(looper.fd + 1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (retval == -1)
          perror("select()");
        else if (retval)
        {
          // printf("Data is available now.\n");
          *outData = looper.data;
          return looper.indent;
          /* FD_ISSET(0, &rfds) will be true. */
        }

        window->pollEvents();

        return -3;
      });
  hybris_hook(
      "ALooper_addFd",
      (void*)+[](void* loopere, int fd, int ident, int events,
                 int (*callback)(int fd, int events, void* data), void* data)
      {
        looper.fd = fd;
        looper.indent = ident;
        looper.data = data;
        return 1;
      });
  hybris_hook(
      "AInputQueue_attachLooper",
      (void*)+[](void* queue, void* looper2, int ident, void* callback,
                 void* data)
      {
        looper.indent2 = ident;
        looper.data2 = data;
      });

  // Hook AppPlatform function directly (functions are too small for a jump
  // instruction) static vtable replace isn't working
  auto hide = (void*)+[](void* t)
  {
    window->setCursorDisabled(true);
  };
  auto show = (void*)+[](void* t)
  {
    window->setCursorDisabled(false);
  };

  hybris_hook("uncompress", (void*)(void (*)()) []() {});

  OpenSLESPatch::install();

  // Hack pthread to run mainthread on the main function #macoscacoa support
  static std::atomic_bool uithread_started;
  uithread_started = false;
  static void* (*main_routine)(void*) = nullptr;
  static void* main_arg = nullptr;
  static pthread_t mainthread = pthread_self();
  static int (*my_pthread_create)(pthread_t* thread,
                                  const pthread_attr_t* __attr,
                                  void* (*start_routine)(void*), void* arg) = 0;
  my_pthread_create = (int (*)(pthread_t* thread, const pthread_attr_t* __attr,
                               void* (*start_routine)(void*),
                               void* arg))get_hooked_symbol("pthread_create");
  hybris_hook(
      "pthread_create",
      (void*)+[](pthread_t* thread, const pthread_attr_t* __attr,
                 void* (*start_routine)(void*), void* arg)
      {
        if (uithread_started.load())
        {
          return my_pthread_create(thread, __attr, start_routine, arg);
        }
        else
        {
          uithread_started = true;
          *thread = mainthread;
          main_routine = start_routine;
          main_arg = arg;
          return 0;
        }
      });

  static auto my_fopen =
      (void* (*)(const char* filename, const char* mode))get_hooked_symbol(
          "fopen");
  hybris_hook(
      "fopen",
      (void*)+[](const char* filename, const char* mode)
      {
        if (!strcmp(filename,
                    "/data/data/com.mojang.minecraftpe/games/"
                    "com.mojang/minecraftpe/external_servers.txt"))
        {
          return my_fopen(
              (PathHelper::getPrimaryDataDirectory() + (filename + 34)).data(),
              mode);
        }
        else
        {
          return my_fopen(filename, mode);
        }
      });
  // For 0.11 or lower
  hybris_hook("ftime", (void*)&ftime_replacement);
  OpenSLESPatch::install();

#ifdef __i386__
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO | SA_RESTART;
  act.sa_sigaction = [](int, siginfo_t* si, void* ptr)
  {
    *(char*)si->si_addr = 0x90;
    *((char*)si->si_addr + 1) = 0x90;
    Log::warn("Minecraft BUG",
              "SIGFPE Experimental patch applied, the Game will continue now");
  };
  sigaction(SIGFPE, &act, NULL);
#endif

  Log::trace("Launcher", "Loading Minecraft library");
  void* handle = MinecraftUtils::loadMinecraftLib();
  if (!handle)
  {
    Log::error("Launcher",
               "Failed to load Minecraft library, please reinstall");
    return 51;
  }
  Log::info("Launcher", "Loaded Minecraft library");
  Log::debug("Launcher", "Minecraft is at offset 0x%x",
             MinecraftUtils::getLibraryBase(handle));

  void** vt = &((void**)hybris_dlsym(handle, "_ZTV21AppPlatform_android23"))[2];
  void** vta = &((void**)hybris_dlsym(handle, "_ZTV19AppPlatform_android"))[2];
  auto myVtableSize = PatchUtils::getVtableSize(vta);
  Log::trace("AppPlatform", "Vtable size = %u", myVtableSize);

  PatchUtils::VtableReplaceHelper vtr(handle, vt, vta);
  vtr.replace("_ZN11AppPlatform16hideMousePointerEv", hide);
  vtr.replace("_ZN11AppPlatform16showMousePointerEv", show);

  patchDesktopUi(vtr);

  patchFixSettingsPath(vtr);

  patchReadAssetFileLegacy(vtr, handle);

  auto client =
      hybris_dlsym(handle,
                   "_ZN3web4http6client7details35verify_cert_chain_platform_"
                   "specificERN5boost4asio3ssl14verify_contextERKSs");
  if (client)
  {
    PatchUtils::patchCallInstruction(
        client,
        (void*)+[]()
        {
          // Log::trace("web::http::client",
          // "verify_cert_chain_platform_specific stub called");
          return true;
        },
        true);
  }

  auto coreModLoader = CoreModLoader::getInstance();

  coreModLoader->loadModsFromDirectory(PathHelper::getPrimaryDataDirectory() +
                                       "coremods/");
  coreModLoader->loadModsFromDirectory(PathHelper::getWorkingDir() +
                                       "coremods/");

  MinecraftUtils::initSymbolBindings(handle);

  setupInitHooks(handle);

  coreModLoader->onCreate(handle);

  ModLoader modLoader;
  modLoader.loadModsFromDirectory(PathHelper::getPrimaryDataDirectory() +
                                  "mods/");

  HbuiPatch::install(handle);
  ANativeActivity activity;
  memset(&activity, 0, sizeof(ANativeActivity));
  activity.internalDataPath = "./idata/";
  activity.externalDataPath = "./edata/";
  activity.obbPath = "./oob/";
  activity.sdkVersion = 28;
  jnivm::VM vm;
  activity.vm = vm.GetJavaVM();
  // activity.assetManager = (struct AAssetManager*)23;
  ANativeActivityCallbacks callbacks;
  memset(&callbacks, 0, sizeof(ANativeActivityCallbacks));
  activity.callbacks = &callbacks;
  activity.vm->GetEnv(&(void*&)activity.env, 0);
  jnienv = activity.env;
  vm.SetReserved3(handle);
  // Avoid using cd by hand
  chdir((PathHelper::getGameDir() + "/assets").data());
  // Initialize fake java interop
  auto JNI_OnLoad =
      (jint (*)(JavaVM* vm, void* reserved))hybris_dlsym(handle, "JNI_OnLoad");
  if (JNI_OnLoad) JNI_OnLoad(activity.vm, 0);
  auto mainactivity = new com::mojang::minecraftpe::MainActivity(handle);
  mainactivity->clazz = (java::lang::Class*)activity.env->FindClass(
      "com/mojang/minecraftpe/MainActivity");  // new jnivm::Object<void> { .cl
                                               // =
                                               // activity.env->FindClass("com/mojang/minecraftpe/MainActivity"),
                                               // .value = new int() };
  mainactivity->window = window;
  activity.clazz = mainactivity;
  WindowCallbacks windowCallbacks(*window, activity);
  windowCallbacks.handle = handle;
  windowCallbacks.vm = &vm;
  windowCallbacks.client = &minecraftClient;
  windowCallbacks.registerCallbacks();
  std::thread(
      [&,
       ANativeActivity_onCreate = (ANativeActivity_createFunc*)hybris_dlsym(
           handle, "ANativeActivity_onCreate"),
       registerthis = (void (*)(JNIEnv* env, void*))hybris_dlsym(
           jnienv->functions->reserved3,
           "Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis")]()
      {
        ANativeActivity_onCreate(&activity, 0, 0);
        if (registerthis) registerthis(jnienv, activity.clazz);
        activity.callbacks->onInputQueueCreated(&activity, (AInputQueue*)2);
        activity.callbacks->onNativeWindowCreated(&activity,
                                                  (ANativeWindow*)window.get());
        activity.callbacks->onStart(&activity);
        // For 0.14 or lower
        activity.callbacks->onResume(&activity);
      })
      .detach();
  while (!uithread_started.load())
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  window->prepareRunLoop();

  CoreModLoader::getInstance()->onGameWindowCreated(window);
  auto res = main_routine(main_arg);
  _Exit(0);
}

void printVersionInfo()
{
  printf("mcpelauncher-client %s / manifest %s\n", CLIENT_GIT_COMMIT_HASH,
         MANIFEST_GIT_COMMIT_HASH);
#ifdef __i386__
  CpuId cpuid;
  printf("CPU: %s %s\n", cpuid.getManufacturer(), cpuid.getBrandString());
  printf("SSSE3 support: %s\n",
         cpuid.queryFeatureFlag(CpuId::FeatureFlag::SSSE3) ? "YES" : "NO");
#endif
  auto windowManager = GameWindowManager::getManager();
  GraphicsApi graphicsApi = GraphicsApi::OPENGL_ES2;
  auto window =
      windowManager->createWindow("mcpelauncher", 32, 32, graphicsApi);
  auto glGetString =
      (const char* (*)(int))windowManager->getProcAddrFunc()("glGetString");
  printf("GL Vendor: %s\n", glGetString(0x1F00 /* GL_VENDOR */));
  printf("GL Renderer: %s\n", glGetString(0x1F01 /* GL_RENDERER */));
  printf("GL Version: %s\n", glGetString(0x1F02 /* GL_VERSION */));
  printf("MSA daemon path: %s\n", XboxLiveHelper::findMsa().c_str());
}
