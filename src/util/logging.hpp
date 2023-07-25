#pragma once

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <string_view>

void LogFormatter(const boost::log::record_view &rec, boost::log::formatting_ostream &stream);

void LogStart(std::string_view address, unsigned int port);
void LogExit(int code);
void LogExit(int code, std::string_view exception);
void LogRequest(std::string_view address, std::string_view uri, std::string_view method);
void LogResponse(int response_time, int code, std::string_view content_type);
void LogError(int code, std::string_view text, std::string_view where);