#pragma once

#include "Common.h"

#include "Texture.h"

namespace aie
{
	class ShaderProgram;
}

class Mesh
{
public:
	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 normal;
		glm::vec2 texCoord;
		glm::vec4 tangent;
	};

public:
	Mesh();
	virtual ~Mesh();

	void InitializeQuad();
	void Initialize(unsigned int vertexCount, const Vertex *vertices, unsigned int indexCount = 0, unsigned int *indices = nullptr);
	void InitializeFromFile(const char *filePath);

	void LoadMaterial(const char *filePath);
	void ApplyMaterial(aie::ShaderProgram *shader);

	virtual void Draw();

private:
	void CalculateTangents(Vertex *vertices, unsigned int vertexCount, const std::vector<unsigned int> &indices);

protected:
	unsigned int m_triCount = 0;
	unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;

	float specular = 1.0f;
	glm::vec3 Ka = { 1.0f, 1.0f, 1.0f };
	glm::vec3 Kd = { 1.0f, 1.0f, 1.0f };
	glm::vec3 Ks = { 1.0f, 1.0f, 1.0f };

	aie::Texture mapKd;
	aie::Texture mapKs;
	aie::Texture mapBump;

};
