#pragma once

#include <algorithm>
#include <boost/json.hpp>
#include <cctype>
#include <unordered_map>

#include "http_server.hpp"
#include "model.hpp"
#include "util/error.hpp"
#include "util/filesystem.hpp"
#include "util/mime_type.hpp"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace fs = std::filesystem;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

// Создает text response
StringResponse MakeTextResponse(http::status status, std::string_view body);

// Создаёт json response
StringResponse MakeJsonResponse(http::status status, std::string_view body);

// Создает file response
FileResponse MakeFileResponse(http::status status, std::string_view mime_type, std::string_view filepath,
                              boost::system::error_code &ec);

template <typename Body>
void FinalizeResponse(http::response<Body> &response, unsigned http_version, bool keep_alive) {
    response.version(http_version);
    response.keep_alive(keep_alive);
}

class RequestHandler {
  public:
    explicit RequestHandler(model::Game &game, std::string_view static_path) : game_{game}, base_path_(static_path) {}

    RequestHandler(const RequestHandler &) = delete;
    RequestHandler &operator=(const RequestHandler &) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>> &&request, Send &&send) const {
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
            response = json_response(http::status::bad_request,
                                     json::value_from(util::Error{"badRequest"sv, "Bad request"sv}));
        } else {
            auto path = GetPath(target, base_path_);
            if (ValidatePath(path, base_path_)) {
                auto extension = path.extension().string();

                boost::system::error_code ec;
                auto file_response = MakeFileResponse(http::status::ok, GetMimeType(extension), path.string(), ec);
                if (!ec) {
                    FinalizeResponse(file_response, request.version(), request.keep_alive());
                    send(file_response);
                    return;
                } else {
                    response = MakeTextResponse(http::status::not_found, "File not found");
                }
            } else {
                response = MakeTextResponse(http::status::bad_request, "Invalid path");
            }
        }

        FinalizeResponse(response, request.version(), request.keep_alive());
        send(response);
    }

  private:
    StringResponse json_response(http::status status, json::value value) const {
        return MakeJsonResponse(status, json::serialize(value));
    }

    // Handle get maps request
    // Endpoint: /api/v1/maps
    StringResponse get_maps() const { return json_response(http::status::ok, json::value_from(game_.GetMaps())); }

    // Handle get map request
    // Endpoint: /api/v1/maps/{id}
    StringResponse get_map(model::Map::Id id) const {
        const auto *map_ptr = game_.FindMap(id);

        if (map_ptr == nullptr)
            return json_response(http::status::not_found,
                                 json::value_from(util::Error{"mapNotFound"sv, "Map not found"sv}));
        else
            return json_response(http::status::ok, json::value_from(*map_ptr));
    }

    model::Game &game_;
    fs::path base_path_;
};

} // namespace request_handler
