#include "request_handler.hpp"

namespace request_handler {

// Создаёт StringResponse с заданными параметрами
StringResponse MakeJsonResponse(http::status status, std::string_view body) {
    StringResponse response;
    response.result(status);
    response.set(http::field::content_type, "application/json"sv);
    response.body() = body;
    response.content_length(body.size());
    return response;
}

void FinalizeJsonResponse(StringResponse &response, unsigned http_version, bool keep_alive) {
    response.version(http_version);
    response.keep_alive(keep_alive);
}

} // namespace request_handler
