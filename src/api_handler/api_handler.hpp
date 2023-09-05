#pragma once

#include "model.hpp"
#include "util/error.hpp"
#include "util/response.hpp"
#include <string>

namespace api_handler {

using namespace util;

using verb = boost::beast::http::verb;

class APIHandler {
  public:
    APIHandler(model::Game &game) : game_(game) {}

    template <typename Body, typename Allocator>
    bool dispatch(http::request<Body, http::basic_fields<Allocator>> &request, Response &response) {
        auto target = request.target();
        auto method = request.method();
        auto body = request.body();

        if (target == maps_endpoint) {
            response = get_maps();
        } else if (target.starts_with(maps_endpoint) && !target.ends_with("/")) {
            std::string_view id = target.substr(maps_endpoint.size() + 1);
            response = get_map(model::Map::Id{std::string{id}});
        } else if (target == join_endpoint) {
            if (method != verb::post) {
                response = Response::Json(
                    http::status::method_not_allowed,
                    json::value_from(util::Error{.code = "invalidMethod", .message = "Only POST method is expected"}));
                response.set("Allow", "POST");
                response.set("Cache-Control", "no-cache");
                return true;
            }

            try {
                auto [username, map_ident] = value_to<model::JoinRequest>(boost::json::parse(body));
                response = join(std::move(username), model::Map::Id{std::move(map_ident)});
            } catch (...) {
                response = Response::Json(http::status::bad_request,
                                          json::value_from(util::Error{.code = "invalidArgument",
                                                                       .message = "Join game request parse error"}));
                response.set("Cache-Control", "no-cache");
            }
        } else if (target == players_endpoint) {
            // TODO: Пустой токен засчитывается за валидный, исправить
            if (!request.count("Authorization") || !request.starts_with("Authorization: Bearer ")) {
                response = Response::Json(http::status::unauthorized,
                                          json::value_from(util::Error{.code = "invalidToken",
                                                                       .message = "Authorization header is missing"}));
                response.set("Cache-Control", "no-cache");
            } else if (method != verb::get && method != verb::head) {
                response =
                    Response::Json(http::status::method_not_allowed,
                                   json::value_from(util::Error{.code = "invalidMethod",
                                                                .message = "Only GET and HEAD methods are expected"}));
                response.set("Allow", "GET, HEAD");
                response.set("Cache-Control", "no-cache");
            } else {
                std::string token = request["Authorization"];
                response = get_players(std::move(token));
            }
        } else if (target.starts_with("/api/")) {
            response = fallthrough();
        } else {
            return false;
        }

        return true;
    }

  private:
    // Handle get maps request
    // Endpoint: /api/v1/maps
    Response get_maps() const;

    // Handle get map request
    // Endpoint: /api/v1/maps/{id}
    Response get_map(model::Map::Id id) const;

    // Endpoint: /api/v1/game/join
    Response join(std::string username, model::Map::Id id);

    // Endpoint: /api/v1/game/players
    Response get_players(std::string token);

    // Handle request if endpoint doesn't match requirements of other handlers
    Response fallthrough() const;

    std::string_view maps_endpoint{"/api/v1/maps"};
    std::string_view join_endpoint{"/api/v1/game/join"};
    std::string_view players_endpoint{"/api/v1/game/players"};
    model::Game &game_;
};

} // namespace api_handler
