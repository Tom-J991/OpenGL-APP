#pragma once

#include "Common.h"

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 color;
	float size;
	float lifetime;
	float lifespan;
};

struct ParticleVertex
{
	glm::vec4 position;
	glm::vec4 color;
};

// TODO: Could probably make this a subclass of Instance.
class ParticleEmitter
{
public:
	ParticleEmitter();
	virtual ~ParticleEmitter();

	void Initialise(unsigned int maxParticles,
					unsigned int emitRate,
					float lifetimeMin, float lifetimeMax,
					float velocityMin, float velocityMax,
					float startSize, float endSize,
					const glm::vec4 &startColor, const glm::vec4 &endColor);

	void Emit();
	void Update(float dt, const glm::mat4 &cameraTransform);
	void Draw();

protected:
	Particle *m_particles;
	unsigned int m_firstDead;
	unsigned int m_maxParticles;

	glm::vec3 m_position;
	glm::vec4 m_startColor;
	glm::vec4 m_endColor;
	float m_emitTimer;
	float m_emitRate;
	float m_lifespanMin;
	float m_lifespanMax;
	float m_velocityMin;
	float m_velocityMax;
	float m_startSize;
	float m_endSize;

	unsigned int m_vao, m_vbo, m_ebo;
	ParticleVertex *m_vertexData;

};
