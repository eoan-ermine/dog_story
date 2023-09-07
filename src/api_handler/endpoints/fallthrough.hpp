#pragma once

#include "api_handler/endpoints/endpoint.hpp"

class FallthroughEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override {
        return request.target().starts_with("/api/");
    }
    util::Response handle(const http::request<http::string_body> &request) override { return responses::ok(); }

  private:
    struct responses {
        static util::Response ok() {
            return util::Response::Json(http::status::bad_request,
                                        json::value_from(util::Error{"badRequest", "Bad request"}));
        }
    };
};