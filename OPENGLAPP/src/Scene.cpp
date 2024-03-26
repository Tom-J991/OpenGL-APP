#include "Scene.h"

#include "Light.h"

#include <iostream>

#include <glad.h>

// TODO: Scene Management, bounding volumes, frustrum culling, quadtrees/octrees/bsp.

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
	for (auto it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		delete *it;
	}
}

void Scene::Update(float dt)
{
}

void Scene::LateUpdate(float dt)
{
	// Delete instances marked as delete after all updates to avoid any errors.
	CheckInstanceDeletion();

	// Do the same for the lights.
	CheckPointLightDeletion();
	CheckSpotLightDeletion();
}

void Scene::Draw()
{
	// Draw everything in the scene.
	for (auto it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		Instance *instance = *it;
		instance->Draw(this);
	}
}

void Scene::AddInstance(Instance *instance)
{
	if (m_instances.size() >= MAX_INSTANCES)
	{
		std::cout << "WARNING: " << "Already hit maximum amount of instances in scene! Cannot add any more." << std::endl;
		return;
	}

	m_instances.push_back(instance);
}

void Scene::RemoveInstance(Instance *instance)
{
	for (auto it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if (instance == *it)
		{
			m_instancesToDelete.push_back(*it); // Mark instance as deleted.
		}
	}
}

void Scene::AddPointLight(PointLight light)
{
	if (m_pointLights.size() >= MAX_LIGHTS)
	{
		std::cout << "WARNING: " << "Already hit maximum amount of point lights in scene! Cannot add any more." << std::endl;
		return;
	}

	m_pointLights.push_back(light);
}

void Scene::RemovePointLight(PointLight *light)
{
	for (auto it = m_pointLights.begin(); it != m_pointLights.end(); ++it)
	{
		if (light == &(*it))
		{
			m_pointLightsToDelete.push_back(*it);
		}
	}
}

void Scene::AddSpotLight(SpotLight light)
{
	if (m_spotLights.size() >= MAX_LIGHTS)
	{
		std::cout << "WARNING: " << "Already hit maximum amount of spotlights in scene! Cannot add any more." << std::endl;
		return;
	}

	m_spotLights.push_back(light);
}

void Scene::RemoveSpotLight(SpotLight *light)
{
	for (auto it = m_spotLights.begin(); it != m_spotLights.end(); ++it)
	{
		if (light == &(*it))
		{
			m_spotLightsToDelete.push_back(*it);
		}
	}
}

void Scene::CheckInstanceDeletion()
{
	if (m_instancesToDelete.size() <= 0) // Avoid checking if there isn't anything to delete.
		return;

	// Probably safe to assume only one instance will be marked as delete at a time lol, though that probably wouldn't be true for a more complex game probably
	Instance *instance = m_instancesToDelete[0];
	if (instance == nullptr)
		return;

	for (auto it = m_instances.rbegin(), e = m_instances.rend(); it != e; ++it) // Iterate reverse order because it works.
	{
		if (*it == instance)
		{
			auto i = std::remove(m_instances.begin(), m_instances.end(), instance);
			m_instances.erase(i, m_instances.end());
			auto j = std::remove(m_instancesToDelete.begin(), m_instancesToDelete.end(), instance);
			m_instancesToDelete.erase(j, m_instancesToDelete.end());
			delete instance;
			break;
		}
	}
}

void Scene::CheckPointLightDeletion()
{
	if (m_pointLightsToDelete.size() <= 0) // Avoid checking if there isn't anything to delete.
		return;

	PointLight light = m_pointLightsToDelete[0]; // TODO: Would be better to rework it to store point lights as pointers.
	for (auto it = m_pointLights.rbegin(), e = m_pointLights.rend(); it != e; ++it) // Iterate reverse order because it works.
	{
		if (*it == light)
		{
			auto i = std::remove(m_pointLights.begin(), m_pointLights.end(), light);
			m_pointLights.erase(i, m_pointLights.end());
			auto j = std::remove(m_pointLightsToDelete.begin(), m_pointLightsToDelete.end(), light);
			m_pointLightsToDelete.erase(j, m_pointLightsToDelete.end());
			break;
		}
	}
}

void Scene::CheckSpotLightDeletion()
{
	if (m_spotLightsToDelete.size() <= 0) // Avoid checking if there isn't anything to delete.
		return;

	SpotLight light = m_spotLightsToDelete[0]; // TODO: Would be better to rework it to store spotlights as pointers.
	for (auto it = m_spotLights.rbegin(), e = m_spotLights.rend(); it != e; ++it) // Iterate reverse order because it works.
	{
		if (*it == light)
		{
			auto i = std::remove(m_spotLights.begin(), m_spotLights.end(), light);
			m_spotLights.erase(i, m_spotLights.end());
			auto j = std::remove(m_spotLightsToDelete.begin(), m_spotLightsToDelete.end(), light);
			m_spotLightsToDelete.erase(j, m_spotLightsToDelete.end());
			break;
		}
	}
}
