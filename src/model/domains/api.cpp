#include "api.hpp"

namespace model {

JoinRequest tag_invoke(value_to_tag<JoinRequest>, const value &value) {
    const object &obj = value.as_object();
    return JoinRequest{.userName = value_to<std::string>(obj.at("userName")),
                       .mapId = value_to<std::string>(obj.at("mapId"))};
}

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

} // namespace model
