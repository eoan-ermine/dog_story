#pragma once

#include "model.hpp"
#include "util/response.hpp"
#include <string>

namespace api_handler {

using namespace util;

using verb = boost::beast::http::verb;

class APIHandler {
  public:
    APIHandler(model::Game &game) : game_(game) {}

    bool dispatch(Response &response, verb method, std::string_view target, std::string_view body);

  private:
    // Handle get maps request
    // Endpoint: /api/v1/maps
    Response get_maps() const;

    // Handle get map request
    // Endpoint: /api/v1/maps/{id}
    Response get_map(model::Map::Id id) const;

    // Endpoint: /api/v1/game/join
    Response join(std::string username, model::Map::Id id);

    // Handle request if endpoint doesn't match requirements of other handlers
    Response fallthrough() const;

    std::string_view maps_endpoint{"/api/v1/maps"};
    std::string_view join_endpoint{"/api/v1/game/join"};
    model::Game &game_;
};

} // namespace api_handler
