#include "../JNIBinding.h"
#include "../utf8_util.h"
#include "minecraft/Keyboard.h"

jnivm::com::mojang::minecraftpe::MainActivity::MainActivity(void * handle) {
    nativeOnPickImageSuccess = (decltype(nativeOnPickImageSuccess))hybris_dlsym(handle, "Java_com_mojang_minecraftpe_MainActivity_nativeOnPickImageSuccess");
    nativeOnPickImageCanceled = (decltype(nativeOnPickImageCanceled))hybris_dlsym(handle, "Java_com_mojang_minecraftpe_MainActivity_nativeOnPickImageCanceled");
    nativeSetTextboxText = (decltype(nativeSetTextboxText))hybris_dlsym(handle, "Java_com_mojang_minecraftpe_MainActivity_nativeSetTextboxText");
    stbi_load_from_memory = (decltype(stbi_load_from_memory))hybris_dlsym(handle, "stbi_load_from_memory");
    stbi_image_free = (decltype(stbi_image_free))hybris_dlsym(handle, "stbi_image_free");
    nativeWebRequestCompleted = (decltype(nativeWebRequestCompleted))hybris_dlsym(handle, "Java_com_mojang_minecraftpe_MainActivity_nativeWebRequestCompleted");
}

void jnivm::com::mojang::minecraftpe::MainActivity::onKeyboardText(JNIEnv *env, std::string const &text) {
    if (text.size() == 1 && text[0] == 8) { // backspace
        if (currentTextPositionUTF <= 0)
            return;
        currentTextPositionUTF--;
        auto deleteStart = currentTextPosition - 1;
        while (deleteStart > 0 && (currentText[deleteStart] & 0b11000000) == 0b10000000)
            deleteStart--;
        currentText.erase(currentText.begin() + deleteStart, currentText.begin() + currentTextPosition);
        currentTextPosition = deleteStart;
    } else if (text.size() == 1 && text[0] == 127) { // delete key
        if (currentTextPosition >= currentText.size())
            return;
        auto deleteEnd = currentTextPosition + 1;
        while (deleteEnd < currentText.size() && (currentText[deleteEnd] & 0b11000000) == 0b10000000)
            deleteEnd++;
        currentText.erase(currentText.begin() + currentTextPosition, currentText.begin() + deleteEnd);
    } else {
        size_t utf8length = 0;
        size_t length = 0;
        while (length < text.size() && utf8length < maxcurrentTextLength) {
            char c = text[length];
            length += UTF8Util::getCharByteSize(c);
            utf8length++;
        }
        currentText.insert(currentText.begin() + currentTextPosition, text.begin(), text.begin() + length);
        currentTextPosition += length;
        currentTextPositionUTF += utf8length;
    }
    if(nativeSetTextboxText) {
        nativeSetTextboxText(env, this, env->NewStringUTF(currentText.data()));
    } else {
        Log::error("MainActivity", "Cannot set text with nativeSetTextboxText");
    }
    currentTextCopyPosition = currentTextPosition;
}

void jnivm::com::mojang::minecraftpe::MainActivity::onKeyboardDirectionKey(DirectionKey key) {
    if (key == DirectionKey::RightKey) {
        if (currentTextPosition >= currentText.size())
            return;
        currentTextPosition++;
        while (currentTextPosition < currentText.size() &&
               (currentText[currentTextPosition] & 0b11000000) == 0b10000000)
            currentTextPosition++;
        currentTextPositionUTF++;
    } else if (key == DirectionKey::LeftKey) {
        if (currentTextPosition <= 0)
            return;
        currentTextPosition--;
        while (currentTextPosition > 0 && (currentText[currentTextPosition] & 0b11000000) == 0b10000000)
            currentTextPosition--;
        currentTextPositionUTF--;
    } else if (key == DirectionKey::HomeKey) {
        currentTextPosition = 0;
        currentTextPositionUTF = 0;
    } else if (key == DirectionKey::EndKey) {
        currentTextPosition = currentText.size();
        currentTextPositionUTF = UTF8Util::getLength(currentText.c_str(), currentTextPosition);
    }
    if (!isShiftPressed)
        currentTextCopyPosition = currentTextPosition;
}

void jnivm::com::mojang::minecraftpe::MainActivity::onKeyboardShiftKey(bool shiftPressed) {
    isShiftPressed = shiftPressed;
}

void jnivm::com::mojang::minecraftpe::MainActivity::copyCurrentText() {
    if (currentTextCopyPosition != currentTextPosition) {
        size_t start = std::min(currentTextPosition, currentTextCopyPosition);
        size_t end = std::max(currentTextPosition, currentTextCopyPosition);
        window->setClipboardText(currentText.substr(start, end - start));
    } else {
        window->setClipboardText(currentText);
    }
}

void com::mojang::minecraftpe::MainActivity::saveScreenshot(JNIEnv *env, jclass clazz, jnivm::java::lang::String* arg0, jint arg1, jint arg2, jnivm::Array<jint>* arg3) {
    
}

void com::mojang::minecraftpe::MainActivity::postScreenshotToFacebook(JNIEnv *env, jnivm::java::lang::String* arg0, jint arg1, jint arg2, jnivm::Array<jint>* arg3) {
    
}

jnivm::Array<jint>* com::mojang::minecraftpe::MainActivity::getImageData(JNIEnv *env, jnivm::java::lang::String* arg0) {
    if(!stbi_load_from_memory || !stbi_image_free) return 0;
    int width, height, channels;
    std::ifstream f(arg0->data());
    if(!f.is_open()) return 0;
    std::stringstream s;
    s << f.rdbuf();
    auto buf = s.str();
    auto image = stbi_load_from_memory((unsigned char*)buf.data(), buf.length(), &width, &height, &channels, 4);
    if(!image) return 0;
    auto ret = new jnivm::Array<jint>(new jint[2 + width * height] { 0 }, 2 + width * height);
    ret->data[0] = width;
    ret->data[1] = height;
    
    for(int x = 0; x < width * height; x++) {
        ret->data[2 + x] = (image[x * 4 + 2]) | (image[x * 4 + 1] << 8) | (image[x * 4 + 0] << 16) | (image[x * 4 + 3] << 24);
    }
    stbi_image_free(image);
    return ret;
}

// Implementation needed for Minecraft < 1.7
jnivm::Array<jbyte>* com::mojang::minecraftpe::MainActivity::getFileDataBytes(JNIEnv *env, jnivm::java::lang::String* arg0) {
    std::ifstream file(*arg0, std::ios::binary | std::ios::ate);
    if(file.is_open()) {
        auto length = file.tellg();
        auto value = new jbyte[(size_t)length + 1];
        file.seekg(0, std::ios::beg);
        file.read((char*)value, length);
        value[length] = 0;
        return new jnivm::Array<jbyte>(value, (size_t)length);
    } else {
        if(!arg0->compare(0, 20, "file:/android_asset/")) {
            return getFileDataBytes(env, (jnivm::java::lang::String*)env->NewStringUTF(arg0->data() + 20));
        }
        return nullptr;
    }
}

void com::mojang::minecraftpe::MainActivity::displayDialog(JNIEnv *env, jint arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::tick(JNIEnv *env) {

}

void com::mojang::minecraftpe::MainActivity::quit(JNIEnv *env) {
    
}

void com::mojang::minecraftpe::MainActivity::initiateUserInput(JNIEnv *env, jint arg0) {
    
}

jint com::mojang::minecraftpe::MainActivity::getUserInputStatus(JNIEnv *env) {
    return 0;
}

jnivm::Array<jnivm::java::lang::String*>* com::mojang::minecraftpe::MainActivity::getUserInputString(JNIEnv *env) {
    return 0;
}

jint com::mojang::minecraftpe::MainActivity::checkLicense(JNIEnv *env) {
    return 1;
}

jboolean com::mojang::minecraftpe::MainActivity::hasBuyButtonWhenInvalidLicense(JNIEnv *env) {
    return false;
}

void com::mojang::minecraftpe::MainActivity::buyGame(JNIEnv *env) {
    
}

void com::mojang::minecraftpe::MainActivity::vibrate(JNIEnv *env, jint arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::setIsPowerVR(JNIEnv *env, jboolean arg0) {
    
}

jboolean com::mojang::minecraftpe::MainActivity::isNetworkEnabled(JNIEnv *env, jboolean arg0) {
    return true;
}

jfloat com::mojang::minecraftpe::MainActivity::getPixelsPerMillimeter(JNIEnv *env) {
    return 80;
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getPlatformStringVar(JNIEnv *env, jint arg0) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::java::lang::Object* com::mojang::minecraftpe::MainActivity::getSystemService(JNIEnv *env, jnivm::java::lang::String* arg0) {
    auto sc = new jnivm::java::lang::Object();
    sc->clazz = (jnivm::java::lang::Class*)env->FindClass("android/lang/Object");
    return sc;
}

jnivm::android::view::Window* com::mojang::minecraftpe::MainActivity::getWindow(JNIEnv *env) {
    auto w = new jnivm::android::view::Window();
    w->clazz = (jnivm::java::lang::Class*)env->FindClass("android/view/Window");
    return w;
}

jint com::mojang::minecraftpe::MainActivity::getKeyFromKeyCode(JNIEnv *env, jint arg0, jint arg1, jint arg2) {
    return arg0;
}

void com::mojang::minecraftpe::MainActivity::updateLocalization(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1) {
    
}

void com::mojang::minecraftpe::MainActivity::showKeyboard(JNIEnv *env, jnivm::java::lang::String* text, jint arg1, jboolean arg2, jboolean arg3, jboolean multiline) {
    currentTextMutliline = multiline;
    maxcurrentTextLength = arg1;
    updateTextboxText(env, text);
}

void com::mojang::minecraftpe::MainActivity::hideKeyboard(JNIEnv *env) {
    --iskeyboardvisible;
    Log::debug("Keyboard", "hide %d", iskeyboardvisible);
    if(!iskeyboardvisible) {
        window->setKeyboardState(iskeyboardvisible);
    }
}

jfloat com::mojang::minecraftpe::MainActivity::getKeyboardHeight(JNIEnv *env) {
    return 0;
}

void com::mojang::minecraftpe::MainActivity::updateTextboxText(JNIEnv *env, jnivm::java::lang::String* arg0) {
    if(!iskeyboardvisible) {
        window->setKeyboardState(1);
    }
    ++iskeyboardvisible;
    Log::debug("Keyboard", "show %d", iskeyboardvisible);
    currentText = *arg0;
    currentTextPosition = currentText.size();
    currentTextPositionUTF = UTF8Util::getLength(currentText.c_str(), currentTextPosition);
    currentTextCopyPosition = currentTextPosition;
}

jint com::mojang::minecraftpe::MainActivity::getCursorPosition(JNIEnv *env) {
    return currentTextPositionUTF;
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getAccessToken(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getClientId(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getProfileId(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getProfileName(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

jnivm::Array<jnivm::java::lang::String*>* com::mojang::minecraftpe::MainActivity::getBroadcastAddresses(JNIEnv *env) {
    return nullptr;
}

jnivm::Array<jnivm::java::lang::String*>* com::mojang::minecraftpe::MainActivity::getIPAddresses(JNIEnv *env) {
    return nullptr;
}

jlong com::mojang::minecraftpe::MainActivity::getTotalMemory(JNIEnv *env) {
    return std::numeric_limits<uint32_t>::max();
}

jlong com::mojang::minecraftpe::MainActivity::getMemoryLimit(JNIEnv *env) {
    return std::numeric_limits<uint32_t>::max();
}

jlong com::mojang::minecraftpe::MainActivity::getUsedMemory(JNIEnv *env) {
    return 0;
}

jlong com::mojang::minecraftpe::MainActivity::getFreeMemory(JNIEnv *env) {
    return std::numeric_limits<uint32_t>::max();      
}

void com::mojang::minecraftpe::MainActivity::launchUri(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::setClipboard(JNIEnv *env, jnivm::java::lang::String* arg0) {
    window->setClipboardText(*arg0);
}

void com::mojang::minecraftpe::MainActivity::share(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1, jnivm::java::lang::String* arg2) {
    
}

jnivm::android::content::Intent* com::mojang::minecraftpe::MainActivity::createAndroidLaunchIntent(JNIEnv *env) {
    return 0;
}

jlong com::mojang::minecraftpe::MainActivity::calculateAvailableDiskFreeSpace(JNIEnv *env, jnivm::java::lang::String* arg0) {
    return -1;
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getExternalStoragePath(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF(PathHelper::getPrimaryDataDirectory().data());
}

void com::mojang::minecraftpe::MainActivity::requestStoragePermission(JNIEnv *env, jint arg0) {
    
}

jboolean com::mojang::minecraftpe::MainActivity::hasWriteExternalStoragePermission(JNIEnv *env) {
    return true;
}

void com::mojang::minecraftpe::MainActivity::deviceIdCorrelationStart(JNIEnv *env) {
    
}

jboolean com::mojang::minecraftpe::MainActivity::isMixerCreateInstalled(JNIEnv *env) {
    return false;
}

void com::mojang::minecraftpe::MainActivity::navigateToPlaystoreForMixerCreate(JNIEnv *env) {
    
}

jboolean com::mojang::minecraftpe::MainActivity::launchMixerCreateForBroadcast(JNIEnv *env) {
    return false;
}

jboolean com::mojang::minecraftpe::MainActivity::isTTSEnabled(JNIEnv *env) {
    return true;
}

jnivm::com::mojang::minecraftpe::HardwareInformation* com::mojang::minecraftpe::MainActivity::getHardwareInfo(JNIEnv *env) {
    auto hw = new jnivm::com::mojang::minecraftpe::HardwareInformation();
    hw->clazz = (jnivm::java::lang::Class*)env->FindClass("com/mojang/minecraftpe/HardwareInformation");
    return hw;
}

void com::mojang::minecraftpe::MainActivity::setCachedDeviceId(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::setLastDeviceSessionId(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getLastDeviceSessionId(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("");
}

jint com::mojang::minecraftpe::MainActivity::getAPIVersion(JNIEnv *env, jnivm::java::lang::String* arg0) {
    return 27;
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getSecureStorageKey(JNIEnv *env, jnivm::java::lang::String* arg0) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Unknown");
}

void com::mojang::minecraftpe::MainActivity::setSecureStorageKey(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1) {
    
}

void com::mojang::minecraftpe::MainActivity::trackPurchaseEvent(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1, jnivm::java::lang::String* arg2, jnivm::java::lang::String* arg3, jnivm::java::lang::String* arg4, jnivm::java::lang::String* arg5, jnivm::java::lang::String* arg6, jnivm::java::lang::String* arg7) {
    
}

void com::mojang::minecraftpe::MainActivity::sendBrazeEvent(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::sendBrazeEventWithProperty(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1, jint arg2) {
    
}

void com::mojang::minecraftpe::MainActivity::sendBrazeEventWithStringProperty(JNIEnv *env, jnivm::java::lang::String* arg0, jnivm::java::lang::String* arg1, jnivm::java::lang::String* arg2) {
    
}

void com::mojang::minecraftpe::MainActivity::sendBrazeToastClick(JNIEnv *env) {
    
}

void com::mojang::minecraftpe::MainActivity::sendBrazeDialogButtonClick(JNIEnv *env, jint arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::pickImage(JNIEnv *env, jlong arg0) {
    Log::trace("MainActivity", "pickImage");
    auto picker = FilePickerFactory::createFilePicker();
    picker->setTitle("Select image");
    picker->setFileNameFilters({ "*.png" });
    if (picker->show()) {
        nativeOnPickImageSuccess(env, nullptr, arg0, env->NewStringUTF(picker->getPickedFile().data()));
    } else {
        nativeOnPickImageCanceled(env, nullptr, arg0);
    }
}

void com::mojang::minecraftpe::MainActivity::setFileDialogCallback(JNIEnv *env, jlong arg0) {
    
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getLegacyDeviceID(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("19af3ae9-b15a-44b0-a3c2-aa2c66df489e");
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::createUUID(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("daa78df1-373a-444d-9b1d-4c71a14bb559");
}

jboolean com::mojang::minecraftpe::MainActivity::hasHardwareKeyboard(JNIEnv *env) {
    return false;
}

void com::mojang::minecraftpe::MainActivity::startTextToSpeech(JNIEnv *env, jnivm::java::lang::String* arg0) {
    
}

void com::mojang::minecraftpe::MainActivity::stopTextToSpeech(JNIEnv *env) {
    
}

jboolean com::mojang::minecraftpe::MainActivity::isTextToSpeechInProgress(JNIEnv *env) {
    // return false;
    return false;
}

void com::mojang::minecraftpe::MainActivity::setTextToSpeechEnabled(JNIEnv *env, jboolean arg0) {
    
}

jint com::mojang::minecraftpe::MainActivity::getScreenWidth(JNIEnv *env) {
    return 0;
}

jint com::mojang::minecraftpe::MainActivity::getScreenHeight(JNIEnv *env) {
    return 0;    
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getDeviceModel(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("Android");
}

// Needed for showing gamepad settings
jint com::mojang::minecraftpe::MainActivity::getAndroidVersion(JNIEnv *env) {
    return 28;
}

jnivm::java::lang::String* com::mojang::minecraftpe::MainActivity::getLocale(JNIEnv *env) {
    return (jnivm::java::lang::String*)env->NewStringUTF("en");
}

jboolean com::mojang::minecraftpe::MainActivity::isFirstSnooperStart(JNIEnv *env) {
    return false;
}

jboolean com::mojang::minecraftpe::MainActivity::hasHardwareChanged(JNIEnv *env) {
    return false;
}

jboolean com::mojang::minecraftpe::MainActivity::isTablet(JNIEnv *env) {
    return false;
}

jnivm::java::lang::ClassLoader* com::mojang::minecraftpe::MainActivity::getClassLoader(JNIEnv *env) {
    auto hw = new jnivm::java::lang::ClassLoader();
    hw->clazz = (jnivm::java::lang::Class*)env->FindClass("java/lang/ClassLoader");
    return hw;
}

// Entry points for jnivm

extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_saveScreenshot(JNIEnv *env, jclass clazz, jvalue* values) {
    return com::mojang::minecraftpe::MainActivity::saveScreenshot(env, clazz, (jnivm::java::lang::String*&)values[0], (jint&)values[1], (jint&)values[2], (jnivm::Array<jint>*&)values[3]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_postScreenshotToFacebook(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->postScreenshotToFacebook(env, (jnivm::java::lang::String*&)values[0], (jint&)values[1], (jint&)values[2], (jnivm::Array<jint>*&)values[3]);
}
extern "C" jnivm::Array<jint>* jnivm_com_mojang_minecraftpe_MainActivity_getImageData(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getImageData(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::Array<jbyte>* jnivm_com_mojang_minecraftpe_MainActivity_getFileDataBytes(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getFileDataBytes(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_displayDialog(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->displayDialog(env, (jint&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_tick(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->tick(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_quit(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->quit(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_initiateUserInput(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->initiateUserInput(env, (jint&)values[0]);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getUserInputStatus(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getUserInputStatus(env);
}
extern "C" jnivm::Array<jnivm::java::lang::String*>* jnivm_com_mojang_minecraftpe_MainActivity_getUserInputString(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getUserInputString(env);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_checkLicense(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->checkLicense(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_hasBuyButtonWhenInvalidLicense(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->hasBuyButtonWhenInvalidLicense(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_buyGame(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->buyGame(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_vibrate(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->vibrate(env, (jint&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setIsPowerVR(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setIsPowerVR(env, (jboolean&)values[0]);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_isNetworkEnabled(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->isNetworkEnabled(env, (jboolean&)values[0]);
}
extern "C" jfloat jnivm_com_mojang_minecraftpe_MainActivity_getPixelsPerMillimeter(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getPixelsPerMillimeter(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getPlatformStringVar(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getPlatformStringVar(env, (jint&)values[0]);
}
extern "C" jnivm::java::lang::Object* jnivm_com_mojang_minecraftpe_MainActivity_getSystemService(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getSystemService(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::android::view::Window* jnivm_com_mojang_minecraftpe_MainActivity_getWindow(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getWindow(env);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getKeyFromKeyCode(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getKeyFromKeyCode(env, (jint&)values[0], (jint&)values[1], (jint&)values[2]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_updateLocalization(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->updateLocalization(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_showKeyboard(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->showKeyboard(env, (jnivm::java::lang::String*&)values[0], (jint&)values[1], (jboolean&)values[2], (jboolean&)values[3], (jboolean&)values[4]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_hideKeyboard(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->hideKeyboard(env);
}
extern "C" jfloat jnivm_com_mojang_minecraftpe_MainActivity_getKeyboardHeight(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getKeyboardHeight(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_updateTextboxText(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->updateTextboxText(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getCursorPosition(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getCursorPosition(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getAccessToken(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getAccessToken(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getClientId(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getClientId(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getProfileId(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getProfileId(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getProfileName(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getProfileName(env);
}
extern "C" jnivm::Array<jnivm::java::lang::String*>* jnivm_com_mojang_minecraftpe_MainActivity_getBroadcastAddresses(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getBroadcastAddresses(env);
}
extern "C" jnivm::Array<jnivm::java::lang::String*>* jnivm_com_mojang_minecraftpe_MainActivity_getIPAddresses(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getIPAddresses(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_MainActivity_getTotalMemory(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getTotalMemory(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_MainActivity_getMemoryLimit(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getMemoryLimit(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_MainActivity_getUsedMemory(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getUsedMemory(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_MainActivity_getFreeMemory(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getFreeMemory(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_launchUri(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->launchUri(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setClipboard(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setClipboard(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_share(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->share(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1], (jnivm::java::lang::String*&)values[2]);
}
extern "C" jnivm::android::content::Intent* jnivm_com_mojang_minecraftpe_MainActivity_createAndroidLaunchIntent(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->createAndroidLaunchIntent(env);
}
extern "C" jlong jnivm_com_mojang_minecraftpe_MainActivity_calculateAvailableDiskFreeSpace(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->calculateAvailableDiskFreeSpace(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getExternalStoragePath(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getExternalStoragePath(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_requestStoragePermission(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->requestStoragePermission(env, (jint&)values[0]);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_hasWriteExternalStoragePermission(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->hasWriteExternalStoragePermission(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_deviceIdCorrelationStart(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->deviceIdCorrelationStart(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_isMixerCreateInstalled(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->isMixerCreateInstalled(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_navigateToPlaystoreForMixerCreate(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->navigateToPlaystoreForMixerCreate(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_launchMixerCreateForBroadcast(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->launchMixerCreateForBroadcast(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_isTTSEnabled(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->isTTSEnabled(env);
}
extern "C" jnivm::com::mojang::minecraftpe::HardwareInformation* jnivm_com_mojang_minecraftpe_MainActivity_getHardwareInfo(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getHardwareInfo(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setCachedDeviceId(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setCachedDeviceId(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setLastDeviceSessionId(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setLastDeviceSessionId(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getLastDeviceSessionId(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getLastDeviceSessionId(env);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getAPIVersion(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getAPIVersion(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getSecureStorageKey(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getSecureStorageKey(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setSecureStorageKey(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setSecureStorageKey(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_trackPurchaseEvent(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->trackPurchaseEvent(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1], (jnivm::java::lang::String*&)values[2], (jnivm::java::lang::String*&)values[3], (jnivm::java::lang::String*&)values[4], (jnivm::java::lang::String*&)values[5], (jnivm::java::lang::String*&)values[6], (jnivm::java::lang::String*&)values[7]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_sendBrazeEvent(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->sendBrazeEvent(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_sendBrazeEventWithProperty(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->sendBrazeEventWithProperty(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1], (jint&)values[2]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_sendBrazeEventWithStringProperty(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->sendBrazeEventWithStringProperty(env, (jnivm::java::lang::String*&)values[0], (jnivm::java::lang::String*&)values[1], (jnivm::java::lang::String*&)values[2]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_sendBrazeToastClick(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->sendBrazeToastClick(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_sendBrazeDialogButtonClick(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->sendBrazeDialogButtonClick(env, (jint&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_pickImage(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->pickImage(env, (jlong&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setFileDialogCallback(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setFileDialogCallback(env, (jlong&)values[0]);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getLegacyDeviceID(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getLegacyDeviceID(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_createUUID(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->createUUID(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_hasHardwareKeyboard(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->hasHardwareKeyboard(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_startTextToSpeech(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->startTextToSpeech(env, (jnivm::java::lang::String*&)values[0]);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_stopTextToSpeech(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->stopTextToSpeech(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_isTextToSpeechInProgress(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->isTextToSpeechInProgress(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_setTextToSpeechEnabled(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->setTextToSpeechEnabled(env, (jboolean&)values[0]);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getScreenWidth(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getScreenWidth(env);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getScreenHeight(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getScreenHeight(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getDeviceModel(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getDeviceModel(env);
}
extern "C" jint jnivm_com_mojang_minecraftpe_MainActivity_getAndroidVersion(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getAndroidVersion(env);
}
extern "C" jnivm::java::lang::String* jnivm_com_mojang_minecraftpe_MainActivity_getLocale(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getLocale(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_isFirstSnooperStart(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->isFirstSnooperStart(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_hasHardwareChanged(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->hasHardwareChanged(env);
}
extern "C" jboolean jnivm_com_mojang_minecraftpe_MainActivity_isTablet(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->isTablet(env);
}
extern "C" jnivm::java::lang::ClassLoader* jnivm_com_mojang_minecraftpe_MainActivity_getClassLoader(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->getClassLoader(env);
}
extern "C" void jnivm_com_mojang_minecraftpe_MainActivity_webRequest(JNIEnv *env, jnivm::com::mojang::minecraftpe::MainActivity* obj, jvalue* values) {
    return obj->webRequest(env, (jint&)values[0], (jlong&)values[1], (jnivm::java::lang::String*&)values[2], (jnivm::java::lang::String*&)values[3], (jnivm::java::lang::String*&)values[4], (jnivm::java::lang::String*&)values[5]);
}