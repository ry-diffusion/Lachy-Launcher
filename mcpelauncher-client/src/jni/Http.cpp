#include "../JNIBinding.h"
// Never called since Minecraft 1.13
// Pre 1.13 compatibility

jint com::mojang::android::net::HTTPResponse::getStatus(JNIEnv *env) {
    return 1;
}

jnivm::java::lang::String* com::mojang::android::net::HTTPResponse::getBody(JNIEnv *env) {
    return 0;
}

jint com::mojang::android::net::HTTPResponse::getResponseCode(JNIEnv *env) {
    return 200;
}

jnivm::Array<jnivm::org::apache::http::Header*>* com::mojang::android::net::HTTPResponse::getHeaders(JNIEnv *env) {
    return 0;
}

com::mojang::android::net::HTTPRequest::HTTPRequest(JNIEnv *env, jclass clazz) {
    
}

void com::mojang::android::net::HTTPRequest::setURL(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::android::net::HTTPRequest::setRequestBody(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::android::net::HTTPRequest::setCookieData(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::android::net::HTTPRequest::setContentType(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

jnivm::com::mojang::android::net::HTTPResponse* com::mojang::android::net::HTTPRequest::send(JNIEnv *env, jnivm::java::lang::String* arg0) {
    return 0;
}

void com::mojang::android::net::HTTPRequest::abort(JNIEnv *env) {
    
}

// Entry points for jnivm

extern "C" jint jnivm_com_mojang_android_net_HTTPResponse_getStatus(JNIEnv *env, jnivm::com::mojang::android::net::HTTPResponse* obj, jvalue* values) {
    return obj->getStatus(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_android_net_HTTPResponse_getBody(JNIEnv *env, jnivm::com::mojang::android::net::HTTPResponse* obj, jvalue* values) {
    return obj->getBody(env);
}
extern "C" jint jnivm_com_mojang_android_net_HTTPResponse_getResponseCode(JNIEnv *env, jnivm::com::mojang::android::net::HTTPResponse* obj, jvalue* values) {
    return obj->getResponseCode(env);
}
extern "C" jnivm::Array<jnivm::org::apache::http::Header*>* jnivm_com_mojang_android_net_HTTPResponse_getHeaders(JNIEnv *env, jnivm::com::mojang::android::net::HTTPResponse* obj, jvalue* values) {
    return obj->getHeaders(env);
}
extern "C" jobject jnivm_com_mojang_android_net_HTTPRequest_HTTPRequest(JNIEnv *env, jclass clazz, jvalue* values) {
    return (jobject)new com::mojang::android::net::HTTPRequest(env, clazz);
}
extern "C" void jnivm_com_mojang_android_net_HTTPRequest_setURL(JNIEnv *env, jnivm::com::mojang::android::net::HTTPRequest* obj, jvalue* values) {
    return obj->setURL(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_android_net_HTTPRequest_setRequestBody(JNIEnv *env, jnivm::com::mojang::android::net::HTTPRequest* obj, jvalue* values) {
    return obj->setRequestBody(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_android_net_HTTPRequest_setCookieData(JNIEnv *env, jnivm::com::mojang::android::net::HTTPRequest* obj, jvalue* values) {
    return obj->setCookieData(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_android_net_HTTPRequest_setContentType(JNIEnv *env, jnivm::com::mojang::android::net::HTTPRequest* obj, jvalue* values) {
    return obj->setContentType(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::com::mojang::android::net::HTTPResponse* jnivm_com_mojang_android_net_HTTPRequest_send(JNIEnv *env, jnivm::com::mojang::android::net::HTTPRequest* obj, jvalue* values) {
    return obj->send(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_android_net_HTTPRequest_abort(JNIEnv *env, jnivm::com::mojang::android::net::HTTPRequest* obj, jvalue* values) {
    return obj->abort(env);
}