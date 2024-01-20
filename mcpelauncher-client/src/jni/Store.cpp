#include "../JNIBinding.h"

jlong nativestore = 0;

com::mojang::minecraftpe::store::NativeStoreListener::NativeStoreListener(JNIEnv *env, jclass clazz, jlong arg0) {
    nativestore = arg0;
}

jnivm::com::mojang::minecraftpe::store::Store* com::mojang::minecraftpe::store::StoreFactory::createGooglePlayStore(JNIEnv *env, jclass clazz, jnivm::java::lang::String* arg0, jnivm::com::mojang::minecraftpe::store::StoreListener* arg1) {
    auto store = new jnivm::com::mojang::minecraftpe::store::Store();
    store->clazz = (jnivm::java::lang::Class*)env->FindClass("com/mojang/minecraftpe/store/Store");
    auto callback = (void(*)(JNIEnv*,jnivm::com::mojang::minecraftpe::store::StoreListener*, jlong, jboolean)) hybris_dlsym(env->functions->reserved3, "Java_com_mojang_minecraftpe_store_NativeStoreListener_onStoreInitialized");
    callback(env, arg1, nativestore, true);
    return store;
}

jnivm::com::mojang::minecraftpe::store::Store* com::mojang::minecraftpe::store::StoreFactory::createAmazonAppStore(JNIEnv *env, jclass clazz, jnivm::com::mojang::minecraftpe::store::StoreListener* arg0, jboolean arg1) {
    auto store = new jnivm::com::mojang::minecraftpe::store::Store();
    store->clazz = (jnivm::java::lang::Class*)env->FindClass("com/mojang/minecraftpe/store/Store");
    auto callback = (void(*)(JNIEnv*,jnivm::com::mojang::minecraftpe::store::StoreListener*, jlong, jboolean)) hybris_dlsym(env->functions->reserved3, "Java_com_mojang_minecraftpe_store_NativeStoreListener_onStoreInitialized");
    callback(env, arg0, nativestore, true);
    return store;
}

jnivm::java::lang::String* com::mojang::minecraftpe::store::Store::getStoreId(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("android.googleplay");
}

jnivm::java::lang::String* com::mojang::minecraftpe::store::Store::getProductSkuPrefix(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("com.linux");
}

jnivm::java::lang::String* com::mojang::minecraftpe::store::Store::getRealmsSkuPrefix(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("com.linux");
}

jboolean com::mojang::minecraftpe::store::Store::hasVerifiedLicense(JNIEnv *env) {
    return true;
}

jnivm::com::mojang::minecraftpe::store::ExtraLicenseResponseData* com::mojang::minecraftpe::store::Store::getExtraLicenseData(JNIEnv *env) {
    return 0;
}

jboolean com::mojang::minecraftpe::store::Store::receivedLicenseResponse(JNIEnv *env) {
    return true;
}

void com::mojang::minecraftpe::store::Store::queryProducts(JNIEnv *env, jnivm::Array<jnivm::java::lang::String*>* arg0) {
    
}

void com::mojang::minecraftpe::store::Store::purchase(JNIEnv *env, jnivm::java::lang::String* arg0, jboolean arg1, jnivm::java::lang::String* arg2) {
    
}

void com::mojang::minecraftpe::store::Store::acknowledgePurchase(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1) {
    
}

void com::mojang::minecraftpe::store::Store::queryPurchases(JNIEnv *env) {
    
}

void com::mojang::minecraftpe::store::Store::destructor(JNIEnv *env) {
    
}

jlong com::mojang::minecraftpe::store::ExtraLicenseResponseData::getValidationTime(JNIEnv *env) {
    return 0;
}

jlong com::mojang::minecraftpe::store::ExtraLicenseResponseData::getRetryUntilTime(JNIEnv *env) {
    return 0;    
}

jlong com::mojang::minecraftpe::store::ExtraLicenseResponseData::getRetryAttempts(JNIEnv *env) {
    return 0;    
}

// Entry points for jnivm

extern "C" jobject jnivm_com_mojang_minecraftpe_store_NativeStoreListener_NativeStoreListener(JNIEnv *env, jclass clazz, jvalue* values) {
    return (jobject)new jnivm::com::mojang::minecraftpe::store::NativeStoreListener(env, clazz, (jlong&)values[0]);
}
extern "C" jnivm::java::lang::String*  get_jnivm_com_mojang_minecraftpe_store_Product_mId(jnivm::com::mojang::minecraftpe::store::Product* obj) {
    return obj->mId;
}
extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Product_mId(jnivm::com::mojang::minecraftpe::store::Product* obj, jnivm::java::lang::String* value) {
    obj->mId = value;
}
extern "C" jnivm::java::lang::String*  get_jnivm_com_mojang_minecraftpe_store_Product_mPrice(jnivm::com::mojang::minecraftpe::store::Product* obj) {
    return obj->mPrice;
}
extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Product_mPrice(jnivm::com::mojang::minecraftpe::store::Product* obj, jnivm::java::lang::String* value) {
    obj->mPrice = value;
}
extern "C" jnivm::java::lang::String*  get_jnivm_com_mojang_minecraftpe_store_Product_mCurrencyCode(jnivm::com::mojang::minecraftpe::store::Product* obj) {
    return obj->mCurrencyCode;
}
extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Product_mCurrencyCode(jnivm::com::mojang::minecraftpe::store::Product* obj, jnivm::java::lang::String* value) {
    obj->mCurrencyCode = value;
}
extern "C" jnivm::java::lang::String*  get_jnivm_com_mojang_minecraftpe_store_Product_mUnformattedPrice(jnivm::com::mojang::minecraftpe::store::Product* obj) {
    return obj->mUnformattedPrice;
}
extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Product_mUnformattedPrice(jnivm::com::mojang::minecraftpe::store::Product* obj, jnivm::java::lang::String* value) {
    obj->mUnformattedPrice = value;
}
extern "C" jnivm::java::lang::String*  get_jnivm_com_mojang_minecraftpe_store_Purchase_mProductId(jnivm::com::mojang::minecraftpe::store::Purchase* obj) {
    return obj->mProductId;
}
extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Purchase_mProductId(jnivm::com::mojang::minecraftpe::store::Purchase* obj, jnivm::java::lang::String* value) {
    obj->mProductId = value;
}
extern "C" jnivm::java::lang::String*  get_jnivm_com_mojang_minecraftpe_store_Purchase_mReceipt(jnivm::com::mojang::minecraftpe::store::Purchase* obj) {
    return obj->mReceipt;
}

extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Purchase_mReceipt(jnivm::com::mojang::minecraftpe::store::Purchase* obj, jnivm::java::lang::String* value) {
    obj->mReceipt = value;
}
extern "C" jboolean  get_jnivm_com_mojang_minecraftpe_store_Purchase_mPurchaseActive(jnivm::com::mojang::minecraftpe::store::Purchase* obj) {
    return obj->mPurchaseActive;
}
extern "C" void  set_jnivm_com_mojang_minecraftpe_store_Purchase_mPurchaseActive(jnivm::com::mojang::minecraftpe::store::Purchase* obj, jboolean value) {
    obj->mPurchaseActive = value;
}
extern "C" jnivm::com::mojang::minecraftpe::store::Store* jnivm_com_mojang_minecraftpe_store_StoreFactory_createGooglePlayStore(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::store::StoreFactory::createGooglePlayStore(env, clazz, (jnivm::java::lang::String*&)values[0], (jnivm::com::mojang::minecraftpe::store::StoreListener*&)values[1]);
}
extern "C" jnivm::com::mojang::minecraftpe::store::Store* jnivm_com_mojang_minecraftpe_store_StoreFactory_createAmazonAppStore(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::store::StoreFactory::createAmazonAppStore(env, clazz, (jnivm::com::mojang::minecraftpe::store::StoreListener*&)values[0], (jboolean&)values[1]);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_store_Store_getStoreId(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->getStoreId(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_store_Store_getProductSkuPrefix(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->getProductSkuPrefix(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_store_Store_getRealmsSkuPrefix(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->getRealmsSkuPrefix(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_store_Store_hasVerifiedLicense(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->hasVerifiedLicense(env);
}
extern "C" jnivm::com::mojang::minecraftpe::store::ExtraLicenseResponseData* jnivm_com_mojang_minecraftpe_store_Store_getExtraLicenseData(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->getExtraLicenseData(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_store_Store_receivedLicenseResponse(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->receivedLicenseResponse(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_store_Store_queryProducts(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->queryProducts(env, (jnivm::Array<jnivm::java::lang::String*>*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_store_Store_purchase(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->purchase(env, (jnivm::java::lang::String*&)values[0], (jboolean&)values[1], (jnivm::java::lang::String*&)values[2]);
}
extern "C" void jnivm_com_mojang_minecraftpe_store_Store_acknowledgePurchase(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->acknowledgePurchase(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1]);
}
extern "C" void jnivm_com_mojang_minecraftpe_store_Store_queryPurchases(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->queryPurchases(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_store_Store_destructor(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::Store* obj, jvalue* values) {
    return obj->destructor(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_store_ExtraLicenseResponseData_getValidationTime(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::ExtraLicenseResponseData* obj, jvalue* values) {
    return obj->getValidationTime(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_store_ExtraLicenseResponseData_getRetryUntilTime(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::ExtraLicenseResponseData* obj, jvalue* values) {
    return obj->getRetryUntilTime(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_store_ExtraLicenseResponseData_getRetryAttempts(JNIEnv *env, jnivm::com::mojang::minecraftpe::store::ExtraLicenseResponseData* obj, jvalue* values) {
    return obj->getRetryAttempts(env);
}