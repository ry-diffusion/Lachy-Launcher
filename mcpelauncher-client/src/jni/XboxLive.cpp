#include "../JNIBinding.h"
#include <log.h>

void XBLoginCallback::onLogin(JNIEnv *env, jlong arg0, jboolean arg1) {
    auto invoke_event_initialization = (void (*)(JNIEnv *env, jclass, jlong var0, jstring var2, XBLoginCallback* var3))cl->natives["invoke_event_initialization"];
    auto XBLoginCallbackcl = env->FindClass("XBLoginCallback");
    invoke_event_initialization(env, nullptr, userptr, env->NewStringUTF(token.data()), this);
}

void XBLoginCallback::onSuccess(JNIEnv *env) {
    auth_flow_callback(env, nullptr, userptr, /* No Error */0, env->NewStringUTF(cid.data()));    
}

void XBLoginCallback::onError(JNIEnv *env, jint arg0, jint arg1, jnivm::java::lang::String* arg2) {
    Log::error("XboxLive", "Sign in error: %s", arg2->data());
    auth_flow_callback(env, nullptr, userptr, /* Failed */2, nullptr);        
}

jnivm::java::lang::String* com::microsoft::xbox::idp::interop::Interop::GetLocalStoragePath(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0) {
    return (jnivm::java::lang::String*)env->NewStringUTF(PathHelper::getPrimaryDataDirectory().data());
}

jnivm::java::lang::String* com::microsoft::xbox::idp::interop::Interop::ReadConfigFile(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0) {
    Log::trace("XBOXLive", "Reading xbox config file");
    std::ifstream f("xboxservices.config");
    std::stringstream s;
    s << f.rdbuf();
    return (jnivm::java::lang::String*)env->NewStringUTF(s.str().data());
}

// Sets proxy for Xboxlive, "" for no proxy server
jnivm::java::lang::String* com::microsoft::xbox::idp::interop::Interop::getSystemProxy(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

void * get_uploader_x_token_callback = 0;
void * get_supporting_x_token_callback = 0;

void com::microsoft::xbox::idp::interop::Interop::InitCLL(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0, jnivm::java::lang::String* arg1) {
    get_uploader_x_token_callback = ((jnivm::java::lang::Class*)clazz)->natives["get_uploader_x_token_callback"];
    get_supporting_x_token_callback = ((jnivm::java::lang::Class*)clazz)->natives["get_supporting_x_token_callback"];
    XboxLiveHelper::getInstance().initCll();
}

void com::microsoft::xbox::idp::interop::Interop::LogTelemetrySignIn(JNIEnv *env, jclass clazz, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1) {
    Log::info("com::microsoft::xbox::idp::interop::Interop::LogTelemetrySignIn", "%s:%s", arg0->data(), arg1->data());    
}

void com::microsoft::xbox::idp::interop::Interop::InvokeMSA(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0, jint requestCode, jboolean arg2, jnivm::java::lang::String* cid) {
    auto cl = (jnivm::java::lang::Class*)clazz;
    // auto appconfig = (jnivm::java::lang::Class*)env->FindClass("com/microsoft/xbox/idp/interop/XboxLiveAppConfig");
    // auto id = ((jlong(*)(JNIEnv * env, void*))appconfig->natives["create"])(env, nullptr);
    // auto titleid = ((jint(*)(JNIEnv * env, void*, jlong))appconfig->natives["getTitleId"])(env, nullptr, id);
    // auto scid = ((jnivm::java::lang::String*(*)(JNIEnv * env, void*, jlong))appconfig->natives["getScid"])(env, nullptr, id);
    // auto sandbox = ((jnivm::java::lang::String*(*)(JNIEnv * env, void*, jlong))appconfig->natives["getSandbox"])(env, nullptr, id);
    // auto proxy = ((jnivm::java::lang::String*(*)(JNIEnv * env, void*, jlong))appconfig->natives["getProxy"])(env, nullptr, id);
    // auto overrideTitleId = ((jint(*)(JNIEnv * env, void*, jlong))appconfig->natives["getOverrideTitleId"])(env, nullptr, id);
    // auto environment = ((jnivm::java::lang::String*(*)(JNIEnv * env, void*, jlong))appconfig->natives["getEnvironment"])(env, nullptr, id);
    // ((void(*)(JNIEnv * env, void*, jlong))appconfig->natives["delete"])(env, nullptr, id);
    auto ticket_callback = ((void(*)(JNIEnv *env, void*, jstring paramString1, jint paramInt1, jint paramInt2, jstring paramString2))cl->natives["ticket_callback"]);
    if (requestCode == 1) { // silent signin
        if (!cid->empty()) {
            try {
                XboxLiveHelper::getInstance().requestXblToken(*cid, true,
                    [env,ticket_callback](std::string const& cid, std::string const& token) {
                        XboxLiveHelper::getInstance().initCll(cid);
                        ticket_callback(env, nullptr, env->NewStringUTF(token.data()), 0, /* Error None */ 0, env->NewStringUTF("Got ticket"));
                    }, [=](simpleipc::rpc_error_code err, std::string const& msg) {
                        Log::error("XboxLive", "Auto Sign in failed (RPC): %s", msg.c_str());
                        if (err == -100) { // No such account
                            ticket_callback(env, nullptr, env->NewStringUTF(""), 0, /* Error No such account */ 1, env->NewStringUTF("Must show UI to acquire an account."));
                        } else if (err == -102) { // Must show UI
                            ticket_callback(env, nullptr, env->NewStringUTF(""), 0, /* Error Must show UI */ 1, env->NewStringUTF("Must show UI to update account information."));
                        } else {
                            ticket_callback(env, nullptr, env->NewStringUTF(""), 0, /* Error */ 1, env->NewStringUTF(msg.c_str()));
                        }
                    });
        } catch(const std::exception& ex) {
            Log::error("XboxLive", "Auto Sign in error (exception): %s", ex.what());
            ticket_callback(env, nullptr, env->NewStringUTF(""), 0, /* Error */ 1, nullptr);
        }
    } else {
            ticket_callback(env, nullptr, env->NewStringUTF(""), requestCode, /* Error No such account */ 1, env->NewStringUTF("Must show UI to acquire an account."));
        }
    } else if (requestCode == 6) { // sign out
        ((void(*)(JNIEnv*, void*))cl->natives["sign_out_callback"])(env, nullptr);
    }
}

void com::microsoft::xbox::idp::interop::Interop::InvokeAuthFlow(JNIEnv *env, jclass clazz, jlong userptr, jnivm::android::app::Activity* arg1, jboolean arg2, jnivm::java::lang::String* arg3) {
    auto cl = (jnivm::java::lang::Class*) clazz;
    auto auth_flow_callback = (void(*)(JNIEnv *env, void*, jlong paramLong, jint paramInt, jstring paramString))cl->natives["auth_flow_callback"];
    auto invoke_xb_login = (void(*)(JNIEnv*, void*, jlong paramLong, jstring paramString, jobject))cl->natives["invoke_xb_login"];
    auto onsuccess = [env, auth_flow_callback, userptr, invoke_xb_login,cl](std::string const& cid, std::string const& token) {
        auto XBLoginCallbackcl = env->FindClass("XBLoginCallback");
        auto xblc = new XBLoginCallback();
        xblc->clazz = (jnivm::java::lang::Class*)XBLoginCallbackcl;
        xblc->userptr = userptr;
        xblc->cid = cid;
        xblc->token = token;
        xblc->cl = cl;
        xblc->auth_flow_callback = auth_flow_callback;
        invoke_xb_login(env, nullptr, userptr, env->NewStringUTF(token.data()), (jobject)xblc);
    };
    auto onfail = [=](simpleipc::rpc_error_code rpc, std::string const& msg) {
        Log::error("XboxLive", "Sign in error (RPC): %s", msg.c_str());
        auth_flow_callback(env, nullptr, userptr, /* Failed */2, nullptr);
    };

    try {
        XboxLiveHelper::getInstance().invokeMsaAuthFlow(onsuccess, onfail);
    } catch(const std::exception& ex) {
        Log::error("XboxLive", "Sign in error (exception): %s", ex.what());
        auth_flow_callback(env, nullptr, userptr, /* Failed */2, nullptr);
    }
    
}

jnivm::java::lang::String* com::microsoft::xbox::idp::interop::Interop::getLocale(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("en");
}

void com::microsoft::xbox::idp::interop::Interop::RegisterWithGNS(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0) {
    
}

void com::microsoft::xbox::idp::interop::Interop::LogCLL(JNIEnv *env, jclass clazz, jnivm::java::lang::String* ticket, jnivm::java::lang::String* name, jnivm::java::lang::String* data) {
    Log::trace("com::microsoft::xbox::idp::interop::Interop::LogCLL", "log_cll %s %s %s", ticket->c_str(), name->c_str(), data->c_str());
    cll::Event event(*name, nlohmann::json::parse(*data),
                     cll::EventFlags::PersistenceCritical | cll::EventFlags::LatencyRealtime, {*ticket});
    XboxLiveHelper::getInstance().logCll(event);
}

#ifdef __APPLE__
#define OPENBROWSER "open"
#elif defined(_WIN32)
#define OPENBROWSER "start"
#else
#define OPENBROWSER "xdg-open"
#endif
void com::microsoft::xboxtcui::Interop::ShowFriendFinder(JNIEnv *env, jclass clazz, jnivm::android::app::Activity* arg0, jnivm::java::lang::String* arg1, jnivm::java::lang::String* arg2) {
    system(OPENBROWSER" https://account.xbox.com/en-us/Friends");
    ((void(*)(JNIEnv *env, jclass clazz, jint))((jnivm::java::lang::Class*)clazz)->natives["tcui_completed_callback"])(env, clazz, 0);
}

void jnivm::com::microsoft::xboxtcui::Interop::ShowUserSettings(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0) {
    system(OPENBROWSER" https://account.xbox.com/en-us/Settings");
    ((void(*)(JNIEnv *env, jclass clazz, jint))((jnivm::java::lang::Class*)clazz)->natives["tcui_completed_callback"])(env, clazz, 0);
}

void jnivm::com::microsoft::xboxtcui::Interop::ShowUserProfile(JNIEnv *env, jclass clazz, jnivm::android::content::Context* arg0, jnivm::java::lang::String* arg1) {
    system(OPENBROWSER" https://account.xbox.com/en-us/Profile");
    ((void(*)(JNIEnv *env, jclass clazz, jint))((jnivm::java::lang::Class*)clazz)->natives["tcui_completed_callback"])(env, clazz, 0);
}
#undef OPENBROWSER

// Entry points for jnivm

extern "C" void jnivm_XBLoginCallback_onLogin(JNIEnv *env, jnivm::XBLoginCallback* obj, jvalue* values) {
    return obj->onLogin(env, (jlong&)values[0], (jboolean&)values[1]);
}
extern "C" void jnivm_XBLoginCallback_onSuccess(JNIEnv *env, jnivm::XBLoginCallback* obj, jvalue* values) {
    return obj->onSuccess(env);
}
extern "C" void jnivm_XBLoginCallback_onError(JNIEnv *env, jnivm::XBLoginCallback* obj, jvalue* values) {
    return obj->onError(env, (jint&)values[0], (jint&)values[1], (jnivm::java::lang::String*&)values[2]);
}
extern "C" jnivm::java::lang::String* jnivm_com_microsoft_xbox_idp_interop_Interop_GetLocalStoragePath(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::GetLocalStoragePath(env, clazz, (jnivm::android::content::Context*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_com_microsoft_xbox_idp_interop_Interop_ReadConfigFile(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::ReadConfigFile(env, clazz, (jnivm::android::content::Context*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_com_microsoft_xbox_idp_interop_Interop_getSystemProxy(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::getSystemProxy(env, clazz);
}
extern "C" void jnivm_com_microsoft_xbox_idp_interop_Interop_InitCLL(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::InitCLL(env, clazz, (jnivm::android::content::Context*&)values[0], (jnivm::java::lang::String*&)values[1]);
}
extern "C" void jnivm_com_microsoft_xbox_idp_interop_Interop_LogTelemetrySignIn(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::LogTelemetrySignIn(env, clazz, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1]);
}
extern "C" void jnivm_com_microsoft_xbox_idp_interop_Interop_InvokeMSA(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::InvokeMSA(env, clazz, (jnivm::android::content::Context*&)values[0], (jint&)values[1], (jboolean&)values[2], (jnivm::java::lang::String*&)values[3]);
}
extern "C" void jnivm_com_microsoft_xbox_idp_interop_Interop_InvokeAuthFlow(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::InvokeAuthFlow(env, clazz, (jlong&)values[0], (jnivm::android::app::Activity*&)values[1], (jboolean&)values[2], (jnivm::java::lang::String*&)values[3]);
}
extern "C" jnivm::java::lang::String* jnivm_com_microsoft_xbox_idp_interop_Interop_getLocale(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::getLocale(env, clazz);
}
extern "C" void jnivm_com_microsoft_xbox_idp_interop_Interop_RegisterWithGNS(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::RegisterWithGNS(env, clazz, (jnivm::android::content::Context*&)values[0]);
}
extern "C" void jnivm_com_microsoft_xbox_idp_interop_Interop_LogCLL(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xbox::idp::interop::Interop::LogCLL(env, clazz, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1], (jnivm::java::lang::String*&)values[2]);
}
extern "C" void jnivm_com_microsoft_xboxtcui_Interop_ShowFriendFinder(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::microsoft::xboxtcui::Interop::ShowFriendFinder(env, clazz, (jnivm::android::app::Activity*&)values[0], (jnivm::java::lang::String*&)values[1], (jnivm::java::lang::String*&)values[2]);
}
extern "C" void jnivm_com_microsoft_xboxtcui_Interop_ShowUserSettings(JNIEnv *env, jclass clazz, jvalue* values) {
    return jnivm::com::microsoft::xboxtcui::Interop::ShowUserSettings(env, clazz, (jnivm::android::content::Context*&)values[0]);
}
extern "C" void jnivm_com_microsoft_xboxtcui_Interop_ShowUserProfile(JNIEnv *env, jclass clazz, jvalue* values) {
    return jnivm::com::microsoft::xboxtcui::Interop::ShowUserProfile(env, clazz, (jnivm::android::content::Context*&)values[0], (jnivm::java::lang::String*&)values[1]);
}