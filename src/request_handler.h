#pragma once

#include <boost/json.hpp>

#include "json_serializer.h"
#include "http_server.h"
#include "model.h"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = "text/html"sv);

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& request, Send&& send) {
        using namespace json_serializer;

        auto json_response = [&request](http::status status, json::value value) {
            return MakeStringResponse(status, json::serialize(value),
                                      request.version(), request.keep_alive(), "application/json"sv);
        };

        std::string_view endpoint = "/api/v1/maps";
        if (request.target() == endpoint) {
            send(json_response(http::status::ok, serialize(game_.GetMaps())));
        } else if (request.target().starts_with(endpoint)) {
            auto map_id = Map::Id{std::string{endpoint.substr(endpoint.size())}};
            const auto* map_ptr = maps_.FindMap(map_id)

            if (map_ptr == nullptr)
                send(json_response(http::status::bad_request,
                                   serialize_error("badRequest"sv, "Bad request")));
            else {
                send(json_response(http::status::ok, serialize(*map_ptr)));
            }
        }
    }

private:
    model::Game& game_;
};

}  // namespace request_handler
