#pragma once

#include "Common.h"

// Light objects.
struct Light // Base Light.
{
	glm::vec4 position;
	glm::vec4 color;
	float intensity = 1.0f;
	float dummy[3] = {}; // For stupid memory alignment with GLSL storage buffer objects.

	Light() = default;
	Light(glm::vec3 position, glm::vec3 color, float intensity)
	{
		this->position = glm::vec4(position, 1.0f);
		this->color = glm::vec4(color, 1.0f);
		this->intensity = intensity;
	}
};
typedef Light PointLight; // Base light is also a point light.

struct SunLight : public Light
{
	glm::vec4 direction;

	SunLight() = default;
	SunLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float intensity) : Light(position, color, intensity)
	{
		this->direction = glm::vec4(direction, 1.0f);	
	}

};

struct SpotLight : public Light
{
	glm::vec4 direction;
	float innerCutoff;
	float outerCutoff;

	SpotLight() = default;
	SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float innerCutoff, float outerCutoff, float intensity) : Light(position, color, intensity)
	{
		this->direction = glm::vec4(direction, 1.0f);
		this->innerCutoff = innerCutoff;
		this->outerCutoff = outerCutoff;
	}
	
};
