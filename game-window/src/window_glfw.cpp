#include "window_glfw.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <math.h>

#include <codecvt>
#include <iostream>
#include <locale>
#include <thread>
#include <utility>

#include "../../logger/include/log.h"
#include "GLFW/glfw3.h"
#include "joystick_manager_glfw.h"

GLFWGameWindow::GLFWGameWindow(const std::string &title, int width, int height,
                               GraphicsApi api)
    : GameWindow(title, width, height, api),
      windowedWidth(width),
      windowedHeight(height)
{
  glfwDefaultWindowHints();
  if (api == GraphicsApi::OPENGL_ES2)
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  }
  else if (api == GraphicsApi::OPENGL)
  {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  }
  window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, _glfwWindowSizeCallback);
  glfwSetCursorPosCallback(window, _glfwCursorPosCallback);
  glfwSetMouseButtonCallback(window, _glfwMouseButtonCallback);
  glfwSetScrollCallback(window, _glfwScrollCallback);
  glfwSetWindowCloseCallback(window, _glfwWindowCloseCallback);
  glfwSetKeyCallback(window, _glfwKeyCallback);
  glfwSetCharCallback(window, _glfwCharCallback);
  glfwSetWindowFocusCallback(window, _glfwWindowFocusCallback);
  glfwSetWindowContentScaleCallback(window, _glfwWindowContentScaleCallback);
  glfwMakeContextCurrent(window);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 100");

  if (glfwRawMouseMotionSupported())
  {
    Log::info("GLFW", "Raw Motion is Supported. Using it!");
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }
  else
  {
    Log::info("GLFW", "Can't use Raw Motion :c");
  }
  setRelativeScale();

  lastTimeTargetFPS = glfwGetTime();
}

GLFWGameWindow::~GLFWGameWindow()
{
  GLFWJoystickManager::removeWindow(this);
  glfwDestroyWindow(window);
}

void GLFWGameWindow::setIcon(std::string const &iconPath)
{
  // TODO:
}

void GLFWGameWindow::setRelativeScale()
{
  float xs, ys;
  glfwGetWindowContentScale(window, &xs, &ys);

  xScale = xs;
  yScale = ys;
}

std::pair<float, float> GLFWGameWindow::getRelativeScale() const
{
  return std::make_pair(xScale, yScale);
}

void GLFWGameWindow::getWindowSize(int &width, int &height) const
{
  glfwGetFramebufferSize(window, &width, &height);
}

void GLFWGameWindow::show()
{
  glfwShowWindow(window);
}

void GLFWGameWindow::close()
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GLFWGameWindow::prepareRunLoop()
{
  GLFWJoystickManager::addWindow(this);
}

void GLFWGameWindow::runLoop()
{
  GLFWJoystickManager::addWindow(this);
  while (!glfwWindowShouldClose(window))
  {
    auto drawStart = std::chrono::system_clock::now();
    GLFWJoystickManager::update(this);
    onDraw();

    if (!focused)
      std::this_thread::sleep_until(drawStart +
                                    std::chrono::milliseconds(1000 / 20));

    glfwPollEvents();
  }
}

void GLFWGameWindow::setCursorDisabled(bool disabled)
{
  glfwSetInputMode(window, GLFW_CURSOR,
                   disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
}

void GLFWGameWindow::setFullscreen(bool fullscreen)
{
  if (fullscreen)
  {
    glfwGetWindowPos(window, &windowedX, &windowedY);
    glfwGetFramebufferSize(window, &windowedWidth, &windowedHeight);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
  }
  else
  {
    glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth,
                         windowedHeight, GLFW_DONT_CARE);
  }
}

void GLFWGameWindow::setClipboardText(std::string const &text)
{
  glfwSetClipboardString(window, text.c_str());
}

void GLFWGameWindow::swapBuffers()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Measure speed
  const double currentTime = glfwGetTime();
  const double delta = currentTime - lastTimePerformance;
  numFrames++;

  this->onGUIFrame();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (delta >= 1.0)
  {
    this->fps = static_cast<double>(numFrames) / delta;
    numFrames = 0;
    lastTimePerformance = currentTime;
  }

  if (limitFpsMode == LimitFPSMode::Limited)
  {
    while (glfwGetTime() < (lastTimeTargetFPS + 1.0 / targetFPS))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  lastTimeTargetFPS += 1.0 / targetFPS;
  glfwSwapBuffers(window);
}

void GLFWGameWindow::pollEvents()
{
  glfwPollEvents();
  GLFWJoystickManager::update(this);
}

void GLFWGameWindow::swapInterval(int interval)
{
  if (LimitFPSMode::VSync == limitFpsMode)
    glfwSwapInterval(interval);
  else
    glfwSwapInterval(0);
}

void GLFWGameWindow::_glfwWindowSizeCallback(GLFWwindow *window, int w, int h)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
  user->onWindowSizeChanged(w, h);
}

void GLFWGameWindow::_glfwCursorPosCallback(GLFWwindow *window, double x,
                                            double y)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);

  if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
  {
#ifdef __linux__
    double dx = x - user->lastMouseX;
    double dy = y - user->lastMouseY;
#else
    double dx = (x - user->lastMouseX) * user->getRelativeScale();
    double dy = (y - user->lastMouseY) * user->getRelativeScale();
#endif

    user->onMouseRelativePosition(dx, dy);
    user->lastMouseX = x;
    user->lastMouseY = y;
  }
  else
  {
    x *= user->xScale;
    y *= user->yScale;

    user->onMousePosition(x, y);
  }
}

void GLFWGameWindow::_glfwMouseButtonCallback(GLFWwindow *window, int button,
                                              int action, int mods)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  x *= user->xScale;
  y *= user->yScale;

  user->onMouseButton(x, y, button + 1,
                      action == GLFW_PRESS ? MouseButtonAction::PRESS
                                           : MouseButtonAction::RELEASE);
}

void GLFWGameWindow::_glfwScrollCallback(GLFWwindow *window, double x, double y)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
  double cx, cy;
  glfwGetCursorPos(window, &cx, &cy);
  user->onMouseScroll(cx, cy, x, y);
}

int GLFWGameWindow::getKeyMinecraft(int keyCode)
{
  if (keyCode >= GLFW_KEY_F1 && keyCode <= GLFW_KEY_F12)
    return keyCode - GLFW_KEY_F1 + 112;
  switch (keyCode)
  {
    case GLFW_KEY_BACKSPACE:
      return 8;
    case GLFW_KEY_TAB:
      return 9;
    case GLFW_KEY_ENTER:
      return 13;
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
      return 16;
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL:
      return 17;
    case GLFW_KEY_PAUSE:
      return 19;
    case GLFW_KEY_CAPS_LOCK:
      return 20;
    case GLFW_KEY_ESCAPE:
      return 27;
    case GLFW_KEY_PAGE_UP:
      return 33;
    case GLFW_KEY_PAGE_DOWN:
      return 34;
    case GLFW_KEY_END:
      return 35;
    case GLFW_KEY_HOME:
      return 36;
    case GLFW_KEY_LEFT:
      return 37;
    case GLFW_KEY_UP:
      return 38;
    case GLFW_KEY_RIGHT:
      return 39;
    case GLFW_KEY_DOWN:
      return 40;
    case GLFW_KEY_INSERT:
      return 45;
    case GLFW_KEY_DELETE:
      return 46;
    case GLFW_KEY_NUM_LOCK:
      return 144;
    case GLFW_KEY_SCROLL_LOCK:
      return 145;
    case GLFW_KEY_SEMICOLON:
      return 186;
    case GLFW_KEY_EQUAL:
      return 187;
    case GLFW_KEY_COMMA:
      return 188;
    case GLFW_KEY_MINUS:
      return 189;
    case GLFW_KEY_PERIOD:
      return 190;
    case GLFW_KEY_SLASH:
      return 191;
    case GLFW_KEY_GRAVE_ACCENT:
      return 192;
    case GLFW_KEY_LEFT_BRACKET:
      return 219;
    case GLFW_KEY_BACKSLASH:
      return 220;
    case GLFW_KEY_RIGHT_BRACKET:
      return 221;
    case GLFW_KEY_APOSTROPHE:
      return 222;

      // Extra key mappings that are not necessarily correct but map to
      // completely wrong keys otherwise
    case GLFW_KEY_LEFT_SUPER:
    case GLFW_KEY_RIGHT_SUPER:
      return 1;
    case GLFW_KEY_LEFT_ALT:
    case GLFW_KEY_RIGHT_ALT:
      return 2;
  }
  return keyCode;
}

void GLFWGameWindow::_glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                                      int action, int mods)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
#ifdef __APPLE__
  if (action == GLFW_PRESS && mods & GLFW_MOD_SUPER && key == GLFW_KEY_V)
  {
#else
  if (action == GLFW_PRESS && mods & GLFW_MOD_CONTROL && key == GLFW_KEY_V)
  {
#endif
    user->onPaste(glfwGetClipboardString(window));
  }
  if (action == GLFW_PRESS || action == GLFW_REPEAT)
  {
    if (key == GLFW_KEY_BACKSPACE) user->onKeyboardText("\x08");
    if (key == GLFW_KEY_ENTER) user->onKeyboardText("\n");
  }
  KeyAction enumAction =
      (action == GLFW_PRESS
           ? KeyAction::PRESS
           : (action == GLFW_REPEAT ? KeyAction::REPEAT : KeyAction::RELEASE));
  user->onKeyboard(getKeyMinecraft(key), enumAction);
}

void GLFWGameWindow::_glfwCharCallback(GLFWwindow *window, unsigned int ch)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;

  user->onKeyboardText(cvt.to_bytes(ch));
}

void GLFWGameWindow::_glfwWindowCloseCallback(GLFWwindow *window)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
  glfwSetWindowShouldClose(window, GLFW_FALSE);
  user->onClose();
}

void GLFWGameWindow::_glfwWindowFocusCallback(GLFWwindow *window, int focused)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
  GLFWJoystickManager::onWindowFocused(user, focused == GLFW_TRUE);
  user->onFocusChange(user->focused = (focused == GLFW_TRUE));
}

void GLFWGameWindow::_glfwWindowContentScaleCallback(GLFWwindow *window,
                                                     float scalex, float scaley)
{
  GLFWGameWindow *user = (GLFWGameWindow *)glfwGetWindowUserPointer(window);
  user->setRelativeScale();
}
