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
{ 
	glm::vec3 skew; // junk
	glm::vec4 perspective; // junk

	glm::quat orientation;
	glm::decompose(transform, m_scale, orientation, m_position, skew, perspective);
	m_eulerAngles = glm::eulerAngles(orientation);
}
Instance::~Instance()
{ }

glm::mat4 Instance::MakeTransform(glm::vec3 position, glm::vec3 eulerAngles, glm::vec3 scale)
{
	m_position = position;
	m_eulerAngles = eulerAngles;
	m_scale = scale;

	// Calculate transformation matrix with the given information.
	return glm::translate(glm::mat4(1.0f), position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(eulerAngles.z), glm::vec3(0, 0, 1)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(eulerAngles.y), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(eulerAngles.x), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), scale);
}

void Instance::Draw(Scene *scene)
{
	float windowWidth = (float)Application::GetInstance()->GetWindowWidth();
	float windowHeight = (float)Application::GetInstance()->GetWindowHeight();

	Camera *camera = scene->GetCamera();
	SunLight *sunLight = scene->GetSunLight();
	glm::vec3 ambientLight = scene->GetAmbientLight();

	m_transform = MakeTransform(m_position, m_eulerAngles, m_scale);
	glm::mat4 mvp = camera->GetProjectionMatrix(90.0f, windowWidth, windowHeight) * camera->GetViewMatrixFromQuaternion() * m_transform;

	// Setup shaders and materials then draw mesh.
	m_shader->bind();
	m_shader->bindUniform("cameraPosition", camera->GetPosition());

	m_shader->bindUniform("mvp", mvp);
	m_shader->bindUniform("view", camera->GetTransformMatrix());
	m_shader->bindUniform("model", m_transform);

	m_shader->bindUniform("sunlightDir", glm::vec3(sunLight->direction));
	m_shader->bindUniform("sunlightColor", glm::vec3(sunLight->color));
	m_shader->bindUniform("ambientColor", ambientLight);

	auto pointLights = scene->GetPointLights();
	auto spotLights = scene->GetSpotLights();

	m_shader->bindUniform("numPointLights", (int)pointLights->size());
	m_shader->bindUniform("numSpotLights", (int)spotLights->size());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scene->GetPointLightBufferID());
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PointLight) * pointLights->size(), pointLights->data()); // Pass scene point lights to the storage buffer object.

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene->GetSpotLightBufferID());
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpotLight) * spotLights->size(), spotLights->data()); // Pass scene spotlights to the storage buffer object.

	m_mesh->ApplyMaterial(m_shader);
	m_mesh->Draw();
}
