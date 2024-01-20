#include "../JNIBinding.h"

// Needed for loading pictures
jint android::os::Build::VERSION::SDK_INT = 28;

jnivm::android::view::View* android::view::Window::getDecorView(JNIEnv *env) {
    auto view = new jnivm::android::view::View();
    view->clazz = (jnivm::java::lang::Class*)env->FindClass("android/view/View");
    return view;
}

jnivm::android::os::IBinder* android::view::View::getWindowToken(JNIEnv *env) {
    auto ib = new jnivm::android::os::IBinder();
    ib->clazz = (jnivm::java::lang::Class*)env->FindClass("android/os/IBinder");
    return ib;
}

jnivm::java::lang::String* android::content::Context::INPUT_METHOD_SERVICE = {};

void android::content::Context::startActivity(JNIEnv *env, jnivm::android::content::Intent* arg0) {
    
}
jnivm::java::lang::String* android::content::Context::getPackageName(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jnivm::java::io::File* android::content::ContextWrapper::getFilesDir(JNIEnv *env) {
    return new jnivm::java::io::File { "" };
}

jnivm::java::io::File* android::content::ContextWrapper::getCacheDir(JNIEnv *env) {
    return new jnivm::java::io::File { PathHelper::getCacheDirectory() };
}

jnivm::android::content::Context* android::app::NativeActivity::getApplicationContext(JNIEnv *env) {
    auto ctx = new jnivm::android::content::Context();
    ctx->clazz = (jnivm::java::lang::Class*)env->FindClass("android/content/Context");
    return ctx;
}

jnivm::java::lang::Class* java::lang::ClassLoader::loadClass(JNIEnv *env, jnivm::java::lang::String* arg0) {
    return (jnivm::java::lang::Class*)env->FindClass(arg0->data());
}

jnivm::java::lang::String* java::io::File::getPath(JNIEnv *env) {
    return this;
}

// Entry points for jnivm

extern "C" jint  get_jnivm_android_os_Build_VERSION_SDK_INT() {
    return android::os::Build::VERSION::SDK_INT;
}
extern "C" void  set_jnivm_android_os_Build_VERSION_SDK_INT(jint value) {
    android::os::Build::VERSION::SDK_INT = value;
}
extern "C" jnivm::android::view::View* jnivm_android_view_Window_getDecorView(JNIEnv *env, jnivm::android::view::Window* obj, jvalue* values) {
    return obj->getDecorView(env);
}
extern "C" jnivm::android::os::IBinder* jnivm_android_view_View_getWindowToken(JNIEnv *env, jnivm::android::view::View* obj, jvalue* values) {
    return obj->getWindowToken(env);
}
extern "C" jnivm::java::lang::String*  get_jnivm_android_content_Context_INPUT_METHOD_SERVICE() {
    return android::content::Context::INPUT_METHOD_SERVICE;
}
extern "C" void  set_jnivm_android_content_Context_INPUT_METHOD_SERVICE(jnivm::java::lang::String* value) {
    android::content::Context::INPUT_METHOD_SERVICE = value;
}
extern "C" void jnivm_android_content_Context_startActivity(JNIEnv *env, jnivm::android::content::Context* obj, jvalue* values) {
    return obj->startActivity(env, (jnivm::android::content::Intent*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_android_content_Context_getPackageName(JNIEnv *env, jnivm::android::content::Context* obj, jvalue* values) {
    return obj->getPackageName(env);
}
extern "C" jnivm::java::io::File* jnivm_android_content_ContextWrapper_getFilesDir(JNIEnv *env, jnivm::android::content::ContextWrapper* obj, jvalue* values) {
    return obj->getFilesDir(env);
}
extern "C" jnivm::java::io::File* jnivm_android_content_ContextWrapper_getCacheDir(JNIEnv *env, jnivm::android::content::ContextWrapper* obj, jvalue* values) {
    return obj->getCacheDir(env);
}
extern "C" jnivm::android::content::Context* jnivm_android_app_NativeActivity_getApplicationContext(JNIEnv *env, jnivm::android::app::NativeActivity* obj, jvalue* values) {
    return obj->getApplicationContext(env);
}
extern "C" jnivm::java::lang::Class* jnivm_java_lang_ClassLoader_loadClass(JNIEnv *env, jnivm::java::lang::ClassLoader* obj, jvalue* values) {
    return obj->loadClass(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_java_io_File_getPath(JNIEnv *env, jnivm::java::io::File* obj, jvalue* values) {
    return obj->getPath(env);
}