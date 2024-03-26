#pragma once

#include "Common.h"

#include "Instance.h"

#include "Light.h"

#define MAX_LIGHTS 16
#define MAX_INSTANCES 128

class Camera;

class Scene
{
public:
	Scene(Camera *camera, SunLight &sunLight, glm::vec3 ambientLight);
	~Scene();

	void Update(float dt);
	void LateUpdate(float dt);
	void Draw();

	void AddInstance(Instance *instance);
	void RemoveInstance(Instance *instance);

	std::vector<Instance*> *GetInstances() { return &m_instances; }
	int GetNumInstances() { return (int)m_instances.size(); }

	void AddPointLight(PointLight light);
	void RemovePointLight(PointLight *light);
	void AddSpotLight(SpotLight light);
	void RemoveSpotLight(SpotLight *light);

	glm::vec3 &GetAmbientLight() { return m_ambientLight; }
	SunLight *GetSunLight() { return &m_sunLight; }

	std::vector<PointLight> *GetPointLights() { return &m_pointLights; }
	std::vector<SpotLight> *GetSpotLights() { return &m_spotLights; }

	unsigned int &GetPointLightBufferID() { return m_pointLightSBO; }
	unsigned int &GetSpotLightBufferID() { return m_spotLightSBO; }

	Camera *GetCamera() const { return m_currentCamera; }
	void SetCamera(Camera *camera) { m_currentCamera = camera; }

protected:
	void CheckInstanceDeletion();
	void CheckPointLightDeletion();
	void CheckSpotLightDeletion();

protected:
	Camera *m_sceneCamera;
	Camera *m_currentCamera;

	glm::vec3 m_ambientLight;
	SunLight &m_sunLight;

	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;

	std::vector<PointLight> m_pointLightsToDelete;
	std::vector<SpotLight> m_spotLightsToDelete;

	unsigned int m_pointLightSBO; // Storage buffer objects because I think having multiple arrays for each light parameter is gross.
	unsigned int m_spotLightSBO;

	std::vector<Instance*> m_instances;
	std::vector<Instance*> m_instancesToDelete;

};
