#include "Scene.h"

#include "Light.h"

#include <glad.h>

Scene::Scene(Camera *camera, SunLight &sunLight, glm::vec3 ambientLight)
	: m_sceneCamera(camera)
	, m_sunLight(sunLight)
	, m_ambientLight(ambientLight)
{ 
	// Setup storage buffer objects.
	glGenBuffers(1, &m_pointLightSBO); // Point lights.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pointLightSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * MAX_LIGHTS, m_pointLights.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_spotLightSBO); // Spot lights.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_spotLightSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SpotLight) * MAX_LIGHTS, m_spotLights.data(), GL_DYNAMIC_DRAW);
}
Scene::~Scene()
{ 
	// Clean up everything in scene.
	for (auto it = m_instances.begin(); it != m_instances.end(); it++)
	{
		delete *it;
	}
}

void Scene::Draw()
{
	// Draw everything in the scene.
	for (auto it = m_instances.begin(); it != m_instances.end(); it++)
	{
		Instance *instance = *it;
		instance->Draw(this);
	}
}

void Scene::AddInstance(Instance *instance)
{
	m_instances.push_back(instance);
}

void Scene::AddPointLight(Light light)
{
	m_pointLights.push_back(light);
}

void Scene::AddSpotLight(SpotLight light)
{
	m_spotLights.push_back(light);
}
