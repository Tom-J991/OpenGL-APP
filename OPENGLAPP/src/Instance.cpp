#include "Instance.h"

#include "Application.h"

#include "Scene.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Light.h"

#include <glad.h>

Instance::Instance(glm::mat4 transform, Mesh *mesh, aie::ShaderProgram *shader)
	: m_transform(transform)
	, m_mesh(mesh)
	, m_shader(shader)
{ }
Instance::~Instance()
{ }

glm::mat4 Instance::MakeTransform(glm::vec3 position, glm::vec3 eulerAngles, glm::vec3 scale)
{
	// Calculate transformation matrix with the given information.
	return glm::translate(glm::mat4(1.0f), position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(eulerAngles.z), glm::vec3(0, 0, 1)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(eulerAngles.y), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(eulerAngles.x), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), scale);
}

void Instance::Draw(Scene *scene)
{
	float windowWidth = (float)Application::Instance()->GetWindowWidth();
	float windowHeight = (float)Application::Instance()->GetWindowHeight();

	Camera *camera = scene->GetCamera();
	SunLight *sunLight = scene->GetSunLight();
	glm::vec3 ambientLight = scene->GetAmbientLight();

	glm::mat4 mvp = camera->GetProjectionMatrix(90.0f, windowWidth, windowHeight) * camera->GetViewMatrix() * m_transform;

	// Setup shaders and materials then draw mesh.
	m_shader->bind();
	m_shader->bindUniform("cameraPosition", camera->GetPosition());

	m_shader->bindUniform("mvp", mvp);
	m_shader->bindUniform("view", camera->GetViewMatrix());
	m_shader->bindUniform("model", m_transform);

	m_shader->bindUniform("sunlightDir", glm::vec3(sunLight->direction));
	m_shader->bindUniform("sunlightColor", glm::vec3(sunLight->color));
	m_shader->bindUniform("ambientColor", ambientLight);

	m_shader->bindUniform("numPointLights", scene->GetNumPointLights());
	m_shader->bindUniform("numSpotLights", scene->GetNumSpotLights());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scene->GetPointLightBufferID());
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PointLight) * scene->GetNumPointLights(), scene->GetPointLights()->data()); // Pass scene point lights to the storage buffer object.

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene->GetSpotLightBufferID());
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpotLight) * scene->GetNumSpotLights(), scene->GetSpotLights()->data()); // Pass scene spotlights to the storage buffer object.

	m_mesh->ApplyMaterial(m_shader);
	m_mesh->Draw();
}
