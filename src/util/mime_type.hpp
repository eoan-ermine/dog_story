#include <boost/beast/core.hpp>

#include <string_view>

namespace beast = boost::beast;

// Return a reasonable mime type based on the extension of a file.
std::string_view GetMimeType(std::string_view path);