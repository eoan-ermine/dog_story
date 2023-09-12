#pragma once

#include <boost/json.hpp>
#include <string_view>

namespace model {

using namespace boost::json;

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

enum class Direction {
    NORTH, // Север
    SOUTH, // Юг
    WEST,  // Запад
    EAST,  // Восток
    NO,
};

Direction parse(std::string_view direction);
std::string_view serialize(Direction direction);

enum class Orientation {
    HORIZONTAL,
    VERTICAL,
};

} // namespace model