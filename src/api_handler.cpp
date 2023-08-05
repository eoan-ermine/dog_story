#include "api_handler.hpp"

#include <boost/json.hpp>

#include "util/error.hpp"
#include "util/response.hpp"

namespace api_handler {

bool APIHandler::dispatch(Response &response, std::string_view target) const {
    if (target == endpoint) {
        response = get_maps();
    } else if (target.starts_with(endpoint) && !target.ends_with("/")) {
        std::string_view id = target.substr(endpoint.size() + 1);
        response = get_map(model::Map::Id{std::string{id}});
    } else if (target.starts_with("/api/")) {
        response = fallthrough();
    } else {
        return false;
    }

    return true;
}

Response APIHandler::get_maps() const { return Response::Json(http::status::ok, json::value_from(game_.GetMaps())); }

Response APIHandler::get_map(model::Map::Id id) const {
    const auto *map_ptr = game_.FindMap(id);

    if (map_ptr == nullptr)
        return Response::Json(http::status::not_found, json::value_from(util::Error{"mapNotFound", "Map not found"}));
    else
        return Response::Json(http::status::ok, json::value_from(*map_ptr));
}

Response APIHandler::fallthrough() const {
    return Response::Json(http::status::bad_request, json::value_from(util::Error{"badRequest", "Bad request"}));
}

} // namespace api_handler
