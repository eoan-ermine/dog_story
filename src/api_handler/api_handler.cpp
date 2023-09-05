#include "api_handler.hpp"

#include <boost/json.hpp>

#include "model.hpp"
#include "util/error.hpp"
#include "util/response.hpp"

namespace api_handler {

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

Response APIHandler::get_players(std::string token) {
    auto player = game_.GetPlayer(token);
    if (!player) {
        auto response = Response::Json(
            http::status::unauthorized,
            json::value_from(util::Error{.code = "invalidToken", .message = "Player token has not been found"}));
        response.set("Cache-Control", "no-cache");
        return response;
    }
    const auto &players = game_.GetPlayers(player->GetSession()->GetMap()->GetId());
    auto response = Response::Json(http::status::ok, json::value_from(model::GetPlayersResponse{.players = players}));
    response.set("Cache-Control", "no-cache");
    return response;
}

Response APIHandler::fallthrough() const {
    return Response::Json(http::status::bad_request, json::value_from(util::Error{"badRequest", "Bad request"}));
}

} // namespace api_handler
