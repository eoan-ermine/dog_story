#pragma once

#include <boost/json.hpp>

#include <string>

#include "game.hpp"
#include "map.hpp"

namespace model {

using namespace boost::json;

struct JoinRequest {
    std::string userName;
    Map::Id::ValueType mapId;
};

// Deserialize json value to join request
JoinRequest tag_invoke(value_to_tag<JoinRequest>, const value &value);

struct JoinResponse {
    Token authToken;
    Player::Id playerId;
};

// Serialize join response to json value
void tag_invoke(value_from_tag, value &value, const JoinResponse &response);

struct GetPlayersResponse {
    const std::unordered_map<Player::Id, std::shared_ptr<Player>> &players;
};

// Serialize get players response to json value
void tag_invoke(value_from_tag, value &value, const GetPlayersResponse &response);

struct GetStateResponse {
    const std::unordered_map<Player::Id, std::shared_ptr<Player>> &players;
};

// Serialize get state response to json value
void tag_invoke(value_from_tag, value &value, const GetStateResponse &response);

} // namespace model