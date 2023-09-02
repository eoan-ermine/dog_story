#include "api_handler.hpp"

#include <boost/json.hpp>

#include "model.hpp"
#include "util/error.hpp"
#include "util/response.hpp"

namespace api_handler {

bool APIHandler::dispatch(Response &response, verb method, std::string_view target, std::string_view body) {
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
            response = Response::Json(
                http::status::bad_request,
                json::value_from(util::Error{.code = "invalidArgument", .message = "Join game request parse error"}));
            response.set("Cache-Control", "no-cache");
        }

    } else if (target.starts_with("/api/")) {
        response = fallthrough();
    } else {
        return false;
    }
    return true;
}

Response APIHandler::get_maps() const { return Response::Json(http::status::ok, json::value_from(game_.GetMaps())); }

Response APIHandler::get_map(model::Map::Id id) const {
    const auto *map_ptr = game_.FindMap(id);

    if (map_ptr == nullptr)
        return Response::Json(http::status::not_found, json::value_from(util::Error{"mapNotFound", "Map not found"}));
    else
        return Response::Json(http::status::ok, json::value_from(*map_ptr));
}

Response APIHandler::join(std::string username, model::Map::Id id) {
    if (username.empty()) {
        auto response =
            Response::Json(http::status::bad_request,
                           json::value_from(util::Error{.code = "invalidArgument", .message = "Invalid meme"}));
        response.set("Cache-Control", "no-cache");
        return response;
    }

    std::shared_ptr<model::GameSession> session{game_.FindSession(id)};
    if (!session) {
        std::shared_ptr<model::Map> map{game_.FindMap(id)};
        if (!map) {
            auto response =
                Response::Json(http::status::not_found,
                               json::value_from(util::Error{.code = "mapNotFound", .message = "Map not found"}));
            response.set("Cache-Control", "no-cache");
            return response;
        }
        game_.AddSession(model::GameSession(map));
    }

    auto [player, token] = game_.AddPlayer(std::move(username), session);
    auto response = Response::Json(
        http::status::ok, json::value_from(model::JoinResponse{.authToken = token, .playerId = player->GetId()}));
    response.set("Cache-Control", "no-cache");
    return response;
}

Response APIHandler::fallthrough() const {
    return Response::Json(http::status::bad_request, json::value_from(util::Error{"badRequest", "Bad request"}));
}

} // namespace api_handler
