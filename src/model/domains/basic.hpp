#pragma once

#include <boost/json.hpp>

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
    EAST   // Восток
};

std::string_view serialize(Direction direction);

} // namespace model