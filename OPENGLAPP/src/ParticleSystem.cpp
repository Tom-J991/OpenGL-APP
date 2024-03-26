#include "ParticleSystem.h"

#include <glad.h>

// TODO: GPU-Based particle system using geometry shaders.

ParticleEmitter::ParticleEmitter()
	: m_particles(nullptr)
	, m_firstDead(0)
	, m_maxParticles(0)
	, m_position(0, 0, 0)
	, m_vao(0), m_vbo(0), m_ebo(0)
	, m_vertexData(nullptr)
{ }

ParticleEmitter::~ParticleEmitter()
{
	// Clean up.
	delete[] m_particles;
	delete[] m_vertexData;

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ebo);
}

void ParticleEmitter::Initialise(unsigned int maxParticles, 
								 unsigned int emitRate, 
								 float lifetimeMin, float lifetimeMax, 
								 float velocityMin, float velocityMax, 
								 float startSize, float endSize, 
								 const glm::vec4 &startColor, const glm::vec4 &endColor)
{
	// Setup variables.
	m_emitTimer = 0.0f;
	m_emitRate = 1.0f / emitRate;

	m_startColor = startColor;
	m_endColor = endColor;
	m_startSize = startSize;
	m_endSize = endSize;
	m_velocityMin = velocityMin;
	m_velocityMax = velocityMax;
	m_lifespanMin = lifetimeMin;
	m_lifespanMax = lifetimeMax;
	m_maxParticles = maxParticles;

	// Create list of particles.
	m_particles = new Particle[m_maxParticles];
	m_firstDead = 0;

	// Create render data.
	m_vertexData = new ParticleVertex[m_maxParticles * 4];
	unsigned int *indexData = new unsigned int[m_maxParticles * 6];

	// Indices for all particles.
	for (unsigned int i = 0; i < m_maxParticles; ++i)
	{
		indexData[i * 6 + 0] = i * 4 + 0; // tri 1
		indexData[i * 6 + 1] = i * 4 + 1;
		indexData[i * 6 + 2] = i * 4 + 2;
		indexData[i * 6 + 3] = i * 4 + 0; // tri 2
		indexData[i * 6 + 4] = i * 4 + 2;
		indexData[i * 6 + 5] = i * 4 + 3;
	}

	// Generate OpenGL objects
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(ParticleVertex), m_vertexData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxParticles * 6 * sizeof(unsigned int), indexData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)0); // position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)16); // color attribute
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	delete[] indexData;
}

void ParticleEmitter::Emit()
{
	if (m_firstDead >= m_maxParticles)
		return;

	// Spawn new particle.
	Particle &particle = m_particles[m_firstDead++];
	particle.position = m_position;
	particle.lifetime = 0;
	particle.lifespan = (rand() / (float)RAND_MAX) * (m_lifespanMax - m_lifespanMin) + m_lifespanMin;
	particle.color = m_startColor;
	particle.size = m_startSize;

	// random velocity.
	float velocity = (rand() / (float)RAND_MAX) * (m_velocityMax - m_velocityMin) + m_velocityMin;
	particle.velocity.x = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity.y = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity.z = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity = glm::normalize(particle.velocity);
}

void ParticleEmitter::Update(float dt, const glm::mat4 &cameraTransform)
{
	m_emitTimer += dt;
	while (m_emitTimer > m_emitRate)
	{
		Emit();
		m_emitTimer -= m_emitRate;
	}

	// Update particles.
	unsigned int quad = 0;
	for (unsigned int i = 0; i < m_firstDead; ++i)
	{
		Particle *particle = &m_particles[i];

		particle->lifetime += dt;
		if (particle->lifetime >= particle->lifespan)
		{
			*particle = m_particles[m_firstDead - 1]; // Recycle particle.
			m_firstDead--;
		}
		else
		{
			// Update particle.
			particle->position += particle->velocity * dt;
			particle->size = glm::mix(m_startSize, m_endSize, particle->lifetime / particle->lifespan);
			particle->color = glm::mix(m_startColor, m_endColor, particle->lifetime / particle->lifespan);

			// Update particle vertices to match new size.
			float halfSize = particle->size * 0.5f;
			m_vertexData[quad * 4 + 0].position = glm::vec4(halfSize, halfSize, 0.0f, 1.0f);
			m_vertexData[quad * 4 + 0].color = particle->color;
			m_vertexData[quad * 4 + 1].position = glm::vec4(-halfSize, halfSize, 0.0f, 1.0f);
			m_vertexData[quad * 4 + 1].color = particle->color;
			m_vertexData[quad * 4 + 2].position = glm::vec4(-halfSize, -halfSize, 0.0f, 1.0f);
			m_vertexData[quad * 4 + 2].color = particle->color;
			m_vertexData[quad * 4 + 3].position = glm::vec4(halfSize, -halfSize, 0.0f, 1.0f);
			m_vertexData[quad * 4 + 3].color = particle->color;

			// TODO: fix the billboarding.
			glm::vec3 toCamera = glm::normalize(glm::vec3(cameraTransform[3]) - particle->position);
			glm::vec3 up = -glm::cross(glm::vec3(cameraTransform[1]), toCamera);
			glm::vec3 right = glm::cross(toCamera, up);

			glm::mat4 billboard = glm::mat4(glm::vec4(up, 0.0f), glm::vec4(right, 0.0f), glm::vec4(toCamera, 0.0f), glm::vec4(0, 0, 0, 1));
			m_vertexData[quad * 4 + 0].position = billboard * m_vertexData[quad * 4 + 0].position + glm::vec4(particle->position, 0.0f);
			m_vertexData[quad * 4 + 1].position = billboard * m_vertexData[quad * 4 + 1].position + glm::vec4(particle->position, 0.0f);
			m_vertexData[quad * 4 + 2].position = billboard * m_vertexData[quad * 4 + 2].position + glm::vec4(particle->position, 0.0f);
			m_vertexData[quad * 4 + 3].position = billboard * m_vertexData[quad * 4 + 3].position + glm::vec4(particle->position, 0.0f);

			quad++;
		}
	}
}

void ParticleEmitter::Draw()
{
	// Bind OpenGL objects and draw.
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_firstDead * 4 * sizeof(ParticleVertex), m_vertexData); // Update particle vertices.
	glDrawElements(GL_TRIANGLES, m_firstDead * 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
