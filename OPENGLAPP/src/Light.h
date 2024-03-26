#pragma once

#include "Common.h"

// Light objects.
struct Light // Base Light.
{
	glm::vec4 position; // vec4s seem easier to manage than vec3s with the memory alignment.
	glm::vec4 color;
	float intensity = 1.0f;
	float dummy[3] = { }; // For stupid memory alignment with GLSL storage buffer objects (std430).
	// I'm aware of alignas() & #pragma pack(n), but I'm not really sure how to use them or how helpful they would be & this just seems way easier.

	Light() = default;
	Light(glm::vec3 position, glm::vec3 color, float intensity)
	{
		this->position = glm::vec4(position, 1.0f);
		this->color = glm::vec4(color, 1.0f);
		this->intensity = intensity;
	}

	bool operator==(const Light &other) 
	{
		if (this->color == other.color && 
			this->position == other.position && 
			this->intensity == other.intensity)
			return true;
		return false;
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

	bool operator==(const SunLight &other)
	{
		if (this->color == other.color &&
			this->position == other.position &&
			this->intensity == other.intensity &&
			this->direction == other.direction)
			return true;
		return false;
	}
};

struct SpotLight : public Light
{
	glm::vec4 direction;
	float innerCutoff;
	float outerCutoff;
	float dummy[2] = { }; // For stupid memory alignment with GLSL storage buffer objects (std430).

	SpotLight() = default;
	SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float innerCutoff, float outerCutoff, float intensity) : Light(position, color, intensity)
	{
		this->direction = glm::vec4(direction, 1.0f);
		this->innerCutoff = innerCutoff;
		this->outerCutoff = outerCutoff;
	}
	
	bool operator==(const SpotLight &other)
	{
		if (this->color == other.color &&
			this->position == other.position &&
			this->intensity == other.intensity &&
			this->direction == other.direction &&
			this->innerCutoff == other.innerCutoff &&
			this->outerCutoff == other.outerCutoff)
			return true;
		return false;
	}
};
