#pragma once

#include "api_handler/endpoints/endpoint.hpp"
#include "model/domains/api.hpp"

class GetMapEndpoint : public Endpoint {
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
            return model::api::errors::map_not_found();
        else
            return responses::ok(map_ptr);
    }

  private:
    struct responses {
        static util::Response ok(const model::Map *map_ptr) {
            return util::Response::Json(http::status::ok, json::value_from(*map_ptr));
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/maps/"};
};