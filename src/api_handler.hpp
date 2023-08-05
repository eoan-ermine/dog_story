#pragma once

#include "model.hpp"
#include "util/response.hpp"

namespace api_handler {

using namespace util;

class APIHandler {
  public:
    APIHandler(model::Game &game) : game_(game) {}

    bool dispatch(Response &response, std::string_view target) const;

  private:
    // Handle get maps request
    // Endpoint: /api/v1/maps
    Response get_maps() const;

    // Handle get map request
    // Endpoint: /api/v1/maps/{id}
    Response get_map(model::Map::Id id) const;

    // Handle request if endpoint doesn't match requirements of other handlers
    Response fallthrough() const;

    std::string_view endpoint{"/api/v1/maps"};
    model::Game &game_;
};

} // namespace api_handler
