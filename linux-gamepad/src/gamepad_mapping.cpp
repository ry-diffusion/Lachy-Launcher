#include <gamepad/gamepad_mapping.h>
#include <stdexcept>
#include <cmath>

using namespace gamepad;

std::size_t GamepadMapping::find_e(std::string const& s, char c, size_t a) {
    auto p = s.find(c, a);
    if (p == std::string::npos)
        throw std::invalid_argument("Invalid mapping");
    return p;
}

int GamepadMapping::parse_int(std::string const& s, size_t o, size_t& ro) {
    char* endp;
    long ret = std::strtol(s.data() + o, &endp, 10);
    if (endp == nullptr || endp == s.data() + o)
        throw std::invalid_argument("Invalid integer");
    ro = (size_t) (endp - s.data());
    return (int) ret;
}

std::unordered_map<std::string, GamepadButton> GamepadMapping::createKnownButtonList() {
    std::unordered_map<std::string, GamepadButton> knownButtons;
    knownButtons["a"] = GamepadButton::A;
    knownButtons["b"] = GamepadButton::B;
    knownButtons["x"] = GamepadButton::X;
    knownButtons["y"] = GamepadButton::Y;
    knownButtons["leftshoulder"] = GamepadButton::LB;
    knownButtons["rightshoulder"] = GamepadButton::RB;
    knownButtons["back"] = GamepadButton::BACK;
    knownButtons["start"] = GamepadButton::START;
    knownButtons["guide"] = GamepadButton::GUIDE;
    knownButtons["leftstick"] = GamepadButton::LEFT_STICK;
    knownButtons["rightstick"] = GamepadButton::RIGHT_STICK;
    knownButtons["dpup"] = GamepadButton::DPAD_UP;
    knownButtons["dpright"] = GamepadButton::DPAD_RIGHT;
    knownButtons["dpdown"] = GamepadButton::DPAD_DOWN;
    knownButtons["dpleft"] = GamepadButton::DPAD_LEFT;
    return knownButtons;
}

std::unordered_map<std::string, GamepadAxis> GamepadMapping::createKnownAxisList() {
    std::unordered_map<std::string, GamepadAxis> knownAxis;
    knownAxis["leftx"] = GamepadAxis::LEFT_X;
    knownAxis["lefty"] = GamepadAxis::LEFT_Y;
    knownAxis["rightx"] = GamepadAxis::RIGHT_X;
    knownAxis["righty"] = GamepadAxis::RIGHT_Y;
    knownAxis["lefttrigger"] = GamepadAxis::LEFT_TRIGGER;
    knownAxis["righttrigger"] = GamepadAxis::RIGHT_TRIGGER;
    return knownAxis;
}

void GamepadMapping::parse(std::string const& mapping) {
    static std::unordered_map<std::string, GamepadButton> knownButtons = createKnownButtonList();
    static std::unordered_map<std::string, GamepadAxis> knownAxis = createKnownAxisList();

    size_t iof = find_e(mapping, ',');
    guid = mapping.substr(0, iof);
    iof = iof + 1;
    size_t iof2 = find_e(mapping, ',', iof);
    name = mapping.substr(iof, iof2 - iof);
    iof = iof2;
    while (iof != std::string::npos && iof + 1 < mapping.length()) {
        Mapping c;

        iof++;
        // parse target
        char toMod = 0;
        if (mapping[iof] == '-' || mapping[iof] == '+')
            toMod = mapping[iof++];
        iof2 = find_e(mapping, ':', iof);
        std::string from (mapping.substr(iof, iof2 - iof));
        auto btn = knownButtons.find(from);
        auto axis = knownAxis.find(from);
        if (btn != knownButtons.end()) {
            c.to.type = MapTo::Type::BUTTON;
            c.to.d.button.id = btn->second;
        } else if (axis != knownAxis.end()) {
            c.to.type = MapTo::Type::AXIS;
            c.to.d.axis.id = axis->second;
            if (axis->second == GamepadAxis::LEFT_TRIGGER || axis->second == GamepadAxis::RIGHT_TRIGGER) {
                c.to.d.axis.min = 0.f;
                c.to.d.axis.max = 1.f;
            } else if (toMod == 0) {
                c.to.d.axis.min = -1.f;
                c.to.d.axis.max = 1.f;
            } else {
                c.to.d.axis.min = 0.f;
                c.to.d.axis.max = toMod == '+' ? 1.f : -1.f;
            }
        } else {
            iof = mapping.find(',', iof + 1);

            continue;
        }
        iof = iof2 + 1;

        // parse source
        char fromMod = 0;
        if (mapping[iof] == '-' || mapping[iof] == '+')
            fromMod = mapping[iof++];
        bool inv = false;
        if (mapping[iof] == '~') {
            inv = true;
            iof++;
        }
        if (mapping[iof] == 'b') {
            c.from.type = MapFrom::Type::BUTTON;
            c.from.d.button.id = parse_int(mapping, iof + 1, iof);
        } else if (mapping[iof] == 'a') {
            c.from.type = MapFrom::Type::AXIS;
            c.from.d.axis.id = parse_int(mapping, iof + 1, iof);
            if (fromMod == 0) {
                c.from.d.axis.min = -1.f;
                c.from.d.axis.max = 1.f;
            } else {
                c.from.d.axis.min = 0.f;
                c.from.d.axis.max = fromMod == '+' ? 1.f : -1.f;
            }
            if (inv)
                std::swap(c.to.d.axis.min, c.to.d.axis.max);
        } else if (mapping[iof] == 'h') {
            c.from.type = MapFrom::Type::HAT;
            c.from.d.hat.id = parse_int(mapping, iof + 1, iof);
            if (mapping[iof] != '.')
                throw std::invalid_argument("Invalid mapping: expected . after hat id");
            c.from.d.hat.mask = parse_int(mapping, iof + 1, iof);
        } else {
            throw std::invalid_argument("Invalid mapping: invalid map-to");
        }
        mappings.push_back(c);

        iof = mapping.find(',', iof);
    }
}


bool GamepadMapping::isAxisActive(MapFrom const& map, float value) {
    if (map.d.axis.min < map.d.axis.max)
        return (value >= (map.d.axis.min + map.d.axis.max) / 2);
    else
        return (value <= (map.d.axis.min + map.d.axis.max) / 2);
}

float GamepadMapping::getAxisTransformedValue(Mapping const& map, float value) {
    auto& a = map.from.d.axis;
    auto& d = map.to.d.axis;
    if (value < std::min(a.min, a.max) || value > std::max(a.min, a.max))
        return NAN;
    value = (value - a.min) / (a.max - a.min);
    value = d.min + value * (d.max - d.min);
    return value;
}