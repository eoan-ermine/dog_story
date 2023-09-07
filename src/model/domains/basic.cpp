#include "basic.hpp"
#include <utility>

using namespace std::literals;

namespace model {

std::string_view serialize(Direction direction) {
    switch (direction) {
    case model::Direction::EAST:
        return "E";
    case model::Direction::NORTH:
        return "N";
    case model::Direction::SOUTH:
        return "S";
    case model::Direction::WEST:
        return "W";
    }
    std::unreachable();
}

} // namespace model