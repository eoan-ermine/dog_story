#include <boost/beast/http.hpp>

#include <type_traits>
#include <variant>

namespace beast = boost::beast;
namespace http = beast::http;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

#define FOR_RESPONSE(response, action)                                                                                 \
    {                                                                                                                  \
        std::visit(                                                                                                    \
            [&](auto &&arg) {                                                                                          \
                using T = std::decay_t<decltype(arg)>;                                                                 \
                if constexpr (std::is_same_v<T, StringResponse>) {                                                     \
                    action                                                                                             \
                } else if constexpr (std::is_same_v<T, FileResponse>) {                                                \
                    action                                                                                             \
                }                                                                                                      \
            },                                                                                                         \
            response);                                                                                                 \
    }                                                                                                                  \
    while (0)

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
        FOR_RESPONSE(response, { code = arg.result_int(); });
        return code;
    }

    std::string_view content_type() const {
        std::string_view content_type;
        FOR_RESPONSE(response, {
            content_type = arg.count(http::field::content_type) ? arg[http::field::content_type] : "null";
        });
        return content_type;
    }

    void finalize(unsigned http_version, bool keep_alive) {
        FOR_RESPONSE(response, { FinalizeResponse(arg, http_version, keep_alive); });
    }

    template <typename Send>
    void send(Send &&send) {
        FOR_RESPONSE(response, { send(arg); });
    }

  private:
    std::variant<StringResponse, FileResponse> response;
};