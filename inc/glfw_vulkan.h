#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>

// TODO: check that the compilers are actually inlining the code and that it actually makes
//  a difference in performance.
#if !defined(FORCE_INLINE)
    #define force_inline inline
#elif defined(_MSC_VER) && defined(FORCE_INLINE) // msvc
    #define force_inline __forceinline
#elif defined(__GNUG__) && defined(FORCE_INLINE) // gcc
    #define force_inline __attribute__((always_inline)) inline
#endif

