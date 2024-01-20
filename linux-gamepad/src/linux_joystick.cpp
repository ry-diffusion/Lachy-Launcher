#include "linux_joystick.h"
#include "linux_joystick_manager.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>

using namespace gamepad;

#define LBITS (sizeof(long) * 8)
#define NLONGS(x) (((x) - 1) / LBITS + 1)
#define TEST_BIT(v, i) !!((v)[(i) / LBITS] & (1L << ((i) % LBITS)))

LinuxJoystick::LinuxJoystick(LinuxJoystickManager* mgr, std::string const& path, struct libevdev* edev) :
        mgr(mgr), devPath(path), edev(edev) {
    int fd = libevdev_get_fd(edev);

    memset(buttons, 0xff, sizeof(buttons)); // sets all the buttons to -1 by default
    // buttonValues is zero-initialized by the bitset
    memset(axisValues, 0, sizeof(axisValues));

    unsigned long keys[NLONGS(KEY_CNT)];
    int len = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keys)), keys);
    if (len < 0)
        throw std::runtime_error("Failed to get joystick keys");
    len *= 8;

    int nextId = 0;
    // SDL first maps the buttons after BTN_JOYSTICK (BTN_JOYSTICK - BTN_MAX),
    // and then the ones before it (0 - BTN_JOYSTICK)
    for (size_t i = BTN_JOYSTICK; i < len + BTN_JOYSTICK; i++) {
        if (TEST_BIT(keys, i % len))
            buttons[i] = nextId++;
    }

    unsigned long abs[NLONGS(ABS_CNT)];
    len = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs)), abs);
    if (len < 0)
        throw std::runtime_error("Failed to get joystick abs");
    len *= 8;

    nextId = 0;
    for (size_t i = 0; i < len; i++) {
        if (!TEST_BIT(abs, i)) {
            axis[i].index = -1;
            continue;
        }
        const input_absinfo* absinfo = libevdev_get_abs_info(edev, i);
        if (absinfo == nullptr)
            continue;
        int id = isHat(i) ? 0 : (nextId++);
        axis[i] = {id, absinfo->minimum, absinfo->maximum, absinfo->flat, absinfo->fuzz};
    }
    nextId = 0;
    for (size_t i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2) {
        if (axis[i].index == -1 && axis[i + 1].index == -1)
            continue;
        axis[i].index = axis[i + 1].index = nextId++;
    }
}

static void writeLEShort(std::ostream& s, unsigned short v) {
    s << std::setw(2) << (v & 0xff) << std::setw(2) << ((v >> 8) & 0xff);
}
std::string LinuxJoystick::getGUID() const {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    writeLEShort(ss, (unsigned short) libevdev_get_id_bustype(edev));
    writeLEShort(ss, 0);
    writeLEShort(ss, (unsigned short) libevdev_get_id_vendor(edev));
    writeLEShort(ss, 0);
    writeLEShort(ss, (unsigned short) libevdev_get_id_product(edev));
    writeLEShort(ss, 0);
    writeLEShort(ss, (unsigned short) libevdev_get_id_version(edev));
    writeLEShort(ss, 0);
    return ss.str();
}

void LinuxJoystick::poll() {
    struct input_event e;
    while (true) {
        int r = libevdev_next_event(edev, LIBEVDEV_READ_FLAG_NORMAL, &e);
        if (r == -EAGAIN)
            break;
        if (r != 0) {
            printf("LinuxJoystick::pool error\n");
            break;
        }
        if (e.type == EV_KEY) {
            if (e.code >= KEY_CNT)
                continue;
            int btn = buttons[e.code];
            if (btn == -1)
                continue;
            bool v = e.value != 0;
            buttons[btn] = v;
            if (mgr)
                mgr->onJoystickButton(this, btn, v);
        } else if (e.type == EV_ABS && isHat(e.code)) {
            auto& a = axis[e.code];
            if (a.index == -1)
                continue;
            const bool y = (bool) (e.code & 1);
            int v = hatValues[a.index];
            // v (left, down, right, up)
            v &= ~(y ? 0b0101 : 0b1010);
            if (e.value != 0) {
                if (y)
                    v |= (e.value > 0 ? 4 : 1);
                else
                    v |= (e.value > 0 ? 2 : 8);
            }
            hatValues[a.index] = v;
            if (mgr)
                mgr->onJoystickHat(this, a.index, v);
        } else if (e.type == EV_ABS) {
            if (e.code >= AXIS_COUNT)
                continue;
            auto& a = axis[e.code];
            if (a.index == -1)
                continue;
            int iv = e.value - (a.min + a.max) / 2;
            float v;
            if (iv >= 0)
                v = (float) iv / (a.max - (a.min + a.max) / 2);
            else
                v = - (float) iv / (a.min - (a.min + a.max) / 2);
            if (std::abs(iv) < a.flat)
                v = 0.f;
            v = std::min(std::max(v, -1.f), 1.f);
            axisValues[a.index] = v;
            if (mgr)
                mgr->onJoystickAxis(this, a.index, v);
        }
    }
}
