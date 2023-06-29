#pragma once

#include <boost/json.hpp>

#include "model.h"

namespace json_serializer {

using namespace model;

boost::json::value serialize_error(std::string_view code, std::string_view message);
boost::json::value serialize(const Game::Maps& maps);
boost::json::value serialize(const Map& map);

}  // namespace json_serializer
