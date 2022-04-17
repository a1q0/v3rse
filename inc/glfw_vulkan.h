#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// TODO: check that the compilers are actually inlining the code and that it actually makes
//  a difference in performance.
#ifdef _MSC_VER // msvsc
    #define _inline __forceinline
#else
    #ifdef __GNUG__ // gcc
        #define _inline __attribute__((always_inline))
    #else
        #define _inline inline
    #endif
#endif