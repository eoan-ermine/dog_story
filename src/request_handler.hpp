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
#include "util/response.hpp"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace fs = std::filesystem;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;

// Создает text response
StringResponse MakeTextResponse(http::status status, std::string_view body);

// Создаёт json response
StringResponse MakeJsonResponse(http::status status, std::string_view body);

// Создает file response
FileResponse MakeFileResponse(http::status status, std::string_view mime_type, std::string_view filepath,
                              boost::system::error_code &ec);

class RequestHandler {
  public:
    explicit RequestHandler(model::Game &game, std::string_view static_path) : game_{game}, base_path_(static_path) {}

    RequestHandler(const RequestHandler &) = delete;
    RequestHandler &operator=(const RequestHandler &) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>> &&request, Send &&send) const {
        auto target = request.target();
        std::string_view endpoint = "/api/v1/maps";
        Response response;

        if (target == endpoint) {
            response = get_maps();
        } else if (target.starts_with(endpoint) && !target.ends_with("/"sv)) {
            std::string_view id = target.substr(endpoint.size() + 1);
            response = get_map(model::Map::Id{std::string{id}});
        } else if (target.starts_with("/api/"sv)) {
            response = json_response(http::status::bad_request,
                                     json::value_from(util::Error{"badRequest"sv, "Bad request"sv}));
        } else {
            response = get_file(target);
        }

        response.finalize(request.version(), request.keep_alive());
        response.send(send);
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

    // Handle static files requests
    Response get_file(std::string_view target) const {
        Response response;

        auto path = GetPath(target, base_path_);
        if (ValidatePath(path, base_path_)) {
            auto extension = path.extension().string();

            boost::system::error_code ec;
            response = MakeFileResponse(http::status::ok, GetMimeType(extension), path.string(), ec);
            if (ec)
                response = MakeTextResponse(http::status::not_found, "File not found");
        } else {
            response = MakeTextResponse(http::status::bad_request, "Invalid path");
        }

        return response;
    }

    model::Game &game_;
    fs::path base_path_;
};

} // namespace request_handler
