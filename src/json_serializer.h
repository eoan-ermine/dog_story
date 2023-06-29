#pragma once

#include <boost/json.hpp>

#include "model.h"

namespace json_serializer {

boost::json::value serialize(const Game::Maps& maps);
boost::json::value serialize(const Map& map);
boost::json::value serialize_error(std::string_view code, std::string_view message);

}  // namespace json_serializer
