/*
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <X11/extensions/XInput2.h>

#include "eglutint.h"
#include "eglut_x11.h"
#include "xinput.h"

void
_eglutNativeInitDisplay(void)
{
    char * res = getenv("MCPELAUNCHER_FEATURE_IME");
    if(res && strcmp(res, "1") == 0) {
        setlocale(LC_CTYPE, "");
        XSetLocaleModifiers("");
    }

    _eglut->native_dpy = XOpenDisplay(_eglut->display_name);
    if (!_eglut->native_dpy)
        _eglutFatal("failed to initialize native display");

    _eglut->surface_type = EGL_WINDOW_BIT;

    _eglutInitXinput();
}

void
_eglutNativeFiniDisplay(void)
{
    XCloseDisplay(_eglut->native_dpy);
    _eglut->native_dpy = NULL;
}


static char *_eglutClassName;
static char *_eglutClassInstanceName;
XIC x11_ic;

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
    XVisualInfo *visInfo, visTemplate;
    int num_visuals;
    Window root, xwin;
    XSetWindowAttributes attr;
    unsigned long mask;
    EGLint vid;

    if (!eglGetConfigAttrib(_eglut->dpy,
                            win->config, EGL_NATIVE_VISUAL_ID, &vid))
        _eglutFatal("failed to get visual id");

    /* The X window visual must match the EGL config */
    visTemplate.visualid = vid;
    visInfo = XGetVisualInfo(_eglut->native_dpy,
                             VisualIDMask, &visTemplate, &num_visuals);
    if (!visInfo)
        _eglutFatal("failed to get an visual of id 0x%x", vid);

    root = RootWindow(_eglut->native_dpy, DefaultScreen(_eglut->native_dpy));

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(_eglut->native_dpy,
                                    root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    xwin = XCreateWindow(_eglut->native_dpy, root, x, y, w, h,
                         0, visInfo->depth, InputOutput, visInfo->visual, mask, &attr);
    if (!xwin)
        _eglutFatal("failed to create a window");

    XFree(visInfo);

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.x = x;
        sizehints.y = y;
        sizehints.width  = w;
        sizehints.height = h;
        if (x == -1 && y == -1) {
            sizehints.win_gravity = CenterGravity;
            sizehints.flags = USSize | PWinGravity;
        } else {
            sizehints.flags = USSize | USPosition;
        }
        XSetNormalHints(_eglut->native_dpy, xwin, &sizehints);
        XSetStandardProperties(_eglut->native_dpy, xwin,
                               title, title, None, (char **) NULL, 0, &sizehints);

        XClassHint hints;
        hints.res_name = _eglutClassInstanceName;
        hints.res_class = _eglutClassName;
        if (!_eglutClassName)
            hints.res_class = _eglutClassInstanceName;
        XSetClassHint(_eglut->native_dpy, xwin, &hints);
    }

    win->native.u.window = xwin;
    win->native.width = w;
    win->native.height = h;

    if (_eglutIsXinputAvailable()) {
        XIEventMask em;
        unsigned char mask[XIMaskLen(XI_LASTEVENT)] = { 0 };
        em.deviceid = XIAllMasterDevices;
        em.mask_len = sizeof(mask);
        em.mask = mask;
        XISetMask(mask, XI_TouchBegin);
        XISetMask(mask, XI_TouchUpdate);
        XISetMask(mask, XI_TouchEnd);
        XISelectEvents(_eglut->native_dpy, xwin, &em, 1);
    }

    Atom WM_DELETE_WINDOW = XInternAtom(_eglut->native_dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(_eglut->native_dpy, xwin, &WM_DELETE_WINDOW, 1);

    XIM im = XOpenIM(_eglut->native_dpy, NULL, NULL, NULL);
    if (im != NULL) {
        x11_ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, xwin, NULL);
    }

    Atom wm_pid = XInternAtom(_eglut->native_dpy, "_NET_WM_PID", False);
    Atom cardinal = XInternAtom(_eglut->native_dpy, "CARDINAL", False);
    pid_t pid = getpid();
    XChangeProperty(_eglut->native_dpy, xwin, wm_pid, cardinal, 32, PropModeReplace, (void*) &pid, sizeof(pid));
}

void eglutSetWindowIcon(const char *path)
{
    unsigned int img_w, img_h;
    Atom wm_icon = XInternAtom(_eglut->native_dpy, "_NET_WM_ICON", False);
    Atom cardinal = XInternAtom(_eglut->native_dpy, "CARDINAL", False);

    unsigned char* img_data = _eglutReadPNG(path, &img_w, &img_h);
    if (img_data != NULL) {
        unsigned char* data = malloc((img_w * img_h + 2) * 4);
        *((unsigned int*) &data[0]) = img_w;
        *((unsigned int*) &data[4]) = img_h;
        for (ssize_t i = (img_w * img_h - 1) * 4; i >= 0; i -= 4) {
            data[8 + i + 3] = img_data[i + 3];
            data[8 + i + 2] = img_data[i + 0];
            data[8 + i + 1] = img_data[i + 1];
            data[8 + i + 0] = img_data[i + 2];
        }
        XChangeProperty(_eglut->native_dpy, _eglut->current->native.u.window, wm_icon, cardinal, 32, PropModeReplace,
                data, img_w * img_h + 2);
        //free(data);
    }
    free(img_data);
}

void eglutShowWindow()
{
    XMapWindow(_eglut->native_dpy, _eglut->current->native.u.window);

    if (x11_ic != NULL) {
        XSetICFocus(x11_ic);
    }
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
    XDestroyWindow(_eglut->native_dpy, win->native.u.window);
}

static int
lookup_keysym(KeySym sym)
{
    int special;

    switch (sym) {
        case XK_F1:
            special = EGLUT_KEY_F1;
            break;
        case XK_F2:
            special = EGLUT_KEY_F2;
            break;
        case XK_F3:
            special = EGLUT_KEY_F3;
            break;
        case XK_F4:
            special = EGLUT_KEY_F4;
            break;
        case XK_F5:
            special = EGLUT_KEY_F5;
            break;
        case XK_F6:
            special = EGLUT_KEY_F6;
            break;
        case XK_F7:
            special = EGLUT_KEY_F7;
            break;
        case XK_F8:
            special = EGLUT_KEY_F8;
            break;
        case XK_F9:
            special = EGLUT_KEY_F9;
            break;
        case XK_F10:
            special = EGLUT_KEY_F10;
            break;
        case XK_F11:
            special = EGLUT_KEY_F11;
            break;
        case XK_F12:
            special = EGLUT_KEY_F12;
            break;
        case XK_KP_Left:
        case XK_Left:
            special = EGLUT_KEY_LEFT;
            break;
        case XK_KP_Up:
        case XK_Up:
            special = EGLUT_KEY_UP;
            break;
        case XK_KP_Right:
        case XK_Right:
            special = EGLUT_KEY_RIGHT;
            break;
        case XK_KP_Down:
        case XK_Down:
            special = EGLUT_KEY_DOWN;
            break;
        default:
            special = -1;
            break;
    }

    return special;
}

static int _eglutRelativeMovementEnabled;
static int _eglutRelativeMovementLastX, _eglutRelativeMovementLastY;
static int _eglutRelativeMovementRawMode;

static int _eglutFocused = 1;

static char *_eglutClipboard;

static void
handle_xinput_event(struct eglut_window *win, XEvent* event)
{
    switch (event->xcookie.evtype) {
        case XI_RawMotion: {
            XIRawEvent *data = event->xcookie.data;
            double const *val = data->raw_values;
            double x = 0.f, y = 0.f;
            if (data->valuators.mask_len >= 1 && XIMaskIsSet(data->valuators.mask, 0))
                x = *(val++);
            if (data->valuators.mask_len >= 1 && XIMaskIsSet(data->valuators.mask, 1))
                y = *(val++);
            if (win->mouse_raw_cb)
                win->mouse_raw_cb(x, y);
            break;
        }
        case XI_Motion:
        {
            XIDeviceEvent *data = event->xcookie.data;
            if (_eglutRelativeMovementEnabled) {
                _eglutRelativeMovementLastX = (int) data->event_x;
                _eglutRelativeMovementLastY = (int) data->event_y;
                break;
            }
            if (win->mouse_cb)
                win->mouse_cb((int) data->event_x, (int) data->event_y);
            break;
        }
        case XI_ButtonPress: {
            XIDeviceEvent *data = event->xcookie.data;
            if (win->mouse_button_cb)
                win->mouse_button_cb((int) data->event_x, (int) data->event_y, data->detail, EGLUT_MOUSE_PRESS);
            break;
        }
        case XI_ButtonRelease: {
            XIDeviceEvent *data = event->xcookie.data;
            if (win->mouse_button_cb)
                win->mouse_button_cb((int) data->event_x, (int) data->event_y, data->detail, EGLUT_MOUSE_RELEASE);
            break;
        }
        case XI_TouchBegin: {
            XIDeviceEvent *data = event->xcookie.data;
            if (win->touch_start_cb)
                win->touch_start_cb(data->detail, data->event_x, data->event_y);
            break;
        }
        case XI_TouchUpdate: {
            XIDeviceEvent *data = event->xcookie.data;
            if (win->touch_update_cb)
                win->touch_update_cb(data->detail, data->event_x, data->event_y);
            break;
        }
        case XI_TouchEnd: {
            XIDeviceEvent *data = event->xcookie.data;
            if (win->touch_end_cb)
                win->touch_end_cb(data->detail, data->event_x, data->event_y);
            break;
        }
    }
}

static void
next_event(struct eglut_window *win)
{
    int redraw = 0;
    XEvent event, ahead;

    /* block for next event */
    XNextEvent(_eglut->native_dpy, &event);
    if((win->keyboardstate || event.type != KeyPress && event.type != KeyRelease) && XFilterEvent(&event, None)) {
        _eglut->redisplay = redraw;
        return;
    }

    switch (event.type) {
        case Expose:
            redraw = 1;
            break;
        case ConfigureNotify:
            win->native.x = event.xconfigure.x;
            win->native.y = event.xconfigure.y;
            win->native.width = event.xconfigure.border_width;
            win->native.width = event.xconfigure.width;
            win->native.height = event.xconfigure.height;
            if (win->reshape_cb)
                win->reshape_cb(win->native.width, win->native.height);
            break;
        case GenericEvent:
        {
            if (XGetEventData(_eglut->native_dpy, &event.xcookie) &&
                    event.xcookie.extension == _eglut_xinput_state.opcode) {
                handle_xinput_event(win, &event);
            }
            break;
        }
        case KeyPress:
        case KeyRelease:
        {
            char _buffer[8];
            char* buffer = _buffer;
            memset(_buffer, 0, sizeof(_buffer));
            KeySym sym;
            int r;
            int type;
            if (event.type == KeyPress) {
                int status;
                r = Xutf8LookupString(x11_ic, &event.xkey, buffer, sizeof(_buffer), &sym, &status);
                if(status == XBufferOverflow) {
                    buffer = malloc(r + 1);
                    r = Xutf8LookupString(x11_ic, (XKeyPressedEvent*) &event, buffer, r + 1, &sym, &status);
                }
                type = EGLUT_KEY_PRESS;
            } else {
                r = XLookupString(&event.xkey, buffer, sizeof(_buffer), &sym, NULL);
                type = EGLUT_KEY_RELEASE;
            }

            if (event.type == KeyRelease) {
                if (XEventsQueued(_eglut->native_dpy, QueuedAfterReading)) {
                    XPeekEvent(_eglut->native_dpy, &ahead);
                    if (ahead.type == KeyPress &&
                        ahead.xkey.window == event.xkey.window &&
                        ahead.xkey.keycode == event.xkey.keycode &&
                        ahead.xkey.time == event.xkey.time) {
                        type = EGLUT_KEY_REPEAT;
                        XNextEvent(_eglut->native_dpy, &event);
                    }
                }
            }

            if (r > 0 && win->keyboard_cb) {
                win->keyboard_cb(buffer, type);
            }

            if (win->special_cb) {
                /*r = lookup_keysym(sym);
                if (r == -1)*/
                    r = sym;
                if (r >= 0)
                    win->special_cb(r, type);
            }
            if (type != EGLUT_KEY_REPEAT)
                redraw = 1;
            if (buffer != _buffer)
                free(buffer);
            break;
        }
        case MotionNotify:
        {
            if (_eglutRelativeMovementEnabled) {
                if (win->mouse_raw_cb && !_eglutRelativeMovementRawMode)
                    win->mouse_raw_cb(event.xmotion.x - _eglutRelativeMovementLastX, event.xmotion.y - _eglutRelativeMovementLastY);
                _eglutRelativeMovementLastX = event.xmotion.x;
                _eglutRelativeMovementLastY = event.xmotion.y;
                break;
            }
            if (win->mouse_cb)
                win->mouse_cb(event.xmotion.x, event.xmotion.y);
            break;
        }
        case ButtonPress:
        {
            if (win->mouse_button_cb)
                win->mouse_button_cb(event.xbutton.x, event.xbutton.y, event.xbutton.button, EGLUT_MOUSE_PRESS);
            break;
        }
        case ButtonRelease:
        {
            if (win->mouse_button_cb)
                win->mouse_button_cb(event.xbutton.x, event.xbutton.y, event.xbutton.button, EGLUT_MOUSE_RELEASE);
            break;
        }
        case FocusIn:
        {
            if (event.xfocus.mode == NotifyNormal || event.xfocus.mode == NotifyWhileGrabbed) {
                if (_eglutRelativeMovementEnabled)
                    eglutSetMousePointerLocked(EGLUT_POINTER_LOCKED);
                if (win->focus_cb)
                    win->focus_cb(EGLUT_FOCUSED);
            }
            break;
        }
        case FocusOut:
        {
            if (event.xfocus.mode == NotifyNormal || event.xfocus.mode == NotifyWhileGrabbed) {
                if (_eglutRelativeMovementEnabled) {
                    _eglutXinputSetRawMotion(0);
                    eglutSetMousePointerVisiblity(EGLUT_POINTER_VISIBLE);
                }
                if (win->focus_cb)
                    win->focus_cb(EGLUT_NOT_FOCUSED);
            }
            break;
        }
        case ClientMessage:
        {
            if ((ulong) event.xclient.data.l[0] == XInternAtom(_eglut->native_dpy, "WM_DELETE_WINDOW", False)) {
                if (win->close_cb) {
                    win->close_cb();
                } else {
                    if (_eglut->current)
                        eglutDestroyWindow(_eglut->current->index);
                    eglutFini();
                }
            }
            break;
        }
        case SelectionRequest:
        {
            Atom targets_atom = XInternAtom(_eglut->native_dpy, "TARGETS", False);
            Atom string_atom = XInternAtom(_eglut->native_dpy, "STRING", False);
            Atom utf8_string_atom = XInternAtom(_eglut->native_dpy, "UTF8_STRING", False);

            XEvent reply;
            reply.xselection.type = SelectionNotify;
            reply.xselection.requestor = event.xselectionrequest.requestor;
            reply.xselection.selection = event.xselectionrequest.selection;
            reply.xselection.target = event.xselectionrequest.target;
            reply.xselection.property = None;
            reply.xselection.time = event.xselectionrequest.time;
            if (event.xselectionrequest.target == targets_atom) {
                reply.xselection.property = event.xselectionrequest.property;
                Atom val[] = {utf8_string_atom, string_atom};
                XChangeProperty(_eglut->native_dpy, event.xselectionrequest.requestor, event.xselectionrequest.property,
                                targets_atom, 32, PropModeReplace, (unsigned char*) val, 2);
            } else if (event.xselectionrequest.target == string_atom ||
                       event.xselectionrequest.target == utf8_string_atom) {
                reply.xselection.property = event.xselectionrequest.property;
                XChangeProperty(_eglut->native_dpy, event.xselectionrequest.requestor, event.xselectionrequest.property,
                                event.xselectionrequest.target, 8, PropModeReplace, (unsigned char*) _eglutClipboard,
                                strlen(_eglutClipboard));
            }
            XSendEvent(_eglut->native_dpy, event.xselectionrequest.requestor, True, 0, &reply);
            break;
        }
        case SelectionNotify:
        {
            Atom utf8_string = XInternAtom(_eglut->native_dpy, "UTF8_STRING", False);
            Atom app_cb_atom = XInternAtom(_eglut->native_dpy, "EGLUT_CLIPBOARD", False);

            Atom type;
            int format;
            unsigned long length, remaining;
            unsigned char *data;
            if (!XGetWindowProperty(_eglut->native_dpy, _eglut->current->native.u.window, app_cb_atom, 0, 65536, False,
                                    utf8_string, &type, &format, &length, &remaining, &data) && data) {
                if (win->paste_cb)
                    win->paste_cb(data, length);
                XFree(data);
            }
            break;
        }
        default:
            ; /*no-op*/
    }

    _eglut->redisplay = redraw;
}

void
eglutPollEvents(void)
{
    struct eglut_window *win = _eglut->current;
    while (XPending(_eglut->native_dpy))
        next_event(win);

    if (_eglut->idle_cb)
        _eglut->idle_cb();

    if (_eglutRelativeMovementEnabled) {
        int cx = eglutGetWindowWidth() / 2;
        int cy = eglutGetWindowHeight() / 2;
        if (cx != _eglutRelativeMovementLastX || cy != _eglutRelativeMovementLastY) {
            _eglutRelativeMovementLastX = cx;
            _eglutRelativeMovementLastY = cy;
            eglutWarpMousePointer(cx, cy);
        }
    }
}

void
_eglutNativeEventLoop(void)
{
    while (1) {
        struct eglut_window *win = _eglut->current;

        if (_eglut->native_dpy == NULL)
            break;

        while (XPending(_eglut->native_dpy) && !_eglut->redisplay)
            next_event(win);

        if (!_eglut->redisplay && _eglut->idle_cb)
            _eglut->idle_cb();

        if (_eglutRelativeMovementEnabled) {
            int cx = eglutGetWindowWidth() / 2;
            int cy = eglutGetWindowHeight() / 2;
            if (cx != _eglutRelativeMovementLastX || cy != _eglutRelativeMovementLastY) {
                _eglutRelativeMovementLastX = cx;
                _eglutRelativeMovementLastY = cy;
                eglutWarpMousePointer(cx, cy);
            }
        }

        if (_eglut->redisplay) {
            _eglut->redisplay = 0;

            if (win->display_cb)
                win->display_cb();
        }
    }
}

void eglutInitX11ClassInstanceName(const char *value)
{
    if (_eglutClassInstanceName)
        free(_eglutClassInstanceName);
    if (value)
        _eglutClassInstanceName = strdup(value);
    else
        _eglutClassInstanceName = NULL;
}

void eglutInitX11ClassName(const char *value)
{
    if (_eglutClassName)
        free(_eglutClassName);
    if (value)
        _eglutClassName = strdup(value);
    else
        _eglutClassName = NULL;
}

void eglutWarpMousePointer(int x, int y) {
    if (!_eglut->native_dpy)
        return;
    XWarpPointer(_eglut->native_dpy, None, _eglut->current->native.u.window, 0, 0, 0, 0, x, y);
    XFlush(_eglut->native_dpy);
}

void eglutSetMousePointerLocked(int locked) {
    if (locked) {
        _eglutRelativeMovementLastX = eglutGetWindowWidth() / 2;
        _eglutRelativeMovementLastY = eglutGetWindowHeight() / 2;
        eglutWarpMousePointer(_eglutRelativeMovementLastX, _eglutRelativeMovementLastY);
    }
    if (locked && !_eglutRelativeMovementEnabled)
        XSetInputFocus(_eglut->native_dpy, _eglut->current->native.u.window, RevertToNone, CurrentTime);

    _eglutRelativeMovementEnabled = locked;
    _eglutRelativeMovementRawMode = _eglutXinputSetRawMotion(locked) && locked;
    eglutSetMousePointerVisiblity(!locked);
}

static int _eglutCursorGrabbed = 0;

void eglutSetMousePointerVisiblity(int visible) {
    if (!_eglut->native_dpy)
        return;
    if (visible == EGLUT_POINTER_INVISIBLE) {
        char emptyData[] = {0, 0, 0, 0, 0, 0, 0, 0};
        XColor black;
        black.red = 0;
        black.green = 0;
        black.blue = 0;
        Pixmap emptyBitmap = XCreateBitmapFromData(_eglut->native_dpy, _eglut->current->native.u.window, emptyData, 8, 8);
        Cursor cursor = XCreatePixmapCursor(_eglut->native_dpy, emptyBitmap, emptyBitmap, &black, &black, 0, 0);
        XDefineCursor(_eglut->native_dpy, _eglut->current->native.u.window, cursor);
        if (_eglutRelativeMovementEnabled && !_eglutRelativeMovementRawMode) {
            XGrabPointer(_eglut->native_dpy, _eglut->current->native.u.window, True,
                         PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                         GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);
            _eglutCursorGrabbed = 1;
        }
        XFreeCursor(_eglut->native_dpy, cursor);
        XFreePixmap(_eglut->native_dpy, emptyBitmap);
    } else if (visible == EGLUT_POINTER_VISIBLE) {
        XUndefineCursor(_eglut->native_dpy, _eglut->current->native.u.window);
        if (_eglutCursorGrabbed) {
            XUngrabPointer(_eglut->native_dpy, CurrentTime);
            _eglutCursorGrabbed = 0;
        }
    }
}

int eglutToggleFullscreen()
{
    // http://stackoverflow.com/questions/10897503/opening-a-fullscreen-opengl-window
    _eglut->window_fullscreen = (_eglut->window_fullscreen == EGLUT_WINDOWED ? EGLUT_FULLSCREEN : EGLUT_WINDOWED);
    Atom wm_state = XInternAtom(_eglut->native_dpy, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(_eglut->native_dpy, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = _eglut->current->native.u.window;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = _eglut->window_fullscreen;
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;

    XMapWindow(_eglut->native_dpy, _eglut->current->native.u.window);

    XSendEvent (_eglut->native_dpy, DefaultRootWindow(_eglut->native_dpy), False,
                SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    XFlush(_eglut->native_dpy);
    return -1;
}

void eglutRequestPaste()
{
    Atom clipboard = XInternAtom(_eglut->native_dpy, "CLIPBOARD", False);
    Window owner = XGetSelectionOwner(_eglut->native_dpy, clipboard);
    if (owner == None)
        return;
    Atom utf8_string = XInternAtom(_eglut->native_dpy, "UTF8_STRING", False);
    Atom app_cb_atom = XInternAtom(_eglut->native_dpy, "EGLUT_CLIPBOARD", False);
    XConvertSelection(_eglut->native_dpy, clipboard, utf8_string, app_cb_atom, _eglut->current->native.u.window, CurrentTime);
}

void
eglutSetClipboardText(const char* value)
{
    if (_eglutClipboard)
        free(_eglutClipboard);
    _eglutClipboard = strdup(value);

    Atom clipboard = XInternAtom(_eglut->native_dpy, "CLIPBOARD", False);
    XSetSelectionOwner(_eglut->native_dpy, clipboard, _eglut->current->native.u.window, CurrentTime);
    XFlush(_eglut->native_dpy);
}

void
eglutSwapBuffers() {
    eglSwapBuffers(_eglut->dpy, _eglut->current->surface);
}

void eglutSwapInterval(int interval) {
    eglSwapInterval(_eglut->dpy, interval);
}

Display* eglutGetDisplay() {
    return _eglut->native_dpy;
}

Window eglutGetWindowHandle() {
    return _eglut->current->native.u.window;
}

