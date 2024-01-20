#pragma once

#include <string>
#include <functional>

enum class GraphicsApi {
    OPENGL, OPENGL_ES2
};
enum class KeyAction {
    PRESS, REPEAT, RELEASE
};
enum class MouseButtonAction {
    PRESS, RELEASE
};
enum class GamepadButtonId {
    A, B, X, Y, LB, RB, BACK, START, GUIDE, LEFT_STICK, RIGHT_STICK,
    DPAD_UP, DPAD_RIGHT, DPAD_DOWN, DPAD_LEFT,
    UNKNOWN = -1
};
enum class GamepadAxisId {
    LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y, LEFT_TRIGGER, RIGHT_TRIGGER,
    UNKNOWN = -1
};

class GameWindow {

public:
    using DrawCallback = std::function<void ()>;
    using WindowSizeCallback = std::function<void (int, int)>;
    using MouseButtonCallback = std::function<void (double, double, int, MouseButtonAction)>;
    using MousePositionCallback = std::function<void (double, double)>;
    using MouseScrollCallback = std::function<void (double, double, double, double)>;
    using TouchStartCallback = std::function<void (int, double, double)>;
    using TouchUpdateCallback = std::function<void (int, double, double)>;
    using TouchEndCallback = std::function<void (int, double, double)>;
    using KeyboardCallback = std::function<void (int, KeyAction)>;
    using KeyboardTextCallback = std::function<void (std::string const&)>;
    using PasteCallback = std::function<void (std::string const&)>;
    using GamepadStateCallback = std::function<void (int, bool)>;
    using GamepadButtonCallback = std::function<void (int, GamepadButtonId, bool)>;
    using GamepadAxisCallback = std::function<void (int, GamepadAxisId, float)>;
    using CloseCallback = std::function<void ()>;
    using FocusChangeCallback = std::function<void (bool)>;

private:
    DrawCallback drawCallback;
    WindowSizeCallback windowSizeCallback;
    MouseButtonCallback mouseButtonCallback;
    MousePositionCallback mousePositionCallback, mouseRelativePositionCallback;
    MouseScrollCallback mouseScrollCallback;
    TouchStartCallback touchStartCallback;
    TouchUpdateCallback touchUpdateCallback;
    TouchEndCallback touchEndCallback;
    KeyboardCallback keyboardCallback;
    KeyboardTextCallback keyboardTextCallback;
    PasteCallback pasteCallback;
    GamepadStateCallback gamepadStateCallback;
    GamepadButtonCallback gamepadButtonCallback;
    GamepadAxisCallback gamepadAxisCallback;
    CloseCallback closeCallback;
    FocusChangeCallback focusChangeCallback;

public:

    GameWindow(std::string const& title, int width, int height, GraphicsApi api) {}

    virtual ~GameWindow() {}

    virtual void setIcon(std::string const& iconPath) = 0;

    virtual void show() = 0;

    virtual void close() = 0;

    virtual void prepareRunLoop() = 0;

    virtual void runLoop() = 0;

    virtual void setCursorDisabled(bool disabled) = 0;

    virtual void setFullscreen(bool fullscreen) = 0;

    virtual void getWindowSize(int& width, int& height) const = 0;

    virtual void setClipboardText(std::string const& text) = 0;

    virtual void swapBuffers() = 0;

    virtual void pollEvents() = 0;

    virtual void swapInterval(int interval) = 0;

    virtual void setKeyboardState(int state) {}

    void setDrawCallback(DrawCallback callback) { drawCallback = std::move(callback); }

    void setWindowSizeCallback(WindowSizeCallback callback) { windowSizeCallback = std::move(callback); }

    void setMouseButtonCallback(MouseButtonCallback callback) { mouseButtonCallback = std::move(callback); }

    void setMousePositionCallback(MousePositionCallback callback) { mousePositionCallback = std::move(callback); }

    void setMouseScrollCallback(MouseScrollCallback callback) { mouseScrollCallback = std::move(callback); }

    void setTouchStartCallback(TouchStartCallback callback) { touchStartCallback = std::move(callback); }

    void setTouchUpdateCallback(TouchUpdateCallback callback) { touchUpdateCallback = std::move(callback); }

    void setTouchEndCallback(TouchEndCallback callback) { touchEndCallback = std::move(callback); }

    void setKeyboardCallback(KeyboardCallback callback) { keyboardCallback = std::move(callback); }

    void setKeyboardTextCallback(KeyboardTextCallback callback) { keyboardTextCallback = std::move(callback); }

    void setPasteCallback(PasteCallback callback) { pasteCallback = std::move(callback); }

    // Used when the cursor is disabled
    void setMouseRelativePositionCallback(MousePositionCallback callback) { mouseRelativePositionCallback = std::move(callback); }

    void setGamepadStateCallback(GamepadStateCallback callback) { gamepadStateCallback = std::move(callback); }

    void setGamepadButtonCallback(GamepadButtonCallback callback) { gamepadButtonCallback = std::move(callback); }

    void setGamepadAxisCallback(GamepadAxisCallback callback) { gamepadAxisCallback = std::move(callback); }

    void setCloseCallback(CloseCallback callback) { closeCallback = std::move(callback); }

    void setFocusChangeCallback(FocusChangeCallback callback) { focusChangeCallback = std::move(callback); }


protected:

    void onDraw() {
        if (drawCallback != nullptr)
            drawCallback();
    }
    void onWindowSizeChanged(int w, int h) {
        if (windowSizeCallback != nullptr)
            windowSizeCallback(w, h);
    }
    void onMouseButton(double x, double y, int button, MouseButtonAction action) {
        if (mouseButtonCallback != nullptr)
            mouseButtonCallback(x, y, button, action);
    }
    void onMousePosition(double x, double y) {
        if (mousePositionCallback != nullptr)
            mousePositionCallback(x, y);
    }
    void onMouseRelativePosition(double x, double y) {
        if (mouseRelativePositionCallback != nullptr)
            mouseRelativePositionCallback(x, y);
    }
    void onMouseScroll(double x, double y, double dx, double dy) {
        if (mouseScrollCallback != nullptr)
            mouseScrollCallback(x, y, dx, dy);
    }
    void onTouchStart(int id, double x, double y) {
        if (touchStartCallback != nullptr)
            touchStartCallback(id, x, y);
    }
    void onTouchUpdate(int id, double x, double y) {
        if (touchUpdateCallback != nullptr)
            touchUpdateCallback(id, x, y);
    }
    void onTouchEnd(int id, double x, double y) {
        if (touchEndCallback != nullptr)
            touchEndCallback(id, x, y);
    }
    void onKeyboard(int key, KeyAction action) {
        if (keyboardCallback != nullptr)
            keyboardCallback(key, action);
    }
    void onKeyboardText(std::string const& c) {
        if (keyboardTextCallback != nullptr)
            keyboardTextCallback(c);
    }
    void onPaste(std::string const& c) {
        if (pasteCallback != nullptr)
            pasteCallback(c);
    }
    void onGamepadState(int id, bool connected) {
        if (gamepadStateCallback != nullptr)
            gamepadStateCallback(id, connected);
    }
    void onGamepadButton(int id, GamepadButtonId btn, bool pressed) {
        if (gamepadButtonCallback != nullptr)
            gamepadButtonCallback(id, btn, pressed);
    }
    void onGamepadAxis(int id, GamepadAxisId axis, float val) {
        if (gamepadAxisCallback != nullptr)
            gamepadAxisCallback(id, axis, val);
    }
    void onClose() {
        if (closeCallback != nullptr)
            closeCallback();
    }

    void onFocusChange(bool hasfocus) {
        if (focusChangeCallback != nullptr)
            focusChangeCallback(hasfocus);
    }
};
