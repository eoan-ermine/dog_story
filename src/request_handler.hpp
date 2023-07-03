#pragma once

#include <boost/json.hpp>

#include "http_server.hpp"
#include "model.hpp"
#include "util/error.hpp"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Создаёт StringResponse с заданными параметрами
StringResponse MakeJsonResponse(http::status status, std::string_view body);

void FinalizeJsonResponse(StringResponse &response, unsigned http_version, bool keep_alive);

class RequestHandler {
  public:
    explicit RequestHandler(model::Game &game) : game_{game} {}

    RequestHandler(const RequestHandler &) = delete;
    RequestHandler &operator=(const RequestHandler &) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>> &&request, Send &&send) {
        auto target = request.target();
        std::string_view endpoint = "/api/v1/maps";
        StringResponse response;

        if (target == endpoint) {
            response = get_maps();
        } else if (target.starts_with(endpoint) && !target.ends_with("/"sv)) {
            std::string_view id = target.substr(endpoint.size() + 1);
            auto map_id = model::Map::Id{std::string{id}};
            response = get_map(map_id);
        } else if (target.starts_with("/api/"sv)) {
            response = bad_request();
        }

        FinalizeJsonResponse(response, request.version(), request.keep_alive());
        send(response);
    }

  private:
    StringResponse json_response(http::status status, json::value value) {
        return MakeJsonResponse(status, json::serialize(value));
    }

    // Handle get maps request
    // Endpoint: /api/v1/maps
    StringResponse get_maps() { return json_response(http::status::ok, json::value_from(game_.GetMaps())); }

    // Handle get map request
    // Endpoint: /api/v1/maps/{id}
    StringResponse get_map(model::Map::Id id) {
        const auto *map_ptr = game_.FindMap(id);

        if (map_ptr == nullptr)
            return json_response(http::status::not_found,
                                 json::value_from(util::Error{"mapNotFound"sv, "Map not found"sv}));
        else
            return json_response(http::status::ok, json::value_from(*map_ptr));
    }

    // Handle bad request
    // Endpoint: all except known endpoints
    StringResponse bad_request() {
        return json_response(http::status::bad_request, json::value_from(util::Error{"badRequest"sv, "Bad request"sv}));
    }

    model::Game &game_;
};

} // namespace request_handler
