#pragma once

#include "api_handler/endpoints/endpoint.hpp"

class GetPlayersEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override {
        auto method = request.method();

        // TODO: Пустой токен засчитывается за валидный, исправить
        if (!request.count("Authorization") || !request["Authorization"].starts_with("Authorization: Bearer ")) {
            return responses::no_token();
        } else if (method != http::verb::get && method != http::verb::head) {
            return responses::invalid_method();
        } else {
            return execute(request["Authorization"]);
        }
    }
    util::Response execute(std::string token) {
        auto player = game_.GetPlayer(token);
        if (!player) {
            return responses::no_user_found();
        }
        const auto &players = game_.GetPlayers(player->GetSession()->GetMap()->GetId());
        return responses::ok(players);
    }

  private:
    struct responses {
        static util::Response no_token() {
            return util::Response::Json(http::status::unauthorized,
                                        json::value_from(util::Error{.code = "invalidToken",
                                                                     .message = "Authorization header is missing"}))
                .no_cache();
        }
        static util::Response invalid_method() {
            return util::Response::Json(
                       http::status::method_not_allowed,
                       json::value_from(
                           util::Error{.code = "invalidMethod", .message = "Only GET and HEAD methods are expected"}))
                .no_cache()
                .allow("GET, HEAD");
        }
        static util::Response no_user_found() {
            auto response = util::Response::Json(
                http::status::unauthorized,
                json::value_from(util::Error{.code = "invalidToken", .message = "Player token has not been found"}));
            response.set("Cache-Control", "no-cache");
            return response;
        }
        static util::Response ok(const std::unordered_map<model::Player::Id, std::shared_ptr<model::Player>> &players) {
            return util::Response::Json(http::status::ok,
                                        json::value_from(model::api::responses::GetPlayersResponse{.players = players}))
                .no_cache();
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/game/players"};
};