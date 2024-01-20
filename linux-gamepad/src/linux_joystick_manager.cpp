#include "linux_joystick_manager.h"
#include <gamepad/joystick_manager_factory.h>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

using namespace gamepad;

LinuxJoystickManager::~LinuxJoystickManager() {
}

void LinuxJoystickManager::initialize() {
    if (!udev) {
        udev = udev_new();
        if (!udev)
            throw std::runtime_error("Failed to initialize udev");
    }

    udevMonitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor, "input", NULL);
    udev_monitor_enable_receiving(udevMonitor);
    udevMonitorFd = udev_monitor_get_fd(udevMonitor);

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char* path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        if (dev == nullptr)
            continue;
        onDeviceAdded(dev);
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
}

void LinuxJoystickManager::poll() {
    if (udevMonitor != NULL) {
        while (true) {
            struct timeval tv;
            tv.tv_sec = tv.tv_usec = 0;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(udevMonitorFd, &fds);

            int r = select(udevMonitorFd + 1, &fds, NULL, NULL, &tv);
            if (r <= 0 || !FD_ISSET(udevMonitorFd, &fds))
                break;
            struct udev_device* dev = udev_monitor_receive_device(udevMonitor);
            const char* action = udev_device_get_action(dev);
            if (strcmp(action, "add") == 0)
                onDeviceAdded(dev);
            else if (strcmp(action, "remove") == 0)
                onDeviceRemoved(dev);
            udev_device_unref(dev);
        }
    }

    for (auto const& js : joysticks)
        js->poll();
}

void LinuxJoystickManager::onDeviceAdded(struct udev_device* dev) {
    const char* val = udev_device_get_property_value(dev, "ID_INPUT_JOYSTICK");
    if (val != nullptr && strcmp(val, "1") == 0) {
        const char* devPath = udev_device_get_devnode(dev);
        if (devPath == nullptr)
            return;

        int fd = open(devPath, O_RDONLY | O_NONBLOCK);
        struct libevdev* edev = nullptr;
        int err = libevdev_new_from_fd(fd, &edev);
        if (err) {
            printf("libevdev_new_from_fd error %i (%s)\n", err, devPath);
            close(fd);
            return;
        }

        std::unique_ptr<LinuxJoystick> js (new LinuxJoystick(this, devPath, edev));
        onJoystickConnected(js.get());
        joysticks.push_back(std::move(js));
    }
}

void LinuxJoystickManager::onDeviceRemoved(struct udev_device* dev) {
    const char* devPath = udev_device_get_devnode(dev);
    if (devPath == nullptr)
        return;
    for (auto it = joysticks.begin(); it != joysticks.end(); it++) {
        if (strcmp(it->get()->getPath().c_str(), devPath) == 0) {
            onJoystickDisconnected(it->get());
            joysticks.erase(it);
            return;
        }
    }
}

std::shared_ptr<JoystickManager> JoystickManagerFactory::create() {
    return std::shared_ptr<JoystickManager>(new LinuxJoystickManager());
}
