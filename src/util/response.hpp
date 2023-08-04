#include <boost/beast/http.hpp>

#include <type_traits>
#include <variant>

namespace beast = boost::beast;
namespace http = beast::http;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

template <typename Body>
void FinalizeResponse(http::response<Body> &response, unsigned http_version, bool keep_alive) {
    response.version(http_version);
    response.keep_alive(keep_alive);
}

class Response {
  public:
    Response() {}
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
        std::visit([&](auto &&arg) {
            code = arg.result_int();
        }, response);
        return code;
    }

    std::string_view content_type() const {
        std::string_view content_type;
        std::visit([&](auto &&arg) {
            content_type = arg.count(http::field::content_type) ? arg[http::field::content_type] : "null";
        }, response);
        return content_type;
    }

    void finalize(unsigned http_version, bool keep_alive) {
        std::visit([&](auto &&arg) {
            FinalizeResponse(arg, http_version, keep_alive);
        }, response);
    }

    template <typename Send>
    void send(Send &&send) {
        std::visit([&](auto &&arg) {
            send(arg);
        }, response);
    }

  private:
    std::variant<StringResponse, FileResponse> response;
};