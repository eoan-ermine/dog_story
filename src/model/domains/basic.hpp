#pragma once

namespace model {

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
    NORTH = 'N', // Север
    SOUTH = 'S', // Юг
    WEST = 'W',  // Запад
    EAST = 'E'   // Восток
};

} // namespace model