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

    void finalize(unsigned http_version, bool keep_alive) {
        std::visit(
            [&](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, StringResponse>) {
                    FinalizeResponse(arg, http_version, keep_alive);
                } else if constexpr (std::is_same_v<T, FileResponse>) {
                    FinalizeResponse(arg, http_version, keep_alive);
                }
            },
            response);
    }

    template <typename Send>
    void send(Send &&send) {
        std::visit(
            [&](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, StringResponse>) {
                    send(arg);
                } else if constexpr (std::is_same_v<T, FileResponse>) {
                    send(arg);
                }
            },
            response);
    }

  private:
    std::variant<StringResponse, FileResponse> response;
};