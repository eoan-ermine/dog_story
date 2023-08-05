#pragma once

#include <boost/json.hpp>

#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "util/tagged.hpp"

namespace model {

using namespace boost::json;

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        HorizontalTag() = default;
    };

    struct VerticalTag {
        VerticalTag() = default;
    };

  public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept : start_{start}, end_{end_x, start.y} {}

    Road(VerticalTag, Point start, Coord end_y) noexcept : start_{start}, end_{start.x, end_y} {}

    bool IsHorizontal() const noexcept { return start_.y == end_.y; }

    bool IsVertical() const noexcept { return start_.x == end_.x; }

    Point GetStart() const noexcept { return start_; }

    Point GetEnd() const noexcept { return end_; }

  private:
    Point start_;
    Point end_;
};

// Serialize road structure to json value
void tag_invoke(value_from_tag, value &value, const Road &road);
// Deserialize json value to road structure
Road tag_invoke(value_to_tag<Road>, const value &value);

class Building {
  public:
    explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

    const Rectangle &GetBounds() const noexcept { return bounds_; }

  private:
    Rectangle bounds_;
};

// Serialize building structure to json value
void tag_invoke(value_from_tag, value &value, const Building &building);
// Deserialize json value to building structure
Building tag_invoke(value_to_tag<Building>, const value &value);

class Office {
  public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept : id_{std::move(id)}, position_{position}, offset_{offset} {}

    const Id &GetId() const noexcept { return id_; }

    Point GetPosition() const noexcept { return position_; }

    Offset GetOffset() const noexcept { return offset_; }

  private:
    Id id_;
    Point position_;
    Offset offset_;
};

// Serialize office structure to json value
void tag_invoke(value_from_tag, value &value, const Office &office);
// Deserialize json value to office structure
Office tag_invoke(value_to_tag<Office>, const value &value);

class Map {
  public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept : id_(std::move(id)), name_(std::move(name)) {}

    Map(Id id, std::string name, Roads &&roads, Buildings &&buildings, Offices &&offices) noexcept
        : Map(std::move(id), std::move(name)) {
        roads_ = std::move(roads);
        buildings_ = std::move(buildings);
        for (auto &&office : offices) {
            AddOffice(std::move(office));
        }
    }

    const Id &GetId() const noexcept { return id_; }

    const std::string &GetName() const noexcept { return name_; }

    const Buildings &GetBuildings() const noexcept { return buildings_; }

    const Roads &GetRoads() const noexcept { return roads_; }

    const Offices &GetOffices() const noexcept { return offices_; }

  private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    void AddOffice(Office &&office);

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

// Serialize map structure to json value
void tag_invoke(value_from_tag, value &value, const Map &map);
// Deserialize json value to map structure
Map tag_invoke(value_to_tag<Map>, const value &value);

class Dog {
  public:
    // TODO...
  private:
    std::size_t id_;
};

// Deserialize json value to dog structure
Dog tag_invoke(value_to_tag<Dog>, const value &value);
// Serialize dog to json value
void tag_invoke(value_from_tag, value &value, const Dog &dog);

class GameSession {
  public:
    GameSession(const std::shared_ptr<Map> &map) : map_(map) {}

  private:
    std::mutex mutex_;
    std::unordered_map<std::size_t, Dog> dogs_;
    std::shared_ptr<Map> map_;
};

// Deserialize json value to game session structure
GameSession tag_invoke(value_to_tag<GameSession>, const value &value);
// Serialize game session to json value
void tag_invoke(value_from_tag, value &value, const GameSession &session);

namespace detail {

struct TokenTag {};

} // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class Player {
  public:
    // TODO,,,
  private:
    std::shared_ptr<GameSession> session_;
    std::shared_ptr<Dog> dog_;
};

// Deserialize json value to player structure
Player tag_invoke(value_to_tag<Player>, const value &value);
// Serialize player to json value
void tag_invoke(value_from_tag, value &value, const Player &player);

class PlayerTokens {
  public:
    std::shared_ptr<Player> FindPlayerByToken(Token token) {
        // TODO...
    }

    Token AddPlayer(Player &player) {
        // TODO...
    }

  private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
};

class Players {
  public:
    Player &Add(Dog &dog, GameSession &session) {
        // TODO...
    }

    std::shared_ptr<Player> FindByDogIdAndMapId(std::size_t dog_id, std::size_t map_id) {
        // TODO...
    }

  private:
    // TODO...
};

class Game {
  public:
    using Maps = std::vector<Map>;

    explicit Game(Maps &&maps) {
        for (auto &&map : maps) {
            AddMap(std::move(map));
        }
    }

    const Maps &GetMaps() const noexcept { return maps_; }

    const Map *FindMap(const Map::Id &id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

  private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    void AddMap(Map &&map);

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
};

// Deserialize json value to game structure
Game tag_invoke(value_to_tag<Game>, const value &value);
// Serialize maps to json value
void tag_invoke(value_from_tag, value &value, const Game::Maps &maps);

} // namespace model
