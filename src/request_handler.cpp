#include "request_handler.hpp"

namespace request_handler {

// Создает text response
StringResponse MakeTextResponse(http::status status, std::string_view body) {
    StringResponse response;
    response.result(status);
    response.set(http::field::content_type, "text/plain"sv);
    response.body() = body;
    response.content_length(body.size());
    return response;
}

// Создаёт json response
StringResponse MakeJsonResponse(http::status status, std::string_view body) {
    StringResponse response;
    response.result(status);
    response.set(http::field::content_type, "application/json"sv);
    response.body() = body;
    response.content_length(body.size());
    return response;
}

// Создает file response
FileResponse MakeFileResponse(http::status status, std::string_view mime_type, std::string_view filepath,
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

    return response;
}

} // namespace request_handler
