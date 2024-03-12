#include "Mesh.h"

#include "Shader.h"

#include <string>
#include <sstream>
#include <fstream>

#include <glad.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>

Mesh::Mesh()
{ }
Mesh::~Mesh()
{
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::Initialize(unsigned int vertexCount, const Vertex *vertices, unsigned int indexCount, unsigned int *indices)
{
	ASSERT(m_VAO != 0, "VAO already initialized.");

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)16);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)32);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)40);
	glEnableVertexAttribArray(3);

	if (indexCount != 0)
	{
		ASSERT(indices == nullptr, "No indices have been passed in.");
		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
		m_triCount = indexCount / 3;
	}
	else
	{
		m_triCount = vertexCount / 3;
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::InitializeFromFile(const char *filePath)
{
	const aiScene *scene = aiImportFile(filePath, 0);
	aiMesh *mesh = scene->mMeshes[0];

	int numFaces = mesh->mNumFaces;
	std::vector<unsigned int> indices;
	for (int i = 0; i < numFaces; i++)
	{
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		if (mesh->mFaces[i].mNumIndices == 4)
		{
			indices.push_back(mesh->mFaces[i].mIndices[0]);
			indices.push_back(mesh->mFaces[i].mIndices[3]);
			indices.push_back(mesh->mFaces[i].mIndices[2]);
		}
	}

	int numV = mesh->mNumVertices;
	Vertex *vertices = new Vertex[numV];
	for (int i = 0; i < numV; i++)
	{
		vertices[i].position = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
		vertices[i].normal = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);

		if (mesh->mTextureCoords[0])
		{
			vertices[i].texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertices[i].texCoord = glm::vec2(0.0f);
		}

		if (mesh->HasTangentsAndBitangents())
		{
			vertices[i].tangent = glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f);
		}
	}

	if (!mesh->HasTangentsAndBitangents())
		CalculateTangents(vertices, numV, indices);

	Initialize(numV, vertices, (unsigned int)indices.size(), indices.data());

	delete[] vertices; vertices = nullptr;
}

void Mesh::LoadMaterial(const char *filePath)
{
	std::fstream file(filePath, std::ios::in);
	std::string line;
	std::string header;
	char buffer[256];

	std::string directory(filePath);
	int index = (int)directory.rfind('/');
	if (index != -1)
		directory = directory.substr(0, index + 1);

	while (!file.eof())
	{
		file.getline(buffer, 256);
		line = buffer;
		std::stringstream ss(line, std::stringstream::in | std::stringstream::out);
		if (line.find("Ka") == 0)
			ss >> header >> Ka.x >> Ka.y >> Ka.z;
		else if (line.find("Ks") == 0)
			ss >> header >> Ks.x >> Ks.y >> Ks.z;
		else if (line.find("Kd") == 0)
			ss >> header >> Kd.x >> Kd.y >> Kd.z;
		else if (line.find("Ns") == 0)
			ss >> header >> specular;
		else if (line.find("map_Kd") == 0)
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			mapKd.load((directory + mapFileName).c_str());
		}
		else if (line.find("map_Ks") == 0)
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			mapKs.load((directory + mapFileName).c_str());
		}
		else if (line.find("bump") == 0)
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			mapBump.load((directory + mapFileName).c_str());
		}
	}
}

void Mesh::ApplyMaterial(aie::ShaderProgram *shader)
{
	shader->bindUniform("specular", specular);
	shader->bindUniform("Ka", Ka);
	shader->bindUniform("Kd", Kd);
	shader->bindUniform("Ks", Ks);
	mapKd.bind(0);
	shader->bindUniform("diffuseTex", 0);
	mapKs.bind(1);
	shader->bindUniform("specularTex", 1);
	mapBump.bind(2);
	shader->bindUniform("normalTex", 2);
}

void Mesh::InitializeQuad()
{
	ASSERT(m_VAO != 0, "VAO not initialized.");

	m_triCount = 2;
	Vertex vertices[6];
	vertices[0].position = { -0.5f, 0.0f,  0.5f, 1.0f };
	vertices[1].position = { 0.5f, 0.0f,  0.5f, 1.0f };
	vertices[2].position = { -0.5f, 0.0f, -0.5f, 1.0f };

	vertices[3].position = { -0.5f, 0.0f, -0.5f, 1.0f };
	vertices[4].position = { 0.5f, 0.0f,  0.5f, 1.0f };
	vertices[5].position = { 0.5f, 0.0f, -0.5f, 1.0f };

	vertices[0].normal = { 0, 1, 0, 0 };
	vertices[1].normal = { 0, 1, 0, 0 };
	vertices[2].normal = { 0, 1, 0, 0 };
	vertices[3].normal = { 0, 1, 0, 0 };
	vertices[4].normal = { 0, 1, 0, 0 };
	vertices[5].normal = { 0, 1, 0, 0 };

	vertices[0].texCoord = { 0.0f, 1.0f };
	vertices[1].texCoord = { 1.0f, 1.0f };
	vertices[2].texCoord = { 0.0f, 0.0f };
	vertices[3].texCoord = { 0.0f, 0.0f };
	vertices[4].texCoord = { 1.0f, 1.0f };
	vertices[5].texCoord = { 1.0f, 0.0f };

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	//glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)16);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)32);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)40);
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::Draw()
{
	glBindVertexArray(m_VAO);
	if (m_EBO != 0)
	{
		glDrawElements(GL_TRIANGLES, 3 * m_triCount, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, 3 * m_triCount);
	}
}

void Mesh::CalculateTangents(Vertex *vertices, unsigned int vertexCount, const std::vector<unsigned int> &indices)
{
	glm::vec4 *tan1 = new glm::vec4[vertexCount * 2];
	glm::vec4 *tan2 = tan1 + vertexCount;
	memset(tan1, 0, vertexCount * sizeof(glm::vec4) * 2);

	unsigned int indexCount = (unsigned int)indices.size();
	for (unsigned int a = 0; a < indexCount; a += 3)
	{
		long i1 = indices[a];
		long i2 = indices[a + 1];
		long i3 = indices[a + 2];

		const glm::vec4 &v1 = vertices[i1].position;
		const glm::vec4 &v2 = vertices[i2].position;
		const glm::vec4 &v3 = vertices[i3].position;

		const glm::vec2 &w1 = vertices[i1].texCoord;
		const glm::vec2 &w2 = vertices[i2].texCoord;
		const glm::vec2 &w3 = vertices[i3].texCoord;

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0f / (s1 * t2 - s2 * t1);
		glm::vec4 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r, 0.0f);
		glm::vec4 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r, 0.0f);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (unsigned int a = 0; a < vertexCount; a++)
	{
		const glm::vec3 &n = glm::vec3(vertices[a].normal);
		const glm::vec3 &t = glm::vec3(tan1[a]);

		vertices[a].tangent = glm::vec4(glm::normalize(t - n * glm::dot(n, t)), 0.0f);
		vertices[a].tangent.w = (glm::dot(glm::cross(glm::vec3(n), glm::vec3(t)), glm::vec3(tan2[a])) < 0.0f) ? 1.0f : -1.0f;
	}

	delete[] tan1;
}
