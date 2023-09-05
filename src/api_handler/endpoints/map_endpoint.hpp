#pragma once

#include "endpoint.hpp"

class MapEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override {
        return request.target().starts_with(endpoint) && !request.target().ends_with(endpoint);
    }
    util::Response handle(const http::request<http::string_body> &request) override {
        std::string_view map_ident = request.target().substr(endpoint.size());
        return execute(model::Map::Id{std::string{map_ident}});
    }
    util::Response execute(model::Map::Id map_ident) {
        const auto *map_ptr = game_.FindMap(map_ident);

        if (!map_ptr)
            return responses::not_found();
        else
            return responses::ok(map_ptr);
    }

  private:
    struct responses {
        static util::Response not_found() {
            return util::Response::Json(http::status::not_found,
                                        json::value_from(util::Error{"mapNotFound", "Map not found"}));
        }
        static util::Response ok(const model::Map *map_ptr) {
            return util::Response::Json(http::status::ok, json::value_from(*map_ptr));
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/maps/"};
};