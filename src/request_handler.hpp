#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <unordered_map>

#include "api_handler.hpp"
#include "http_server.hpp"
#include "model.hpp"
#include "util/error.hpp"
#include "util/filesystem.hpp"
#include "util/logging.hpp"
#include "util/mime_type.hpp"
#include "util/response.hpp"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;

class RequestHandler {
  public:
    explicit RequestHandler(model::Game &game, std::string_view static_path) : api_(game), base_path_(static_path) {}

    RequestHandler(const RequestHandler &) = delete;
    RequestHandler &operator=(const RequestHandler &) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(std::string_view address, http::request<Body, http::basic_fields<Allocator>> &&request,
                    Send &&send) const {
        auto target = request.target();

        LogRequest(address, target, request.method_string());
        auto start_ts = std::chrono::system_clock::now();

        Response response;
        if (!api_.dispatch(response, target)) {
            response = get_file(target);
        }

        auto end_ts = std::chrono::system_clock::now();
        LogResponse(std::chrono::duration_cast<std::chrono::milliseconds>(end_ts - start_ts).count(), response.code(),
                    response.content_type());

        response.finalize(request.version(), request.keep_alive());
        response.send(send);
    }

  private:
    // Handle static files requests
    Response get_file(std::string_view target) const {
        Response response;

        auto path = GetPath(target, base_path_);
        if (ValidatePath(path, base_path_)) {
            auto extension = path.extension().string();

            boost::system::error_code ec;
            response = Response::File(http::status::ok, GetMimeType(extension), path.string(), ec);
            if (ec)
                response = Response::Text(http::status::not_found, "File not found");
        } else {
            response = Response::Text(http::status::bad_request, "Invalid path");
        }

        return response;
    }

    api_handler::APIHandler api_;
    fs::path base_path_;
};

} // namespace request_handler
