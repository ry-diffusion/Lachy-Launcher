#include <log.h>
#include <hybris/hook.h>
#include <hybris/dlfcn.h>
#include <dlfcn.h>
#include <game_window_manager.h>
#include <argparser.h>
#include <mcpelauncher/minecraft_utils.h>
#include <mcpelauncher/minecraft_version.h>
#include <mcpelauncher/crash_handler.h>
#include <mcpelauncher/path_helper.h>
#include <minecraft/Common.h>
#include <minecraft/MinecraftGame.h>
#include <minecraft/ClientInstance.h>
#include <mcpelauncher/mod_loader.h>
#include <mcpelauncher/patch_utils.h>
#include "window_callbacks.h"
#include "xbox_live_helper.h"
#include "hbui_patch.h"
#ifdef USE_ARMHF_SUPPORT
#include "armhf_support.h"
#endif
#ifdef __i386__
#include "cpuid.h"
#endif
#include <build_info.h>
#include "native_activity.h"
#include <jnivm.h>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <hybris/hook.h>
#include <signal.h>
#include <unistd.h>
#include "JNIBinding.h"
#include <sys/timeb.h>
#include "OpenSLESPatch.h"

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

JNIEnv * jnienv = 0;

void printVersionInfo();

#ifdef JNI_DEBUG
void dump() {
    std::ofstream os("../binding.cpp");
    os << jnivm::GeneratePreDeclaration(jnienv);
    os << jnivm::GenerateHeader(jnienv);
    os << jnivm::GenerateStubs(jnienv);
    os << jnivm::GenerateJNIBinding(jnienv);
}
#endif

#ifdef __arm__
namespace FMOD {
  struct ChannelControl {
    int setVolume(float);
    int setPitch(float);
    int addFadePoint(unsigned long long, float);
  };
  struct Sound {
    int set3DMinMaxDistance(float, float);
  };
  struct System {
    int set3DSettings(float, float, float);
  };
}

// Translate arm softfp to armhf
int32_t __attribute__((pcs("aapcs"))) FMOD_ChannelControl_setVolume(FMOD::ChannelControl *ch, float f) {
  return ch->setVolume(f);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_ChannelControl_setPitch(FMOD::ChannelControl *ch, float p) {
  return ch->setPitch(p);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_System_set3DSettings(FMOD::System *sys, float x, float y, float z) {
  return sys->set3DSettings(x, y, z);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_Sound_set3DMinMaxDistance(FMOD::Sound *s, float m, float M) {
  return s->set3DMinMaxDistance(m, M);
}

int32_t __attribute__((pcs("aapcs"))) FMOD_ChannelControl_addFadePoint(FMOD::ChannelControl *ch, unsigned long long i, float f) {
  return ch->addFadePoint(i, f);
}
#endif

int main(int argc, char *argv[]) {
    static auto windowManager = GameWindowManager::getManager();
    CrashHandler::registerCrashHandler();
    MinecraftUtils::workaroundLocaleBug();

    argparser::arg_parser p;
    argparser::arg<bool> printVersion (p, "--version", "-v", "Prints version info");
    argparser::arg<std::string> gameDir (p, "--game-dir", "-dg", "Directory with the game and assets");
    argparser::arg<std::string> dataDir (p, "--data-dir", "-dd", "Directory to use for the data");
    argparser::arg<std::string> cacheDir (p, "--cache-dir", "-dc", "Directory to use for cache");
    argparser::arg<int> windowWidth (p, "--width", "-ww", "Window width", 720);
    argparser::arg<int> windowHeight (p, "--height", "-wh", "Window height", 480);
    argparser::arg<float> pixelScale (p, "--scale", "-s", "Pixel Scale", 2.f);
    argparser::arg<bool> mallocZero (p, "--malloc-zero", "-mz", "Patch malloc to always zero initialize memory, this may help workaround MCPE bugs");
    argparser::arg<bool> disableFmod (p, "--disable-fmod", "-df", "Disables usage of the FMod audio library");
    if (!p.parse(argc, (const char**) argv))
        return 1;
    if (printVersion) {
        printVersionInfo();
        return 0;
    }
    if (!gameDir.get().empty())
        PathHelper::setGameDir(gameDir);
    if (!dataDir.get().empty())        PathHelper::setDataDir(dataDir);
    if (!cacheDir.get().empty())
        PathHelper::setCacheDir(cacheDir);
    if (mallocZero)
        MinecraftUtils::setMallocZero();

    Log::info("Launcher", "Version: client %s / manifest %s", CLIENT_GIT_COMMIT_HASH, MANIFEST_GIT_COMMIT_HASH);
#ifdef __i386__
    {
        CpuId cpuid;
        Log::info("Launcher", "CPU: %s %s", cpuid.getManufacturer(), cpuid.getBrandString());
        Log::info("Launcher", "CPU supports SSSE3: %s",
                cpuid.queryFeatureFlag(CpuId::FeatureFlag::SSSE3) ? "YES" : "NO");
    }
#endif

    GraphicsApi graphicsApi = GraphicsApi::OPENGL_ES2;

    Log::trace("Launcher", "Loading hybris libraries");
    if (!disableFmod) {
      MinecraftUtils::loadFMod();
#ifdef __arm__
      hybris_hook("_ZN4FMOD14ChannelControl9setVolumeEf", (void*)&FMOD_ChannelControl_setVolume);
      hybris_hook("_ZN4FMOD14ChannelControl8setPitchEf", (void*)&FMOD_ChannelControl_setPitch); 
      hybris_hook("_ZN4FMOD6System13set3DSettingsEfff", (void*)&FMOD_System_set3DSettings); 
      hybris_hook("_ZN4FMOD5Sound19set3DMinMaxDistanceEff", (void*)&FMOD_Sound_set3DMinMaxDistance); 
      hybris_hook("_ZN4FMOD14ChannelControl12addFadePointEyf", (void*)&FMOD_ChannelControl_addFadePoint);
#endif
    }
    else
        MinecraftUtils::stubFMod();
    // Get rid of defining OPENSSL_armcap
    hybris_hook("OPENSSL_cpuid_setup", (void*) + []() -> void {});

    MinecraftUtils::setupHybris();

    Log::info("Launcher", "Creating window");
    WindowCallbacks::loadGamepadMappings();
    static auto window = windowManager->createWindow("Minecraft", windowWidth, windowHeight, graphicsApi);
    window->setIcon(PathHelper::getIconPath());
    window->show();
    hybris_hook("ANativeActivity_finish", (void *)+[](ANativeActivity *activity) {
      Log::warn("Launcher", "Android stub %s called", "ANativeActivity_finish");
      std::thread([=]() {
        // Saves nothing (returns every time null)
        // size_t outSize;
        // void * data = activity->callbacks->onSaveInstanceState(activity, &outSize);
        ((void(*)(JNIEnv * env, void*))hybris_dlsym(jnienv->functions->reserved3, "Java_com_mojang_minecraftpe_MainActivity_nativeUnregisterThis"))(jnienv, nullptr);
        ((void(*)(JNIEnv * env, void*))hybris_dlsym(jnienv->functions->reserved3, "Java_com_mojang_minecraftpe_MainActivity_nativeSuspend"))(jnienv, nullptr);
        ((void(*)(JNIEnv * env, void*))hybris_dlsym(jnienv->functions->reserved3, "Java_com_mojang_minecraftpe_MainActivity_nativeShutdown"))(jnienv, nullptr);
        activity->callbacks->onStop(activity);
      }).detach();
      // With Xboxlive it usually don't close the Game with the main function correctly
      // Force exit with code 0 (Maybe Android related)
      _Exit(0);
    });
    hybris_hook("eglChooseConfig", (void *)+[](EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
      *num_config = 1;
      return EGL_TRUE;
    });
    hybris_hook("eglGetError", (void *)(void (*)())[]() {
    });
    hybris_hook("eglGetCurrentDisplay", (void *)+[]() -> EGLDisplay {
      return (EGLDisplay)1;
    });
    hybris_hook("eglCreateWindowSurface", (void *)+[](EGLDisplay display,
      EGLConfig config,
      NativeWindowType native_window,
      EGLint const * attrib_list) {
      return native_window;
    });
    hybris_hook("eglGetConfigAttrib", (void *)+[](EGLDisplay display,
      EGLConfig config,
      EGLint attribute,
      EGLint * value) {
      return EGL_TRUE;
    });
    hybris_hook("eglCreateContext", (void *)+[](EGLDisplay display,
      EGLConfig config,
      EGLContext share_context,
      EGLint const * attrib_list) {
      return 1;
    });
    hybris_hook("eglDestroySurface", (void *)(void (*)())[]() {
    });
    hybris_hook("eglSwapBuffers", (void *)+[](EGLDisplay *display,
      EGLSurface surface) {
        window->swapBuffers();
    });
    hybris_hook("eglMakeCurrent", (void *)+[](EGLDisplay display,
      EGLSurface draw,
      EGLSurface read,
      EGLContext context) {
      Log::warn("Launcher", "EGL stub %s called", "eglMakeCurrent");
      return EGL_TRUE;
    });
    hybris_hook("eglDestroyContext", (void *)(void (*)())[]() {
    });
    hybris_hook("eglTerminate", (void *)(void (*)())[]() {
    });
    hybris_hook("eglGetDisplay", (void *)+[](NativeDisplayType native_display) {
      return 1; 
    });
    hybris_hook("eglInitialize", (void *)+[](void* display, uint32_t * major, uint32_t * minor) {
      return EGL_TRUE;
    });
    hybris_hook("eglQuerySurface", (void *) + [](void* dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
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
    hybris_hook("eglSwapInterval", (void *)+[](EGLDisplay display, EGLint interval) {
        window->swapInterval(interval);
      return EGL_TRUE;
    });
    hybris_hook("eglQueryString", (void *)+[](void* display, int32_t name) {
        return 0;
    });
    hybris_hook("eglGetProcAddress", (void*)+[](char* ch)->void*{
      static std::map<std::string, void*> eglfuncs = {{ "glInvalidateFramebuffer", (void*)+[]() {}}};
      auto hook = eglfuncs[ch];
      return hook ? hook : ((void* (*)(const char*))windowManager->getProcAddrFunc())(ch);
    });
    hybris_hook("eglGetCurrentContext", (void*) + []() -> int {
      return 0;
    });
    MinecraftUtils::setupGLES2Symbols((void* (*)(const char*)) windowManager->getProcAddrFunc());
#ifdef USE_ARMHF_SUPPORT
    ArmhfSupport::install();
#endif

    struct Looper {
      int fd;
      int indent;
      void * data;
      int indent2;
      void * data2;
    };
    static Looper looper;
    hybris_hook("ALooper_pollAll", (void *)+[](  int timeoutMillis,
    int *outFd,
    int *outEvents,
    void **outData) {
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
      else if (retval) {
          // printf("Data is available now.\n");
          *outData = looper.data;
          return looper.indent;
          /* FD_ISSET(0, &rfds) will be true. */
      }

      window->pollEvents();

      return -3;
    });
    hybris_hook("ALooper_addFd", (void *)+[](  void *loopere ,
      int fd,
      int ident,
      int events,
      int(* callback)(int fd, int events, void *data),
      void *data) {
      looper.fd = fd;
      looper.indent = ident;
      looper.data = data;
      return 1;
    });
    hybris_hook("AInputQueue_attachLooper", (void *)+[](  void *queue,
    void *looper2,
    int ident,
    void* callback,
    void *data) {
      looper.indent2 = ident;
      looper.data2 = data;
    });

    // Hook AppPlatform function directly (functions are too small for a jump instruction)
    // static vtable replace isn't working
    auto hide = (void*) + [](void* t) {
        window->setCursorDisabled(true);
    };
    auto show = (void*) + [](void* t) {
        window->setCursorDisabled(false);
    };

    hybris_hook("uncompress", (void *)(void (*)())[]() {
    });

    OpenSLESPatch::install();

    // Hack pthread to run mainthread on the main function #macoscacoa support
    static std::atomic_bool uithread_started;
    uithread_started = false;
    static void *(*main_routine)(void*) = nullptr;
    static void *main_arg = nullptr;
    static pthread_t mainthread = pthread_self();
    static int (*my_pthread_create)(pthread_t *thread, const pthread_attr_t *__attr,
                             void *(*start_routine)(void*), void *arg) = 0;
    my_pthread_create = (int (*)(pthread_t *thread, const pthread_attr_t *__attr,
                             void *(*start_routine)(void*), void *arg))get_hooked_symbol("pthread_create");
    hybris_hook("pthread_create", (void*) + [](pthread_t *thread, const pthread_attr_t *__attr,
        void *(*start_routine)(void*), void *arg) {
        if(uithread_started.load()) {
          return my_pthread_create(thread, __attr, start_routine, arg);
        } else {
          uithread_started = true;
          *thread = mainthread;
          main_routine = start_routine;
          main_arg = arg;
          return 0;
        }
      }
    );

    static auto my_fopen = (void*(*)(const char *filename, const char *mode))get_hooked_symbol("fopen");
    hybris_hook("fopen", (void*) + [](const char *filename, const char *mode) {
      if(!strcmp(filename, "/data/data/com.mojang.minecraftpe/games/com.mojang/minecraftpe/external_servers.txt")) {
          return my_fopen((PathHelper::getPrimaryDataDirectory() + (filename + 34)).data(), mode);
      } else {
        return my_fopen(filename, mode);
      }
    });
    // For 0.11 or lower
    hybris_hook("ftime", (void*)&ftime);
    OpenSLESPatch::install();

    #ifdef __i386__
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    act.sa_sigaction = [](int, siginfo_t *si, void *ptr) {
      *(char*)si->si_addr = 0x90;
      *((char*)si->si_addr + 1) = 0x90;
      Log::warn("Minecraft BUG", "SIGFPE Experimental patch applied, the Game will continue now");
    };
    sigaction(SIGFPE, &act, NULL);
    #endif

    Log::trace("Launcher", "Loading Minecraft library");
    void * handle = MinecraftUtils::loadMinecraftLib();
    if (!handle) {
      Log::error("Launcher", "Failed to load Minecraft library, please reinstall");
      return 51;
    }
    Log::info("Launcher", "Loaded Minecraft library");
    Log::debug("Launcher", "Minecraft is at offset 0x%x", MinecraftUtils::getLibraryBase(handle));

    void** vt = &((void**) hybris_dlsym(handle, "_ZTV21AppPlatform_android23"))[2];
    void** vta = &((void**) hybris_dlsym(handle, "_ZTV19AppPlatform_android"))[2];
    auto myVtableSize = PatchUtils::getVtableSize(vta);
    Log::trace("AppPlatform", "Vtable size = %u", myVtableSize);

    PatchUtils::VtableReplaceHelper vtr (handle, vt, vta);
    vtr.replace("_ZN11AppPlatform16hideMousePointerEv", hide);
    vtr.replace("_ZN11AppPlatform16showMousePointerEv", show);
    auto client = hybris_dlsym(handle, "_ZN3web4http6client7details35verify_cert_chain_platform_specificERN5boost4asio3ssl14verify_contextERKSs");
    if(client) {
        PatchUtils::patchCallInstruction(client, (void*) + []() {
            // Log::trace("web::http::client", "verify_cert_chain_platform_specific stub called");
            return true;
        }, true);
    }
    ModLoader modLoader;
    modLoader.loadModsFromDirectory(PathHelper::getPrimaryDataDirectory() + "mods/");
    MinecraftUtils::initSymbolBindings(handle);
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
    auto JNI_OnLoad = (jint (*)(JavaVM* vm, void* reserved))hybris_dlsym(handle, "JNI_OnLoad");
    if (JNI_OnLoad) JNI_OnLoad(activity.vm, 0);
    auto mainactivity = new com::mojang::minecraftpe::MainActivity(handle);
    mainactivity->clazz = (java::lang::Class*)activity.env->FindClass("com/mojang/minecraftpe/MainActivity");//new jnivm::Object<void> { .cl = activity.env->FindClass("com/mojang/minecraftpe/MainActivity"), .value = new int() };
    mainactivity->window = window;
    activity.clazz = mainactivity;
    WindowCallbacks windowCallbacks (*window, activity);
    windowCallbacks.handle = handle;
    windowCallbacks.vm = &vm;
    windowCallbacks.registerCallbacks();
    std::thread([&,ANativeActivity_onCreate = (ANativeActivity_createFunc*)hybris_dlsym(handle, "ANativeActivity_onCreate"), registerthis = (void(*)(JNIEnv * env, void*))hybris_dlsym(jnienv->functions->reserved3, "Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis")]() {
      ANativeActivity_onCreate(&activity, 0, 0);
      if (registerthis) registerthis(jnienv, activity.clazz);
      activity.callbacks->onInputQueueCreated(&activity, (AInputQueue*)2);
      activity.callbacks->onNativeWindowCreated(&activity, (ANativeWindow*)window.get());
      activity.callbacks->onStart(&activity);
      // For 0.14 or lower
      activity.callbacks->onResume(&activity);
    }).detach();
    while (!uithread_started.load()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    window->prepareRunLoop();
    auto res = main_routine(main_arg);
    _Exit(0);
}

void printVersionInfo() {
    printf("mcpelauncher-client %s / manifest %s\n", CLIENT_GIT_COMMIT_HASH, MANIFEST_GIT_COMMIT_HASH);
#ifdef __i386__
    CpuId cpuid;
    printf("CPU: %s %s\n", cpuid.getManufacturer(), cpuid.getBrandString());
    printf("SSSE3 support: %s\n", cpuid.queryFeatureFlag(CpuId::FeatureFlag::SSSE3) ? "YES" : "NO");
#endif
    auto windowManager = GameWindowManager::getManager();
    GraphicsApi graphicsApi = GraphicsApi::OPENGL_ES2;
    auto window = windowManager->createWindow("mcpelauncher", 32, 32, graphicsApi);
    auto glGetString = (const char* (*)(int)) windowManager->getProcAddrFunc()("glGetString");
    printf("GL Vendor: %s\n", glGetString(0x1F00 /* GL_VENDOR */));
    printf("GL Renderer: %s\n", glGetString(0x1F01 /* GL_RENDERER */));
    printf("GL Version: %s\n", glGetString(0x1F02 /* GL_VERSION */));
    printf("MSA daemon path: %s\n", XboxLiveHelper::findMsa().c_str());
}
