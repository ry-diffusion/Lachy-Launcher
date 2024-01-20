//========================================================================
// GLFW 3.3 X11 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <dlfcn.h>
#include <stdio.h>
#include "xinput.h"
#include "eglutint.h"

struct _eglut_xinput_rt_t _eglut_xinput_rt;
struct eglut_xinput_state_t _eglut_xinput_state;

void _eglutInitXinputRt() {
    if (_eglut_xinput_rt.initialized)
        return;
    _eglut_xinput_rt.initialized = 1;
    void* h = _eglut_xinput_rt.handle = dlopen("libXi.so.6", RTLD_LAZY | RTLD_LOCAL);
    if (h == NULL)
        return;
    _eglut_xinput_rt.QueryVersion = dlsym(h, "XIQueryVersion");
    _eglut_xinput_rt.QueryDevice = dlsym(h, "XIQueryDevice");
    _eglut_xinput_rt.FreeDeviceInfo = dlsym(h, "XIFreeDeviceInfo");
    _eglut_xinput_rt.SelectEvents = dlsym(h, "XISelectEvents");
    _eglut_xinput_rt.GrabDevice = dlsym(h, "XIGrabDevice");
    _eglut_xinput_rt.UngrabDevice = dlsym(h, "XIUngrabDevice");
}

int _eglutCheckXinput() {
    if (_eglut_xinput_rt.handle == NULL)
        return 0;
    int event, error;
    if (!XQueryExtension(_eglut->native_dpy, "XInputExtension", &_eglut_xinput_state.opcode, &event, &error))
        return 0;
    int major = 2, minor = 2;
    if (XIQueryVersion(_eglut->native_dpy, &major, &minor) == BadRequest)
        return 0;
    return 1;
}

void _eglutInitXinput() {
    if (_eglut_xinput_state.initialized)
        return;
    _eglut_xinput_state.initialized = 1;
    _eglutInitXinputRt();
    _eglut_xinput_state.available = _eglutCheckXinput();
}

int _eglutIsXinputAvailable() {
    return _eglut_xinput_state.available;
}

int _eglutXinputSetRawMotion(int raw) {
    if (!_eglut_xinput_state.available)
        return 0;
    if (!_eglut->native_dpy)
        return 0;
    int devc = 0;
    XIDeviceInfo* devices = XIQueryDevice(_eglut->native_dpy, XIAllMasterDevices, &devc);
    if (raw) {
        XIEventMask em;
        unsigned char mask[XIMaskLen(XI_LASTEVENT)] = { 0 };

        em.deviceid = XIAllMasterDevices;
        em.mask_len = sizeof(mask);
        em.mask = mask;
        XISetMask(mask, XI_Motion);
        XISetMask(mask, XI_ButtonPress);
        XISetMask(mask, XI_ButtonRelease);
        XISetMask(mask, XI_RawMotion);
        XISetMask(mask, XI_Motion);
        XISetMask(mask, XI_TouchBegin);
        XISetMask(mask, XI_TouchEnd);
        XISetMask(mask, XI_TouchUpdate);

        for (int i = 0; i < devc; i++) {
            if (devices[i].use == XIMasterPointer) {
                XUngrabPointer(_eglut->native_dpy, CurrentTime);
                XIGrabDevice(_eglut->native_dpy, devices[i].deviceid, _eglut->current->native.u.window, CurrentTime,
                        None, XIGrabModeAsync, XIGrabModeAsync, False, &em);
            }
        }
    } else {
        for (int i = 0; i < devc; i++) {
            if (devices[i].use == XIMasterPointer)
                XIUngrabDevice(_eglut->native_dpy, devices[i].deviceid, CurrentTime);
        }
    }
    XIFreeDeviceInfo(devices);
    return 1;
}