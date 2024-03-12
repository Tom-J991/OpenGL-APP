#pragma once

#include <iostream>
#include <cstdio>

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _DEBUG
#define ASSERT(x, ...) if ((x)) { printf(__VA_ARGS__); __debugbreak(); }
#else
#define ASSERT(x, ...)
#endif
