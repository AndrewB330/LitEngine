#pragma once

#ifdef _MSC_VER
#define LIT_COMPILER "MSVC"
#elif defined(__MINGW32__)
#define LIT_COMPILER "MinGW"
#else
#define LIT_COMPILER "Unknown"
#endif

#if INTPTR_MAX == INT64_MAX
#define LIT_ARCHITECTURE "x64"
#elif INTPTR_MAX == INT32_MAX
#define LIT_ARCHITECTURE "x86"
#endif

#ifndef NDEBUG
#define LIT_DEBUG (true)
#define LIT_CONFIG "Debug"
#else
#define LIT_DEBUG (false)
#define LIT_CONFIG "Release"
#endif