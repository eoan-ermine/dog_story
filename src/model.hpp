#pragma once

#include <boost/json.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
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

enum class Direction {
    NORTH, // Север
    SOUTH, // Юг
    WEST,  // Запад
    EAST   // Восток
};

class Dog {
  public:
    using Id = util::Tagged<std::size_t, Dog>;

    static Dog Create(const Map &map) {
        // Координаты пса — случайно выбранная точка на случайно выбранном отрезке дороги этой карты
        static std::size_t last_id = 0;

        const auto &roads = map.GetRoads();
        std::mt19937_64 generator{[] {
            std::random_device random_device;
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device);
        }()};

        std::pair<double, double> position([&]() -> std::pair<double, double> {
            std::uniform_int_distribution<int> uniform_dist(0, roads.size() - 1);
            const auto &road = roads[uniform_dist(generator)];
            auto start = road.GetStart(), end = road.GetEnd();
            if (road.IsVertical()) {
                std::uniform_int_distribution<int> uniform_dist(start.y, end.y);
                return {start.x, uniform_dist(generator)};
            } else if (road.IsHorizontal()) {
                std::uniform_int_distribution<int> uniform_dist(start.x, end.x);
                return {uniform_dist(generator), start.y};
            }
        }());

        return Dog(Id{last_id++}, position);
    }

    Id GetId() const { return id_; }

    std::pair<double, double> GetPosition() const { return position_; }

    std::pair<double, double> GetSpeed() const { return speed_; }

    Direction GetDirection() const { return direction_; }

  private:
    // После добавления на карту пёс должен иметь скорость, равную нулю. Направление пса по умолчанию — на север.
    Dog(Id id, std::pair<double, double> position)
        : id_(id), position_(position), speed_({0.0, 0.0}), direction_(Direction::NORTH) {}

    Id id_;
    // Координаты пса на карте задаются двумя вещественными числами. Для описания вещественных координат разработайте
    // структуру или класс.
    std::pair<double, double> position_;
    // Скорость пса на карте задаётся также двумя вещественными числами. Скорость измеряется в единицах карты за одну
    // секунду
    std::pair<double, double> speed_;
    // Направление в пространстве принимает одно из четырех значений: NORTH (север), SOUTH (юг), WEST (запад), EAST
    // (восток).
    Direction direction_;
};

// Deserialize json value to dog structure
Dog tag_invoke(value_to_tag<Dog>, const value &value);
// Serialize dog to json value
void tag_invoke(value_from_tag, value &value, const Dog &dog);

class GameSession {
  public:
    using Dogs = std::unordered_map<Dog::Id, std::shared_ptr<Dog>, util::TaggedHasher<Dog::Id>>;

    GameSession(const std::shared_ptr<Map> &map) : map_(map) {}

    void AddDog(std::shared_ptr<Dog> dog) { dogs_.insert({dog->GetId(), std::move(dog)}); }

    const Dogs &GetDogs() const { return dogs_; }

    std::shared_ptr<Map> GetMap() const { return map_; }

  private:
    Dogs dogs_;
    const std::shared_ptr<Map> &map_;
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
    Player(std::shared_ptr<GameSession> session, std::shared_ptr<Dog> dog) : session_(session), dog_(dog) {
        session->AddDog(dog);
    }

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
    std::shared_ptr<Player> FindPlayerByToken(Token token) { return token_to_player_[token]; }

    Token AddPlayer(std::shared_ptr<Player> player) {
        std::stringstream stream;
        stream << std::hex << generator1_() << generator2_();

        Token token(stream.str());
        token_to_player_.insert({token, player});

        return token;
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

    std::unordered_map<Token, std::shared_ptr<Player>> token_to_player_;
};

class Players {
  public:
    Player &Add(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session) {
        auto player = std::make_shared<Player>(session, dog);

        std::pair<Dog::Id, std::shared_ptr<Player>> identity{dog->GetId(), player};

        players_[session->GetMap()->GetId()][dog->GetId()] = player;
        return *player;
    }

    std::shared_ptr<Player> FindByDogIdAndMapId(Dog::Id dog_id, Map::Id map_id) const {
        return players_.at(map_id).at(dog_id);
    }

  private:
    std::unordered_map<Map::Id, std::unordered_map<Dog::Id, std::shared_ptr<Player>, util::TaggedHasher<Dog::Id>>,
                       util::TaggedHasher<Map::Id>>
        players_;
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
