#include "window_eglut.h"
#include "joystick_manager_linux_gamepad.h"

#include <cstring>
#include <sstream>
#include <eglut.h>
#include <eglut_x11.h>

EGLUTWindow* EGLUTWindow::currentWindow;

EGLUTWindow::EGLUTWindow(const std::string& title, int width, int height, GraphicsApi api) :
        WindowWithLinuxJoystick(title, width, height, api), title(title), width(width), height(height),
        graphicsApi(api) {
    eglutInitWindowSize(width, height);
    if (graphicsApi == GraphicsApi::OPENGL_ES2)
        eglutInitAPIMask(EGLUT_OPENGL_ES2_BIT);
    winId = eglutCreateWindow(title.c_str());

    eglutIdleFunc(_eglutIdleFunc);
    eglutDisplayFunc(_eglutDisplayFunc);
    eglutReshapeFunc(_eglutReshapeFunc);
    eglutMouseFunc(_eglutMouseFunc);
    eglutMouseButtonFunc(_eglutMouseButtonFunc);
    eglutMouseRawFunc(_eglutMouseRawFunc);
    eglutTouchStartFunc(_eglutTouchStartFunc);
    eglutTouchUpdateFunc(_eglutTouchUpdateFunc);
    eglutTouchEndFunc(_eglutTouchEndFunc);
    eglutKeyboardFunc(_eglutKeyboardFunc);
    eglutSpecialFunc(_eglutKeyboardSpecialFunc);
    eglutPasteFunc(_eglutPasteFunc);
    eglutFocusFunc(_eglutFocusFunc);
    eglutCloseWindowFunc(_eglutCloseWindowFunc);

    memset(pointerIds, 0xff, sizeof(pointerIds));
}

EGLUTWindow::~EGLUTWindow() {
    if (currentWindow == this)
        currentWindow = nullptr;
    if (winId != -1)
        eglutDestroyWindow(winId);
}

void EGLUTWindow::setIcon(std::string const &iconPath) {
    eglutSetWindowIcon(iconPath.c_str());
}

void EGLUTWindow::show() {
    eglutShowWindow();
    currentWindow = this;
}

void EGLUTWindow::close() {
    eglutDestroyWindow(currentWindow->winId);
    eglutFini();
    currentWindow->winId = -1;
}

void EGLUTWindow::prepareRunLoop() {
    addWindowToGamepadManager();
}

void EGLUTWindow::runLoop() {
    addWindowToGamepadManager();
    eglutMainLoop();
}

void EGLUTWindow::setCursorDisabled(bool disabled) {
    cursorDisabled = disabled;
    eglutSetMousePointerLocked(disabled ? EGLUT_POINTER_LOCKED : EGLUT_POINTER_UNLOCKED);
}

void EGLUTWindow::setFullscreen(bool fullscreen) {
    if (eglutGet(EGLUT_FULLSCREEN_MODE) != (fullscreen ? EGLUT_FULLSCREEN : EGLUT_WINDOWED))
        eglutToggleFullscreen();
}

void EGLUTWindow::swapBuffers() {
    eglutSwapBuffers();
}

void EGLUTWindow::pollEvents() {
    eglutPollEvents();
}

void EGLUTWindow::swapInterval(int interval) {
    eglutSwapInterval(interval);
}

void EGLUTWindow::_eglutIdleFunc() {
    if (currentWindow == nullptr)
        return;
    currentWindow->updateGamepad();
    eglutPostRedisplay();
}

void EGLUTWindow::_eglutDisplayFunc() {
    if (currentWindow == nullptr)
        return;
    currentWindow->onDraw();
}

void EGLUTWindow::_eglutReshapeFunc(int w, int h) {
    if (currentWindow == nullptr || (currentWindow->width == w && currentWindow->height == h))
        return;
    currentWindow->onWindowSizeChanged(w, h);
    currentWindow->width = w;
    currentWindow->height = h;
}

void EGLUTWindow::_eglutMouseFunc(int x, int y) {
    if (currentWindow == nullptr)
        return;
    currentWindow->onMousePosition(x, y);
}

void EGLUTWindow::_eglutMouseRawFunc(double x, double y) {
    if (currentWindow == nullptr)
        return;
    currentWindow->onMouseRelativePosition(x, y);
}

void EGLUTWindow::_eglutMouseButtonFunc(int x, int y, int btn, int action) {
    if (currentWindow == nullptr)
        return;
    if (btn == 4 || btn == 5) {
        currentWindow->onMouseScroll(x, y, 0.0, (btn == 5 ? -1.0 : 1.0));
        return;
    }
    btn = (btn == 2 ? 3 : (btn == 3 ? 2 : btn));
    currentWindow->onMouseButton(x, y, btn, action == EGLUT_MOUSE_PRESS ? MouseButtonAction::PRESS :
                                            MouseButtonAction::RELEASE);
}

int EGLUTWindow::obtainTouchPointer(int eglutId) {
    for (int i = 0; i < sizeof(pointerIds) / sizeof(int); i++) {
        if (pointerIds[i] == eglutId)
            return i;
    }
    for (int i = 0; i < sizeof(pointerIds) / sizeof(int); i++) {
        if (pointerIds[i] == -1) {
            pointerIds[i] = eglutId;
            return i;
        }
    }
    return sizeof(pointerIds) / sizeof(int) + eglutId;
}

void EGLUTWindow::releaseTouchPointer(int ourId) {
    pointerIds[ourId] = -1;
}

void EGLUTWindow::_eglutTouchStartFunc(int id, double x, double y) {
    if (currentWindow == nullptr)
        return;
    int ourId = currentWindow->obtainTouchPointer(id);
    currentWindow->onTouchStart(ourId, x, y);
}

void EGLUTWindow::_eglutTouchUpdateFunc(int id, double x, double y) {
    if (currentWindow == nullptr)
        return;
    int ourId = currentWindow->obtainTouchPointer(id);
    currentWindow->onTouchUpdate(ourId, x, y);
}

void EGLUTWindow::_eglutTouchEndFunc(int id, double x, double y) {
    if (currentWindow == nullptr)
        return;
    int ourId = currentWindow->obtainTouchPointer(id);
    currentWindow->onTouchEnd(ourId, x, y);
    currentWindow->releaseTouchPointer(ourId);
}

void EGLUTWindow::_eglutKeyboardFunc(char* str, int action) {
    if (currentWindow == nullptr ||
        strcmp(str, "\t") == 0 || strcmp(str, "\03") == 0 || strcmp(str, "\26") == 0 ||
        strcmp(str, "\33") == 0) // \t, copy, paste, esc
        return;
    if (action == EGLUT_KEY_PRESS || action == EGLUT_KEY_REPEAT) {
        if (str[0] == 13 && str[1] == 0)
            str[0] = 10;
        std::stringstream ss;
        ss << str;
        currentWindow->onKeyboardText(ss.str());
    }
}

int EGLUTWindow::getKeyMinecraft(int keyCode) {
    // TODO: insert (45), numpad (96-111), numlock (133), scroll (145)
    if (keyCode == 65505 || keyCode == 65506) // left/right shift
        return 16;
    if (keyCode == 65507 || keyCode == 65508) // left/right control
        return 17;
    if (keyCode == 65509) // caps lock
        return 20;
    if (keyCode == 65307) // esc
        return 27;
    if (keyCode == 65293) // enter
        return 13;
    if (keyCode >= 65365 && keyCode <= 65367) // pg up, pg down, end
        return keyCode - 65365 + 33;
    if (keyCode == 65360) // home
        return 36;
    if (keyCode == 65288) // backspace
        return 8;
    if (keyCode == 65289) // tab
        return 9;
    if (keyCode == 65535) // delete
        return 46;
    if (keyCode == 59) // ;
        return 186;
    if (keyCode == 61) // =
        return 187;
    if (keyCode >= 44 && keyCode <= 47) // ,-./
        return keyCode - 44 + 188;
    if (keyCode == 96) // `
        return 192;
    if (keyCode >= 91 && keyCode <= 93) // [\]
        return keyCode - 91 + 219;
    if (keyCode == 39) // '
        return 222;
    if (keyCode == 65515) // tab
        return 8;
    if (keyCode >= 97 && keyCode <= 122)
        return (keyCode + 65 - 97);
    if (keyCode >= 65361 && keyCode <= 65364)
        return (keyCode + 37 - 65361);
    if (keyCode >= 65470 && keyCode <= 65481)
        return (keyCode + 112 - 65470);
    if (keyCode >= 65456 && keyCode <= 65465) // numpad
        return (keyCode + 96 - 65456);

    return keyCode;
}

void EGLUTWindow::_eglutKeyboardSpecialFunc(int key, int action) {
    if (currentWindow == nullptr)
        return;
    if (key == 65507)
        currentWindow->modCTRL = (action != EGLUT_KEY_RELEASE);
    if (currentWindow->modCTRL && (key == 86 || key == 118) && action == EGLUT_KEY_PRESS) {
        eglutRequestPaste();
    }
    int mKey = getKeyMinecraft(key);
    KeyAction enumAction = (action == EGLUT_KEY_PRESS ? KeyAction::PRESS :
                            (action == EGLUT_KEY_REPEAT ? KeyAction::REPEAT : KeyAction::RELEASE));
    currentWindow->onKeyboard(mKey, enumAction);
}

void EGLUTWindow::_eglutPasteFunc(const char* str, int len) {
    if (currentWindow == nullptr)
        return;
    currentWindow->onPaste(std::string(str, len));
}

void EGLUTWindow::_eglutFocusFunc(int action) {
    if (currentWindow == nullptr)
        return;
    LinuxGamepadJoystickManager::instance.onWindowFocused(currentWindow, (action == EGLUT_FOCUSED));
    currentWindow->onFocusChange(action == EGLUT_FOCUSED);
}

void EGLUTWindow::_eglutCloseWindowFunc() {
    if (currentWindow == nullptr)
        return;
    currentWindow->onClose();
}

void EGLUTWindow::getWindowSize(int& width, int& height) const {
    width = this->width;
    height = this->height;
}

void EGLUTWindow::setClipboardText(std::string const &text) {
    eglutSetClipboardText(text.c_str());
}

void EGLUTWindow::setKeyboardState(int state) {
    eglutSetKeyboardState(state);
}