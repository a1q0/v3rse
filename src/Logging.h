#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

//#define SPDLOG_USE_STD_FORMAT

#if defined(SPDLOG_USE_STD_FORMAT)
#    include <format>
#elif defined(SPDLOG_FMT_EXTERNAL)
#    include <fmt/locale.h>
#else
#    include "spdlog/fmt/bundled/format.h"
#endif
