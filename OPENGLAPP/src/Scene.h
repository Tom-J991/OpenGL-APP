#pragma once

#include "Common.h"

#include "Instance.h"

#include "Light.h"

#define MAX_LIGHTS 4

class Camera;

class Scene
{
public:
	Scene(Camera *camera, SunLight &sunLight, glm::vec3 ambientLight);
	~Scene();

	void Draw();

	void AddInstance(Instance *instance);

	void AddPointLight(PointLight light);
	void AddSpotLight(SpotLight light);

	glm::vec3 &GetAmbientLight() { return m_ambientLight; }
	SunLight *GetSunLight() { return &m_sunLight; }

	std::vector<PointLight> *GetPointLights() { return &m_pointLights; }
	int GetNumPointLights() const { return (int)m_pointLights.size(); }

	std::vector<SpotLight> *GetSpotLights() { return &m_spotLights; }
	int GetNumSpotLights() const { return (int)m_spotLights.size(); }

	unsigned int &GetPointLightBufferID() { return m_pointLightSBO; }
	unsigned int &GetSpotLightBufferID() { return m_spotLightSBO; }

	Camera *GetCamera() const { return m_currentCamera; }
	void SetCamera(Camera *camera) { m_currentCamera = camera; }

protected:
	Camera *m_sceneCamera;
	Camera *m_currentCamera;

	glm::vec3 m_ambientLight;
	SunLight &m_sunLight;

	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;

	unsigned int m_pointLightSBO; // Storage buffer objects because I think having multiple arrays for each light parameter is gross.
	unsigned int m_spotLightSBO;

	std::list<Instance*> m_instances;

};
