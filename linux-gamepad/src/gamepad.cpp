#include <gamepad/gamepad.h>
#include <gamepad/gamepad_mapping.h>
#include <gamepad/joystick.h>
#include <cmath>

using namespace gamepad;

bool Gamepad::getButton(GamepadButton index) const {
    for (auto const& map : mapping.mappings) {
        if (map.to.type != GamepadMapping::MapTo::Type::BUTTON || map.to.d.button.id != index)
            continue;

        if (map.from.type == GamepadMapping::MapFrom::Type::BUTTON) {
            if (joystick.getButton(map.from.d.button.id))
                return true;
        } else if (map.from.type == GamepadMapping::MapFrom::Type::AXIS) {
            if (GamepadMapping::isAxisActive(map.from, joystick.getAxis(map.from.d.axis.id)))
                return true;
        } else  if (map.from.type == GamepadMapping::MapFrom::Type::HAT) {
            int v = joystick.getHat(map.from.d.hat.id);
            if (v & map.from.d.hat.mask)
                return true;
        }
    }
    return false;
}

float Gamepad::getAxis(GamepadAxis index) const {
    for (auto const& map : mapping.mappings) {
        if (map.to.type != GamepadMapping::MapTo::Type::AXIS || map.to.d.axis.id != index)
            continue;

        if (map.from.type == GamepadMapping::MapFrom::Type::BUTTON) {
            if (joystick.getButton(map.from.d.button.id))
                return map.to.d.axis.max;
        } else if (map.from.type == GamepadMapping::MapFrom::Type::AXIS) {
            float v = joystick.getAxis(map.from.d.axis.id);
            v = GamepadMapping::getAxisTransformedValue(map, v);
            if (std::isnan(v))
                continue;
            return v;
        } else  if (map.from.type == GamepadMapping::MapFrom::Type::HAT) {
            int v = joystick.getHat(map.from.d.hat.id);
            if (v & map.from.d.hat.mask)
                return map.to.d.axis.max;
        }
    }
    return 0.f;
}