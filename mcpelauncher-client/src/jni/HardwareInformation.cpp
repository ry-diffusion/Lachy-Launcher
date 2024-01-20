#include "../JNIBinding.h"

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getDeviceModelName(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getAndroidVersion(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getCPUType(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getCPUName(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getCPUFeatures(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jint com::mojang::minecraftpe::HardwareInformation::getNumCores(JNIEnv *env, jclass clazz) {
    return 4;
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getSecureId(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("0000000000000000");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getSerialNumber(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getBoard(JNIEnv *env, jclass clazz) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::java::lang::String* com::mojang::minecraftpe::HardwareInformation::getInstallerPackageName(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("com.mojang.minecraftpe");
}

jint com::mojang::minecraftpe::HardwareInformation::getSignaturesHashCode(JNIEnv *env) {
    return 0xccccccc;
}

jboolean com::mojang::minecraftpe::HardwareInformation::getIsRooted(JNIEnv *env) {
    return false;
}

// Entry points for jnivm

extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getDeviceModelName(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getDeviceModelName(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getAndroidVersion(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getAndroidVersion(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getCPUType(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getCPUType(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getCPUName(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getCPUName(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getCPUFeatures(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getCPUFeatures(env, clazz);
}
extern "C" jint jnivm_com_mojang_minecraftpe_HardwareInformation_getNumCores(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getNumCores(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getSecureId(JNIEnv *env, jnivm::com::mojang::minecraftpe::HardwareInformation* obj, jvalue* values) {
    return obj->getSecureId(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getSerialNumber(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getSerialNumber(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getBoard(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::HardwareInformation::getBoard(env, clazz);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_HardwareInformation_getInstallerPackageName(JNIEnv *env, jnivm::com::mojang::minecraftpe::HardwareInformation* obj, jvalue* values) {
    return obj->getInstallerPackageName(env);
}
extern "C" jint jnivm_com_mojang_minecraftpe_HardwareInformation_getSignaturesHashCode(JNIEnv *env, jnivm::com::mojang::minecraftpe::HardwareInformation* obj, jvalue* values) {
    return obj->getSignaturesHashCode(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_HardwareInformation_getIsRooted(JNIEnv *env, jnivm::com::mojang::minecraftpe::HardwareInformation* obj, jvalue* values) {
    return obj->getIsRooted(env);
}