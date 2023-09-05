#pragma once

#include "model.hpp"
#include "util/response.hpp"
#include <memory>

#include "endpoints/get_players_endpoint.hpp"
#include "endpoints/join_endpoint.hpp"
#include "endpoints/map_endpoint.hpp"
#include "endpoints/maps_endpoint.hpp"

namespace api_handler {

using namespace util;

using verb = boost::beast::http::verb;

class APIHandler {
  public:
    APIHandler(model::Game &game) : game_(game) {}

    template <typename Body, typename Allocator>
    bool dispatch(http::request<Body, http::basic_fields<Allocator>> &request, Response &response) {
        for (const auto &endpoint : endpoints) {
            if (endpoint->match(request)) {
                response = endpoint->handle(request);
                return true;
            }
        }

        return false;
    }

  private:
    model::Game &game_;
    std::vector<std::shared_ptr<Endpoint>> endpoints = {
        std::shared_ptr<Endpoint>{new MapEndpoint{game_}}, std::shared_ptr<Endpoint>{new MapsEndpoint{game_}},
        std::shared_ptr<Endpoint>{new JoinEndpoint{game_}}, std::shared_ptr<Endpoint>{new GetPlayersEndpoint(game_)}

    };
};

} // namespace api_handler
