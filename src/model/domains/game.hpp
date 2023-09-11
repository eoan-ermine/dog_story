#pragma once

#include <boost/json.hpp>

#include <memory>
#include <random>
#include <sstream>
#include <string>

#include "basic.hpp"
#include "map.hpp"

namespace model {

using namespace boost::json;

// Пес — персонаж, которым управляет игрок.
class Dog {
  public:
    using Id = util::Tagged<std::size_t, Dog>;

    static std::shared_ptr<Dog> Create(std::string name, const Map &map) {
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
            std::unreachable();
        }());

        return std::shared_ptr<Dog>(new Dog(Id{last_id++}, name, position));
    }

    Id GetId() const { return id_; }

    std::string_view GetName() const { return name_; }

    std::pair<double, double> GetPosition() const { return position_; }

    std::pair<double, double> GetSpeed() const { return speed_; }

    Direction GetDirection() const { return direction_; }

    void SetPosition(std::pair<double, double> position) { position_ = position; }

    void SetSpeed(std::pair<double, double> speed) { speed_ = speed; }

    void SetDirection(Direction direction) { direction_ = direction; }

  private:
    // После добавления на карту пёс должен иметь скорость, равную нулю. Направление пса по умолчанию — на север.
    Dog(Id id, std::string name, std::pair<double, double> position)
        : id_(id), name_(std::move(name)), position_(position), speed_({0.0, 0.0}), direction_(Direction::NORTH) {}

    Id id_;
    std::string name_;
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
    using Dogs = std::unordered_map<Dog::Id, std::shared_ptr<Dog>>;

    GameSession(const std::shared_ptr<Map> &map) : map_(map) {}

    void AddDog(std::shared_ptr<Dog> dog) { dogs_.insert({dog->GetId(), std::move(dog)}); }

    const Dogs &GetDogs() const { return dogs_; }

    std::shared_ptr<Map> GetMap() const { return map_; }

  private:
    Dogs dogs_;
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
    using Id = Dog::Id;

    Player(std::string name, std::shared_ptr<GameSession> session) : session_(session) {
        dog_ = Dog::Create(name, *session->GetMap());
        session->AddDog(dog_);
    }

    Id GetId() const { return dog_->GetId(); }

    std::string_view GetName() const { return dog_->GetName(); }

    std::shared_ptr<GameSession> GetSession() const { return session_; }

    std::shared_ptr<Dog> GetDog() const { return dog_; }

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
        if (token_to_player_.contains(token)) {
            return token_to_player_[token];
        }
        return nullptr;
    }

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
    std::shared_ptr<Player> Add(std::string name, std::shared_ptr<GameSession> session) {
        auto player = std::make_shared<Player>(name, session);
        const auto &dog = player->GetDog();

        players_[session->GetMap()->GetId()][dog->GetId()] = player;
        return player;
    }

    std::shared_ptr<Player> FindByDogIdAndMapId(Dog::Id dog_id, Map::Id map_id) const {
        return players_.at(map_id).at(dog_id);
    }

    const std::unordered_map<Dog::Id, std::shared_ptr<Player>> &GetPlayers(const Map::Id &map_id) {
        return players_[map_id];
    }

  private:
    std::unordered_map<Map::Id, std::unordered_map<Dog::Id, std::shared_ptr<Player>>> players_;
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

    Map *FindMap(const Map::Id &id) noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    void AddSession(GameSession &&session) { sessions_.push_back(std::move(session)); }

    GameSession *FindSession(const Map::Id &id) noexcept {
        auto it = std::find_if(sessions_.begin(), sessions_.end(),
                               [&](const auto &session) { return id == session.GetMap()->GetId(); });
        if (it == sessions_.end())
            return nullptr;
        return &(*it);
    }

    std::pair<std::shared_ptr<Player>, Token> AddPlayer(std::string username, std::shared_ptr<GameSession> session) {
        auto player = players_.Add(std::move(username), std::shared_ptr<model::GameSession>(session));
        auto token = player_tokens_.AddPlayer(player);
        return {player, token};
    }

    std::shared_ptr<Player> GetPlayer(std::string token) {
        return player_tokens_.FindPlayerByToken(Token{std::move(token)});
    }

    const std::unordered_map<Dog::Id, std::shared_ptr<Player>> &GetPlayers(const Map::Id &map_id) {
        return players_.GetPlayers(map_id);
    }

  private:
    using MapIdToIndex = std::unordered_map<Map::Id, size_t>;

    void AddMap(Map &&map);

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::vector<GameSession> sessions_;
    Players players_;
    PlayerTokens player_tokens_;
};

// Deserialize json value to game structure
Game tag_invoke(value_to_tag<Game>, const value &value);
// Serialize maps to json value
void tag_invoke(value_from_tag, value &value, const Game::Maps &maps);

} // namespace model