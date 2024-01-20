#pragma once

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

extern struct _eglut_xinput_rt_t {
    int initialized;
    void* handle;
    Status (*QueryVersion)(Display*, int*, int*);
    XIDeviceInfo* (*QueryDevice)(Display*, int, int*);
    void (*FreeDeviceInfo)(XIDeviceInfo*);
    Status (*SelectEvents)(Display* dpy, Window win, XIEventMask* masks, int num_masks);
    Status (*GrabDevice)(Display* dpy, int devid, Window win, Time time, Cursor cursor, int mode, int paired_mode,
            Bool owner_ev, XIEventMask* mask);
    Status (*UngrabDevice)(Display* dpy, int devid, Time time);
} _eglut_xinput_rt;

extern struct eglut_xinput_state_t {
    int initialized;
    int available;
    int opcode;
} _eglut_xinput_state;

void _eglutInitXinputRt();
void _eglutInitXinput();

int _eglutIsXinputAvailable();

int _eglutXinputSetRawMotion(int raw);

#define XIQueryVersion (_eglut_xinput_rt.QueryVersion)
#define XIQueryDevice (_eglut_xinput_rt.QueryDevice)
#define XIFreeDeviceInfo (_eglut_xinput_rt.FreeDeviceInfo)
#define XISelectEvents (_eglut_xinput_rt.SelectEvents)
#define XIGrabDevice (_eglut_xinput_rt.GrabDevice)
#define XIUngrabDevice (_eglut_xinput_rt.UngrabDevice)