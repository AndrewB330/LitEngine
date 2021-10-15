#pragma once
#ifndef LIT_ASSERTS
#define LIT_ASSERTS true

#include <sstream>
#include <spdlog/spdlog.h>
#define GET_LOGGER_(logger, ...) logger
#define GET_LOGGER(...) GET_LOGGER_(__VA_ARGS__ __VA_OPT__(,) spdlog::default_logger())

#define LIT_ASSERT(exp, msg, ...) \
{ if (!(exp)) { (GET_LOGGER(__VA_ARGS__))->critical("[assert] {} {}:{}", (msg), __FILE__, __LINE__);\
std::stringstream stream{}; \
stream << msg << " " << __FILE__ << ":" << __LINE__; \
throw std::runtime_error(stream.str());}}

#define LIT_ASSERT_NOTHROW(exp, msg, ...) \
{ if (!(exp)) { (GET_LOGGER(__VA_ARGS__))->critical("[assert] {} {}:{}", (msg), __FILE__, __LINE__);}}

#define LIT_ENSURE(constraint, ...) \
{ if (!(constraint)) { (GET_LOGGER(__VA_ARGS__))->warn("[constraint violated] \"{}\" is false {}:{}", #constraint, __FILE__, __LINE__); }}

#define LIT_ENSURE_EQ(expr, val, ...) \
{ auto expr_res = (expr); auto val_res = (val);\
if (expr_res != val_res) {               \
    (GET_LOGGER(__VA_ARGS__))->warn("[constraint violated] {} != {} ({} != {}) {}:{}", #expr, #val, expr_res, val, __FILE__, __LINE__); \
}}

#define LIT_ENSURE_BETWEEN(expr, val_low, val_high, ...) \
{ auto expr_res = (expr); auto val_low_res = (val_low); auto val_high_res = (val_high);\
if (expr_res < val_low_res) \
{ (GET_LOGGER(__VA_ARGS__))->warn("[constraint violated] {} < {} ({} < {}) {}:{}", #expr, #val_low, expr_res, val_low_res, __FILE__, __LINE__);}\
if (expr_res > val_high_res) \
{ (GET_LOGGER(__VA_ARGS__))->warn("[constraint violated] {} > {} ({} > {}) {}:{}", #expr, #val_high, expr_res, val_high_res, __FILE__, __LINE__);}}

#endif