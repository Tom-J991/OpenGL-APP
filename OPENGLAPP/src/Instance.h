#pragma once

#include "Common.h"

class Camera; // Forward Declare
class Mesh; // Forward Declare
class Scene; // Forward Declare

namespace aie
{
	class ShaderProgram; // Forward Declare
}

// Instance class to easily share meshes and shaders for different transforms.
class Instance
{
public:
	Instance(glm::mat4 transform, Mesh *mesh, aie::ShaderProgram *shader);
	~Instance();

	glm::vec3 &GetPosition() { return m_position; }
	glm::vec3 &GetRotation() { return m_eulerAngles; }
	glm::vec3 &GetScale() { return m_scale; }

	glm::mat4 MakeTransform(glm::vec3 position, glm::vec3 eulerAngles, glm::vec3 scale);
	glm::mat4 &GetTransform() { return m_transform; }

	void Draw(Scene *scene);

protected:
	glm::mat4 m_transform;
	glm::vec3 m_position = glm::vec3(0);
	glm::vec3 m_eulerAngles = glm::vec3(0);
	glm::vec3 m_scale = glm::vec3(1);

	Mesh *m_mesh;
	aie::ShaderProgram *m_shader;

};
