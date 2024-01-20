#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "gamepad_ids.h"

namespace gamepad {

class Joystick;

class GamepadMapping {

public:
    struct MapFrom {
        enum class Type {
            BUTTON, AXIS, HAT
        };
        Type type;

        union {
            struct {
                int id;
            } button;
            struct {
                int id;
                float min, max;
            } axis;
            struct {
                int id;
                int mask;
            } hat;
        } d;
    };
    struct MapTo {
        enum class Type {
            BUTTON, AXIS
        };
        Type type;

        union {
            struct {
                GamepadButton id;
            } button;
            struct {
                GamepadAxis id;
                float min, max;
            } axis;
        } d;
    };
    struct Mapping {
        MapFrom from;
        MapTo to;
    };

    std::string guid;
    std::string name;
    std::vector<Mapping> mappings;


    void parse(std::string const& mapping);

    static bool isAxisActive(MapFrom const& map, float value);
    static float getAxisTransformedValue(Mapping const& map, float value);

private:
    static std::unordered_map<std::string, GamepadButton> createKnownButtonList();
    static std::unordered_map<std::string, GamepadAxis> createKnownAxisList();

    static std::size_t find_e(std::string const& s, char c, size_t a = 0);
    static int parse_int(std::string const& s, size_t o, size_t& ro);

};

}