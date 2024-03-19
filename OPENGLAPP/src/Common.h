#pragma once

#include <iostream>
#include <cstdio>

#include <string>
#include <sstream>

#include <vector>
#include <list>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Custom assert, shouldn't build on release.
#ifdef _DEBUG
#define ASSERT(x, ...) if ((x)) { printf(__VA_ARGS__); __debugbreak(); }
#else
#define ASSERT(x, ...)
#endif
