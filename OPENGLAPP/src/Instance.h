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

	glm::mat4 MakeTransform(glm::vec3 position, glm::vec3 eulerAngles, glm::vec3 scale);

	void Draw(Scene *scene);

protected:
	glm::mat4 m_transform;
	Mesh *m_mesh;
	aie::ShaderProgram *m_shader;

};
