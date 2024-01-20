#pragma once

#include <jni.h>
#include <jnivm.h>
#include <tuple>
#include <vector>
#include <fstream>
#include <mcpelauncher/path_helper.h>
#include <sstream>
#include "xbox_live_game_interface.h"
#include "xbox_live_helper.h"
#include <file_picker_factory.h>
#include <hybris/dlfcn.h>
#include <mcpelauncher/path_helper.h>
#include <game_window.h>

namespace jnivm {
namespace com {
    namespace mojang {
        namespace minecraftpe {
            class MainActivity;
            class HardwareInformation;
            namespace store {
                class NativeStoreListener;
                class Product;
                class Purchase;
                class StoreFactory;
                class StoreListener;
                class Store;
                class ExtraLicenseResponseData;
            }
        }
        namespace android {
            namespace net {
                class HTTPResponse;
                class HTTPRequest;
            }
        }
    }
    namespace microsoft {
        namespace xbox {
            namespace idp {
                namespace interop {
                    class Interop;
                    class LocalConfig;
                    class XboxLiveAppConfig;
                }
                namespace util {
                    class HttpCall;
                    class AuthFlowResult;
                }
            }
        }
        namespace xboxtcui {
            class Interop;
        }
    }
}
namespace android {
    namespace os {
        class Build;
        class IBinder;
    }
    namespace view {
        class Window;
        class View;
        namespace inputmethod {
            class InputMethodManager;
        }
    }
    namespace content {
        class Context;
        class Intent;
        class ContextWrapper;
    }
    namespace app {
        class NativeActivity;
        class Activity;
    }
}
namespace java {
    namespace lang {
        class String;
        class Object;
        class ClassLoader;
        class Class;
    }
    namespace io {
        class File;
    }
}
namespace org {
    namespace apache {
        namespace http {
            class Header;
        }
    }
}
class XBLoginCallback;
}
using namespace jnivm;

class jnivm::XBLoginCallback : public jnivm::java::lang::Object {
public:
    jlong userptr;
    std::string cid;
    std::string token;
    jnivm::java::lang::Class* cl;
    void(*auth_flow_callback)(JNIEnv *env, void*, jlong paramLong, jint paramInt, jstring paramString);
    void onLogin(JNIEnv *, jlong, jboolean);
    void onError(JNIEnv *, jint, jint, jnivm::java::lang::String*);
    void onSuccess(JNIEnv *);
};
class com::mojang::minecraftpe::MainActivity : public jnivm::java::lang::Object {
    bool currentTextMutliline = false;
    std::string currentText;
    size_t currentTextPosition = 0;
    size_t currentTextPositionUTF = 0;
    size_t currentTextCopyPosition = 0;
    bool isShiftPressed = false;
    int iskeyboardvisible = 0;
    size_t maxcurrentTextLength = 0;
    void(*nativeOnPickImageSuccess)(JNIEnv*, void*, jlong var1, jstring var3);
    void(*nativeOnPickImageCanceled)(JNIEnv*, void*, jlong var1);
    void(*nativeSetTextboxText)(JNIEnv*,void*, jstring);
    unsigned char* (*stbi_load_from_memory)(unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
    void (*stbi_image_free)(void *retval_from_stbi_load);
    void (*nativeWebRequestCompleted)(JNIEnv*,void*, jint paramInt1, jlong paramLong, jint paramInt2, jstring paramString);

public:
    MainActivity(void * handle);
    std::shared_ptr<GameWindow> window;
    enum DirectionKey {
        LeftKey, RightKey, HomeKey, EndKey
    };
    void onKeyboardText(JNIEnv *,std::string const &text);
    void onKeyboardDirectionKey(DirectionKey key);
    void onKeyboardShiftKey(bool shiftPressed);
    void copyCurrentText();
    bool isKeyboardMultiline() const { return currentTextMutliline; }
    bool isKeyboardVisible() const { return iskeyboardvisible; }
    
    static void saveScreenshot(JNIEnv *, jclass, jnivm::java::lang::String*, jint, jint, jnivm::Array<jint>*);
    void postScreenshotToFacebook(JNIEnv *, jnivm::java::lang::String*, jint, jint, jnivm::Array<jint>*);
    jnivm::Array<jint>* getImageData(JNIEnv *, jnivm::java::lang::String*);
    jnivm::Array<jbyte>* getFileDataBytes(JNIEnv *, jnivm::java::lang::String*);
    void displayDialog(JNIEnv *, jint);
    void tick(JNIEnv *);
    void quit(JNIEnv *);
    void initiateUserInput(JNIEnv *, jint);
    jint getUserInputStatus(JNIEnv *);
    jnivm::Array<jnivm::java::lang::String*>* getUserInputString(JNIEnv *);
    jint checkLicense(JNIEnv *);
    jboolean hasBuyButtonWhenInvalidLicense(JNIEnv *);
    void buyGame(JNIEnv *);
    void vibrate(JNIEnv *, jint);
    void setIsPowerVR(JNIEnv *, jboolean);
    jboolean isNetworkEnabled(JNIEnv *, jboolean);
    jfloat getPixelsPerMillimeter(JNIEnv *);
    jnivm::java::lang::String* getPlatformStringVar(JNIEnv *, jint);
    jnivm::java::lang::Object* getSystemService(JNIEnv *, jnivm::java::lang::String*);
    jnivm::android::view::Window* getWindow(JNIEnv *);
    jint getKeyFromKeyCode(JNIEnv *, jint, jint, jint);
    void updateLocalization(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*);
    void showKeyboard(JNIEnv *, jnivm::java::lang::String*, jint, jboolean, jboolean, jboolean);
    void hideKeyboard(JNIEnv *);
    jfloat getKeyboardHeight(JNIEnv *);
    void updateTextboxText(JNIEnv *, jnivm::java::lang::String*);
    jint getCursorPosition(JNIEnv *);
    jnivm::java::lang::String* getAccessToken(JNIEnv *);
    jnivm::java::lang::String* getClientId(JNIEnv *);
    jnivm::java::lang::String* getProfileId(JNIEnv *);
    jnivm::java::lang::String* getProfileName(JNIEnv *);
    jnivm::Array<jnivm::java::lang::String*>* getBroadcastAddresses(JNIEnv *);
    jnivm::Array<jnivm::java::lang::String*>* getIPAddresses(JNIEnv *);
    jlong getTotalMemory(JNIEnv *);
    jlong getMemoryLimit(JNIEnv *);
    jlong getUsedMemory(JNIEnv *);
    jlong getFreeMemory(JNIEnv *);
    void launchUri(JNIEnv *, jnivm::java::lang::String*);
    void setClipboard(JNIEnv *, jnivm::java::lang::String*);
    void share(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*);
    jnivm::android::content::Intent* createAndroidLaunchIntent(JNIEnv *);
    jlong calculateAvailableDiskFreeSpace(JNIEnv *, jnivm::java::lang::String*);
    jnivm::java::lang::String* getExternalStoragePath(JNIEnv *);
    void requestStoragePermission(JNIEnv *, jint);
    jboolean hasWriteExternalStoragePermission(JNIEnv *);
    void deviceIdCorrelationStart(JNIEnv *);
    jboolean isMixerCreateInstalled(JNIEnv *);
    void navigateToPlaystoreForMixerCreate(JNIEnv *);
    jboolean launchMixerCreateForBroadcast(JNIEnv *);
    jboolean isTTSEnabled(JNIEnv *);
    jnivm::com::mojang::minecraftpe::HardwareInformation* getHardwareInfo(JNIEnv *);
    void setCachedDeviceId(JNIEnv *, jnivm::java::lang::String*);
    void setLastDeviceSessionId(JNIEnv *, jnivm::java::lang::String*);
    jnivm::java::lang::String* getLastDeviceSessionId(JNIEnv *);
    jint getAPIVersion(JNIEnv *, jnivm::java::lang::String*);
    jnivm::java::lang::String* getSecureStorageKey(JNIEnv *, jnivm::java::lang::String*);
    void setSecureStorageKey(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*);
    void trackPurchaseEvent(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*);
    void sendBrazeEvent(JNIEnv *, jnivm::java::lang::String*);
    void sendBrazeEventWithProperty(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*, jint);
    void sendBrazeEventWithStringProperty(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*);
    void sendBrazeToastClick(JNIEnv *);
    void sendBrazeDialogButtonClick(JNIEnv *, jint);
    void pickImage(JNIEnv *, jlong);
    void setFileDialogCallback(JNIEnv *, jlong);
    jnivm::java::lang::String* getLegacyDeviceID(JNIEnv *);
    jnivm::java::lang::String* createUUID(JNIEnv *);
    jboolean hasHardwareKeyboard(JNIEnv *);
    void startTextToSpeech(JNIEnv *, jnivm::java::lang::String*);
    void stopTextToSpeech(JNIEnv *);
    jboolean isTextToSpeechInProgress(JNIEnv *);
    void setTextToSpeechEnabled(JNIEnv *, jboolean);
    jint getScreenWidth(JNIEnv *);
    jint getScreenHeight(JNIEnv *);
    jnivm::java::lang::String* getDeviceModel(JNIEnv *);
    jint getAndroidVersion(JNIEnv *);
    jnivm::java::lang::String* getLocale(JNIEnv *);
    jboolean isFirstSnooperStart(JNIEnv *);
    jboolean hasHardwareChanged(JNIEnv *);
    jboolean isTablet(JNIEnv *);
    jnivm::java::lang::ClassLoader* getClassLoader(JNIEnv *);
    void webRequest(JNIEnv * env, jint paramInt, jlong paramLong, jnivm::java::lang::String* paramString1, jnivm::java::lang::String* paramString2, jnivm::java::lang::String* paramString3, jnivm::java::lang::String* paramString4) {
        // std::thread([=]() {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        //     nativeWebRequestCompleted(env, clazz, paramInt, paramLong, 2, env->NewStringUTF(""));
        // }).detach();
    }
};
class com::mojang::minecraftpe::HardwareInformation : public jnivm::java::lang::Object {
public:
    static jnivm::java::lang::String* getDeviceModelName(JNIEnv *, jclass);
    static jnivm::java::lang::String* getAndroidVersion(JNIEnv *, jclass);
    static jnivm::java::lang::String* getCPUType(JNIEnv *, jclass);
    static jnivm::java::lang::String* getCPUName(JNIEnv *, jclass);
    static jnivm::java::lang::String* getCPUFeatures(JNIEnv *, jclass);
    static jint getNumCores(JNIEnv *, jclass);
    jnivm::java::lang::String* getSecureId(JNIEnv *);
    static jnivm::java::lang::String* getSerialNumber(JNIEnv *, jclass);
    static jnivm::java::lang::String* getBoard(JNIEnv *, jclass);
    jnivm::java::lang::String* getInstallerPackageName(JNIEnv *);
    jint getSignaturesHashCode(JNIEnv *);
    jboolean getIsRooted(JNIEnv *);
};
class com::mojang::minecraftpe::store::NativeStoreListener : public jnivm::java::lang::Object {
public:
    NativeStoreListener(JNIEnv *, jclass, jlong);
};
class com::mojang::minecraftpe::store::Product : public jnivm::java::lang::Object {
public:
    jnivm::java::lang::String* mId;
    jnivm::java::lang::String* mPrice;
    jnivm::java::lang::String* mCurrencyCode;
    jnivm::java::lang::String* mUnformattedPrice;
};
class com::mojang::minecraftpe::store::Purchase : public jnivm::java::lang::Object {
public:
    jnivm::java::lang::String* mProductId;
    jnivm::java::lang::String* mReceipt;
    jboolean mPurchaseActive;
};
class com::mojang::minecraftpe::store::StoreFactory : public jnivm::java::lang::Object {
public:
    static jnivm::com::mojang::minecraftpe::store::Store* createGooglePlayStore(JNIEnv *, jclass, jnivm::java::lang::String*, jnivm::com::mojang::minecraftpe::store::StoreListener*);
    static jnivm::com::mojang::minecraftpe::store::Store* createAmazonAppStore(JNIEnv *, jclass, jnivm::com::mojang::minecraftpe::store::StoreListener*, jboolean);
};
class com::mojang::minecraftpe::store::StoreListener : public jnivm::java::lang::Object {
public:
};
class com::mojang::minecraftpe::store::Store : public jnivm::java::lang::Object {
public:
    jnivm::java::lang::String* getStoreId(JNIEnv *);
    jnivm::java::lang::String* getProductSkuPrefix(JNIEnv *);
    jnivm::java::lang::String* getRealmsSkuPrefix(JNIEnv *);
    jboolean hasVerifiedLicense(JNIEnv *);
    jnivm::com::mojang::minecraftpe::store::ExtraLicenseResponseData* getExtraLicenseData(JNIEnv *);
    jboolean receivedLicenseResponse(JNIEnv *);
    void queryProducts(JNIEnv *, jnivm::Array<jnivm::java::lang::String*>*);
    void purchase(JNIEnv *, jnivm::java::lang::String*, jboolean, jnivm::java::lang::String*);
    void acknowledgePurchase(JNIEnv *, jnivm::java::lang::String*, jnivm::java::lang::String*);
    void queryPurchases(JNIEnv *);
    void destructor(JNIEnv *);
};
class com::mojang::minecraftpe::store::ExtraLicenseResponseData : public jnivm::java::lang::Object {
public:
    jlong getValidationTime(JNIEnv *);
    jlong getRetryUntilTime(JNIEnv *);
    jlong getRetryAttempts(JNIEnv *);
};

class com::mojang::android::net::HTTPResponse : public jnivm::java::lang::Object {
public:
    jint getStatus(JNIEnv *);
    jnivm::java::lang::String* getBody(JNIEnv *);
    jint getResponseCode(JNIEnv *);
    jnivm::Array<jnivm::org::apache::http::Header*>* getHeaders(JNIEnv *);
};
class com::mojang::android::net::HTTPRequest : public jnivm::java::lang::Object {
public:
    HTTPRequest(JNIEnv *, jclass);
    void setURL(JNIEnv *, jnivm::java::lang::String*);
    void setRequestBody(JNIEnv *, jnivm::java::lang::String*);
    void setCookieData(JNIEnv *, jnivm::java::lang::String*);
    void setContentType(JNIEnv *, jnivm::java::lang::String*);
    jnivm::com::mojang::android::net::HTTPResponse* send(JNIEnv *, jnivm::java::lang::String*);
    void abort(JNIEnv *);
};

class com::microsoft::xbox::idp::interop::Interop : public jnivm::java::lang::Object {
public:
    static jnivm::java::lang::String* GetLocalStoragePath(JNIEnv *, jclass, jnivm::android::content::Context*);
    static jnivm::java::lang::String* ReadConfigFile(JNIEnv *, jclass, jnivm::android::content::Context*);
    static jnivm::java::lang::String* getSystemProxy(JNIEnv *, jclass);
    static void InitCLL(JNIEnv *, jclass, jnivm::android::content::Context*, jnivm::java::lang::String*);
    static void LogTelemetrySignIn(JNIEnv *, jclass, jnivm::java::lang::String*, jnivm::java::lang::String*);
    static void InvokeMSA(JNIEnv *, jclass, jnivm::android::content::Context*, jint, jboolean, jnivm::java::lang::String*);
    static void InvokeAuthFlow(JNIEnv *, jclass, jlong, jnivm::android::app::Activity*, jboolean, jnivm::java::lang::String*);
    static jnivm::java::lang::String* getLocale(JNIEnv *, jclass);
    static void RegisterWithGNS(JNIEnv *, jclass, jnivm::android::content::Context*);
    static void LogCLL(JNIEnv *, jclass, jnivm::java::lang::String*, jnivm::java::lang::String*, jnivm::java::lang::String*);
};
class com::microsoft::xbox::idp::interop::LocalConfig : public jnivm::java::lang::Object {
public:
};
class com::microsoft::xbox::idp::interop::XboxLiveAppConfig : public jnivm::java::lang::Object {
public:
};

class com::microsoft::xbox::idp::util::HttpCall : public jnivm::java::lang::Object {
public:
};
class com::microsoft::xbox::idp::util::AuthFlowResult : public jnivm::java::lang::Object {
public:
};



class com::microsoft::xboxtcui::Interop : public jnivm::java::lang::Object {
public:
    static void ShowFriendFinder(JNIEnv *, jclass, jnivm::android::app::Activity*, jnivm::java::lang::String*, jnivm::java::lang::String*);
    static void ShowUserSettings(JNIEnv *, jclass, jnivm::android::content::Context*);
    static void ShowUserProfile(JNIEnv *, jclass, jnivm::android::content::Context*, jnivm::java::lang::String*);
};


class android::os::Build : public jnivm::java::lang::Object {
public:
    class VERSION;
};
class android::os::Build::VERSION : public jnivm::java::lang::Object {
public:
    static jint SDK_INT;
};
class android::os::IBinder : public jnivm::java::lang::Object {
public:
};

class android::view::Window : public jnivm::java::lang::Object {
public:
    jnivm::android::view::View* getDecorView(JNIEnv *);
};
class android::view::View : public jnivm::java::lang::Object {
public:
    jnivm::android::os::IBinder* getWindowToken(JNIEnv *);
};
class android::view::inputmethod::InputMethodManager : public jnivm::java::lang::Object {
public:
    jboolean showSoftInput(JNIEnv *, jnivm::android::view::View*, jint);
    jboolean hideSoftInputFromWindow(JNIEnv *, jnivm::android::os::IBinder*, jint);
};


class android::content::Context : public jnivm::java::lang::Object {
public:
    static jnivm::java::lang::String* INPUT_METHOD_SERVICE;
    void startActivity(JNIEnv *, jnivm::android::content::Intent*);
    jnivm::java::lang::String* getPackageName(JNIEnv *);
};
class android::content::Intent : public jnivm::java::lang::Object {
public:
};
class android::content::ContextWrapper : public jnivm::java::lang::Object {
public:
    jnivm::java::io::File* getFilesDir(JNIEnv *);
    jnivm::java::io::File* getCacheDir(JNIEnv *);
};

class android::app::NativeActivity : public jnivm::java::lang::Object {
public:
    jnivm::android::content::Context* getApplicationContext(JNIEnv *);
};
class android::app::Activity : public jnivm::java::lang::Object {
public:
};

class java::lang::ClassLoader : public jnivm::java::lang::Object {
public:
    jnivm::java::lang::Class* loadClass(JNIEnv *, jnivm::java::lang::String*);
};

class java::io::File : public jnivm::java::lang::String {
public:
    using jnivm::java::lang::String::String;
    jnivm::java::lang::String* getPath(JNIEnv *);
};