#pragma once

#include "api_handler/endpoints/endpoint.hpp"
#include "model/domains/api.hpp"

class JoinEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override {
        if (request.method() != http::verb::post) {
            return model::api::errors::only_post();
        }

        try {
            auto [username, map_ident] =
                value_to<model::api::requests::JoinRequest>(boost::json::parse(request.body()));
            return execute(std::move(username), model::Map::Id{std::move(map_ident)});
        } catch (...) {
            return model::api::errors::parse_error();
        }
    }
    util::Response execute(std::string username, model::Map::Id map_ident) {
        if (username.empty()) {
            return model::api::errors::invalid_username();
        }

        std::shared_ptr<model::GameSession> session{game_.FindSession(map_ident)};
        if (!session) {
            std::shared_ptr<model::Map> map{game_.FindMap(map_ident)};
            if (!map) {
                return model::api::errors::map_not_found();
            }
            game_.AddSession(model::GameSession(map));
        }

        auto [player, token] = game_.AddPlayer(std::move(username), session);
        return responses::ok(player, token);
    }

  private:
    struct responses {
        static util::Response ok(const std::shared_ptr<model::Player> player, const model::Token &token) {
            return util::Response::Json(http::status::ok, json::value_from(model::api::responses::JoinResponse{
                                                              .authToken = token, .playerId = player->GetId()}))
                .no_cache();
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/game/join"};
};