#include "api.hpp"
#include <chrono>

namespace model {

namespace api::requests {

JoinRequest tag_invoke(value_to_tag<JoinRequest>, const value &value) {
    const object &obj = value.as_object();
    return api::requests::JoinRequest{.userName = value_to<std::string>(obj.at("userName")),
                                      .mapId = value_to<std::string>(obj.at("mapId"))};
}

ActionRequest tag_invoke(value_to_tag<ActionRequest>, const value &value) {
    const object &obj = value.as_object();
    return ActionRequest{model::parse(obj.at("move").as_string())};
}

TickRequest tag_invoke(value_to_tag<TickRequest>, const value &value) {
    const object &obj = value.as_object();
    return TickRequest{std::chrono::milliseconds{obj.at("timeDelta").as_int64()}};
}

} // namespace api::requests

namespace api::responses {

void tag_invoke(value_from_tag, value &value, const JoinResponse &response) {
    value = {{"authToken", *response.authToken}, {"playerId", *response.playerId}};
}

void tag_invoke(value_from_tag, value &value, const GetPlayersResponse &response) {
    auto &obj = value.as_object();

    for (const auto &[key, value] : response.players) {
        boost::json::value ident = *value->GetId();
        obj[ident.as_string()] = {{"name", value->GetName()}};
    }
}

void tag_invoke(value_from_tag, value &value, const GetStateResponse &response) {
    auto &obj = value.as_object();
    obj["players"] = object{};

    for (const auto &[key, value] : response.players) {
        const auto &dog = value->GetDog();
        boost::json::value ident = *value->GetId();
        auto [x, y] = dog->GetPosition();
        auto [dx, dy] = dog->GetSpeed();
        obj[ident.as_string()] = {{"pos", {x, y}}, {"speed", {dx, dy}}, {"dir", serialize(dog->GetDirection())}};
    }
}

} // namespace api::responses

} // namespace model
