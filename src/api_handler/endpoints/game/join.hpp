#pragma once

#include "api_handler/endpoints/endpoint.hpp"

class JoinEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override {
        if (request.method() != http::verb::post) {
            return responses::invalid_method();
        }

        try {
            auto [username, map_ident] = value_to<model::JoinRequest>(boost::json::parse(request.body()));
            return execute(std::move(username), model::Map::Id{std::move(map_ident)});
        } catch (...) {
            return responses::parse_error();
        }
    }
    util::Response execute(std::string username, model::Map::Id map_ident) {
        if (username.empty()) {
            return responses::invalid_username();
        }

        std::shared_ptr<model::GameSession> session{game_.FindSession(map_ident)};
        if (!session) {
            std::shared_ptr<model::Map> map{game_.FindMap(map_ident)};
            if (!map) {
                return responses::map_not_found();
            }
            game_.AddSession(model::GameSession(map));
        }

        auto [player, token] = game_.AddPlayer(std::move(username), session);
        return responses::ok(player, token);
    }

  private:
    struct responses {
        static util::Response invalid_method() {
            return util::Response::Json(http::status::method_not_allowed,
                                        json::value_from(util::Error{.code = "invalidMethod",
                                                                     .message = "Only POST method is expected"}))
                .no_cache()
                .allow("POST");
        }
        static util::Response parse_error() {
            return util::Response::Json(http::status::bad_request,
                                        json::value_from(util::Error{.code = "invalidArgument",
                                                                     .message = "Join game request parse error"}))
                .no_cache();
        }
        static util::Response invalid_username() {
            return util::Response::Json(
                       http::status::bad_request,
                       json::value_from(util::Error{.code = "invalidArgument", .message = "Invalid username"}))
                .no_cache();
        }
        static util::Response map_not_found() {
            return util::Response::Json(
                       http::status::not_found,
                       json::value_from(util::Error{.code = "mapNotFound", .message = "Map not found"}))
                .no_cache();
        }
        static util::Response ok(const std::shared_ptr<model::Player> player, const model::Token &token) {
            return util::Response::Json(http::status::ok, json::value_from(model::JoinResponse{
                                                              .authToken = token, .playerId = player->GetId()}))
                .no_cache();
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/game/join"};
};