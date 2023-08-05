#include <boost/beast/http.hpp>
#include <boost/json.hpp>

#include <type_traits>
#include <variant>

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

class Response {
  public:
    Response() {}

    static Response Text(http::status status, std::string_view body) {
        StringResponse response;
        response.result(status);
        response.set(http::field::content_type, "text/plain");
        response.body() = body;
        response.content_length(body.size());

        Response result;
        result = std::move(response);
        return result;
    }

    static Response Json(http::status status, const json::value &value) {
        StringResponse response;
        response.result(status);
        response.set(http::field::content_type, "application/json");
        response.body() = json::serialize(value);
        response.content_length(response.body().size());

        Response result;
        result = std::move(response);
        return result;
    }

    static Response File(http::status status, std::string_view mime_type, std::string_view filepath,
                         boost::system::error_code &ec) {
        FileResponse response;
        response.result(status);
        response.set(http::field::content_type, mime_type);

        http::file_body::value_type file;
        file.open(filepath.data(), beast::file_mode::read, ec);
        if (!ec) {
            response.body() = std::move(file);
            response.prepare_payload();
        }

        Response result;
        result = std::move(response);
        return result;
    }

    Response &operator=(StringResponse &&response_) {
        response = std::move(response_);
        return *this;
    }
    Response &operator=(FileResponse &&response_) {
        response = std::move(response_);
        return *this;
    }

    int code() const {
        int code;
        std::visit([&](auto &&arg) { code = arg.result_int(); }, response);
        return code;
    }

    std::string_view content_type() const {
        std::string_view content_type;
        std::visit(
            [&](auto &&arg) {
                content_type = arg.count(http::field::content_type) ? arg[http::field::content_type] : "null";
            },
            response);
        return content_type;
    }

    void finalize(unsigned http_version, bool keep_alive) {
        std::visit([&](auto &&arg) { FinalizeResponse(arg, http_version, keep_alive); }, response);
    }

    template <typename Send>
    void send(Send &&send) {
        std::visit([&](auto &&arg) { send(arg); }, response);
    }

  private:
    template <typename Body>
    static void FinalizeResponse(http::response<Body> &response, unsigned http_version, bool keep_alive) {
        response.version(http_version);
        response.keep_alive(keep_alive);
    }

    std::variant<StringResponse, FileResponse> response;
};