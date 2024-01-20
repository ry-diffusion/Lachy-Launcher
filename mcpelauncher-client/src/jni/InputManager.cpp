#include "../JNIBinding.h"

jboolean android::view::inputmethod::InputMethodManager::showSoftInput(JNIEnv *env, jnivm::android::view::View* arg0, jint arg1) {
    return true;
}

jboolean android::view::inputmethod::InputMethodManager::hideSoftInputFromWindow(JNIEnv *env, jnivm::android::os::IBinder* arg0, jint arg1) {
    return true;
}

// Entry points for jnivm

extern "C" jboolean jnivm_android_view_inputmethod_InputMethodManager_showSoftInput(JNIEnv *env, jnivm::android::view::inputmethod::InputMethodManager* obj, jvalue* values) {
    return obj->showSoftInput(env, (jnivm::android::view::View*&)values[0], (jint&)values[1]);
}
extern "C" jboolean jnivm_android_view_inputmethod_InputMethodManager_hideSoftInputFromWindow(JNIEnv *env, jnivm::android::view::inputmethod::InputMethodManager* obj, jvalue* values) {
    return obj->hideSoftInputFromWindow(env, (jnivm::android::os::IBinder*&)values[0], (jint&)values[1]);
}