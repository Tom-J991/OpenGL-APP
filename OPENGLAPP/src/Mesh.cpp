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
	// Cleanup OpenGL Objects.
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

// Initialize mesh with given vertices and optionally indices.
void Mesh::Initialize(unsigned int vertexCount, const Vertex *vertices, unsigned int indexCount, unsigned int *indices)
{
	ASSERT(m_VAO != 0, "VAO already initialized.");

	// Create OpenGL objects.
	glGenVertexArrays(1, &m_VAO); // Vertex array object.
	glGenBuffers(1, &m_VBO); // Vertex buffer object.

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW); // Pass vertices array into buffer.

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Setup vertex position attribute for shader.
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)16); // Setup vertex normal attribute for shader.
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)32); // Setup texture coordinate attribute for shader.
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)40); // Setup vertex tangent attribute for shader.
	glEnableVertexAttribArray(3);

	if (indexCount != 0) // Has indices.
	{
		ASSERT(indices == nullptr, "No indices have been passed in.");
		glGenBuffers(1, &m_EBO); // Element buffer object.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW); // Pass indices array into buffer.
		m_triCount = indexCount / 3;
	}
	else
	{
		m_triCount = vertexCount / 3;
	}

	// Unbind OpenGL objects.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Initialize the mesh object from file.
void Mesh::InitializeFromFile(const char *filePath)
{
	const aiScene *scene = aiImportFile(filePath, 0); // Import mesh from file using Assimp.
	aiMesh *mesh = scene->mMeshes[0]; // Get first mesh from assimp scene.

	// Setup indices with the imported mesh.
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

	// Setup vertices with the imported mesh.
	int numV = mesh->mNumVertices;
	Vertex *vertices = new Vertex[numV];
	for (int i = 0; i < numV; i++)
	{
		vertices[i].position = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
		vertices[i].normal = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);

		if (mesh->mTextureCoords[0]) // Imported mesh has texture coords.
		{
			vertices[i].texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y);
		}
		else // Doesn't have texture coords.
		{
			vertices[i].texCoord = glm::vec2(0.0f); // Default to zero.
		}

		if (mesh->HasTangentsAndBitangents())
		{
			vertices[i].tangent = glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f);
		}
	}

	if (!mesh->HasTangentsAndBitangents()) // Imported mesh doesn't have tangent information.
		CalculateTangents(vertices, numV, indices);

	Initialize(numV, vertices, (unsigned int)indices.size(), indices.data()); // Initialize the mesh.

	delete[] vertices; vertices = nullptr;
}

// Get material information from .mtl file.
void Mesh::LoadMaterial(const char *filePath)
{
	std::fstream file(filePath, std::ios::in);
	std::stringstream contents;
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
		contents << line;
		std::stringstream ss(line, std::stringstream::in | std::stringstream::out);
		if (line.find("Ka") == 0) // Ambient colour
			ss >> header >> Ka.x >> Ka.y >> Ka.z;
		else if (line.find("Ks") == 0) // Specular colour
			ss >> header >> Ks.x >> Ks.y >> Ks.z;
		else if (line.find("Kd") == 0) // Diffuse colour
			ss >> header >> Kd.x >> Kd.y >> Kd.z;
		else if (line.find("Ns") == 0) // Specular power.
			ss >> header >> specular;
		else if (line.find("map_Kd") == 0) // Diffuse texture.
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			mapKd.load((directory + mapFileName).c_str());
		}
		else if (line.find("map_Ks") == 0) // Specular texture.
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			mapKs.load((directory + mapFileName).c_str());
		}
		else if (line.find("bump") == 0) // Bump/normal texture.
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			mapBump.load((directory + mapFileName).c_str());
		}
	}

	// Fallback textures.
	// TODO: probably could handle the fallback checking better,
	// currently storing all the contents of the material file in a stringstream and then looking through it to see if the maps are defined.
	// TODO: Fix these fallback textures, they seem to break lighting even though they shouldn't???
	const unsigned int width = 8;
	const unsigned int height = 8;
	if (contents.str().find("map_Kd") == std::string::npos)
	{
		const size_t size = (width * height) * 3;
		unsigned char pixels[size];
		for (int i = 0, k = 0; i < size; i += 3, k++)
		{
			// funny css missing texture pattern
			if (k >= size || i >= size) break;
			int x = (k) % width;
			int y = (k) / width;
			if ((x + y) % 2 == 0)
			{
				// pink
				pixels[i+0] = 0xFF; // r
				pixels[i+1] = 0x00; // g
				pixels[i+2] = 0xFF; // b
			}
			else
			{
				// black
				pixels[i+0] = 0x00; // r
				pixels[i+1] = 0x00; // g
				pixels[i+2] = 0x00; // b
			}
		}

		mapKd.create(width, height, aie::Texture::Format::RGB, pixels);
	}
	else if (contents.str().find("map_Ks") == std::string::npos)
	{
		const size_t size = (width * height) * 3;
		unsigned char pixels[size];
		for (int i = 0; i < size; i += 3)
		{
			// black
			if (i >= size) break;
			pixels[i+0] = 0x00; // r
			pixels[i+1] = 0x00; // g
			pixels[i+2] = 0x00; // b
		}

		mapKs.create(width, height, aie::Texture::Format::RGB, pixels);
	}
	else if (contents.str().find("bump") == std::string::npos)
	{
		const size_t size = (width * height) * 3;
		unsigned char pixels[size];
		for (int i = 0; i < size; i += 3)
		{
			// normal map neutral blue (127, 127, 255)
			if (i >= size) break;
			pixels[i+0] = 0x7F; // r
			pixels[i+1] = 0x7F; // g
			pixels[i+2] = 0xFF; // b
		}

		mapBump.create(width, height, aie::Texture::Format::RGB, pixels);
	}
}

void Mesh::ApplyMaterial(aie::ShaderProgram *shader)
{
	// Bind material information to shader.
	shader->bindUniform("specular", specular); // TODO: fix bug with shader stating "specular" is not being found or used even though it is?
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

void Mesh::MakeMaterial(glm::vec3 Ka, glm::vec3 Kd, glm::vec3 Ks, float specular, const char *diffusePath, const char *specularPath, const char *normalPath)
{
	this->specular = specular;
	this->Ka = Ka;
	this->Kd = Kd;
	this->Ks = Ks;

	if (diffusePath) mapKd.load(diffusePath);
	if (specularPath) mapKs.load(specularPath);
	if (normalPath) mapBump.load(normalPath);
}

void Mesh::InitializeQuad()
{
	ASSERT(m_VAO != 0, "VAO already initialized.");

	// Quad vertices.
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

	glGenVertexArrays(1, &m_VAO); // Vertex Array Object
	glGenBuffers(1, &m_VBO); // Vertex Buffer Object
	//glGenBuffers(1, &m_EBO); // Element Buffer Object, unused

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW); // Pass vertices array into buffer.

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Setup vertex position attribute for shader.
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)16); // Setup vertex normal attribute for shader.
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)32); // Setup texture coordinate attribute for shader.
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)40); // Setup vertex tangent attribute for shader.
	glEnableVertexAttribArray(3);

	// Unbind objects.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::InitializeFullscreenQuad()
{
	ASSERT(m_VAO != 0, "VAO already initialized.");

	// Quad vertices.
	m_triCount = 2;
	float vertices[] =
	{
		-1,  1,
		-1, -1,
		 1,  1,
		-1, -1,
		 1, -1,
		 1,  1
	};

	// Setup vertex array object and buffer objects like every other time.
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	//glGenBuffers(1, &m_EBO); // Unused.

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::InitializePrimitive(PrimitiveID type)
{
	constexpr float unit = 1.0f;

	Vertex *vertices = nullptr;
	std::vector<unsigned int> indices;
	unsigned int vertexCount = 0;

	switch (type)
	{
		case PRIMITIVE_TRIANGLE:
		{
			vertexCount = 3;
			vertices = new Vertex[vertexCount];

			vertices[0].position = glm::vec4(-unit, -unit, 0.0f, 1.0f);
			vertices[1].position = glm::vec4(0.0f, unit, 0.0f, 1.0f);
			vertices[2].position = glm::vec4(unit, -unit, 0.0f, 1.0f);

			vertices[0].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[1].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[2].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

			vertices[0].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[1].texCoord = glm::vec2(0.5f, 1.0f);
			vertices[2].texCoord = glm::vec2(1.0f, 0.0f);
		} break;
		case PRIMITIVE_QUAD:
		{
			vertexCount = 4;
			vertices = new Vertex[vertexCount];

			vertices[0].position = glm::vec4(-unit, unit, 0.0f, 1.0f);
			vertices[1].position = glm::vec4(unit, unit, 0.0f, 1.0f);
			vertices[2].position = glm::vec4(-unit, -unit, 0.0f, 1.0f);
			vertices[3].position = glm::vec4(unit, -unit, 0.0f, 1.0f);

			vertices[0].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[1].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[2].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[3].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

			vertices[0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(0);
			indices.push_back(2);
			indices.push_back(1);
			indices.push_back(2);
			indices.push_back(3);
			indices.push_back(1);
		} break;
		case PRIMITIVE_CUBE:
		{
			// TODO: cube generation could probably be done in a for loop or two instead of being written out entirely.
			vertexCount = 24; // 4 vertices for (6) sides.
			vertices = new Vertex[vertexCount];

			unsigned int index = 0;
			// Front
			vertices[index + 0].position = glm::vec4(-unit, unit, unit, 1.0f);
			vertices[index + 1].position = glm::vec4(unit, unit, unit, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, -unit, unit, 1.0f);
			vertices[index + 3].position = glm::vec4(unit, -unit, unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;
			// Back
			vertices[index + 0].position = glm::vec4(-unit, unit, -unit, 1.0f);
			vertices[index + 1].position = glm::vec4(unit, unit, -unit, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, -unit, -unit, 1.0f);
			vertices[index + 3].position = glm::vec4(unit, -unit, -unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;
			// Left
			vertices[index + 0].position = glm::vec4(-unit, unit, -unit, 1.0f);
			vertices[index + 1].position = glm::vec4(-unit, unit, unit, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, -unit, -unit, 1.0f);
			vertices[index + 3].position = glm::vec4(-unit, -unit, unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;
			// Right
			vertices[index + 0].position = glm::vec4(unit, unit, -unit, 1.0f);
			vertices[index + 1].position = glm::vec4(unit, unit, unit, 1.0f);
			vertices[index + 2].position = glm::vec4(unit, -unit, -unit, 1.0f);
			vertices[index + 3].position = glm::vec4(unit, -unit, unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;
			// Top
			vertices[index + 0].position = glm::vec4(-unit, unit, unit, 1.0f);
			vertices[index + 1].position = glm::vec4(unit, unit, unit, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, unit, -unit, 1.0f);
			vertices[index + 3].position = glm::vec4(unit, unit, -unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;
			// Bottom
			vertices[index + 0].position = glm::vec4(-unit, -unit, unit, 1.0f);
			vertices[index + 1].position = glm::vec4(unit, -unit, unit, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, -unit, -unit, 1.0f);
			vertices[index + 3].position = glm::vec4(unit, -unit, -unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;

		} break;
		case PRIMITIVE_PYRAMID:
		{
			vertexCount = 16; // 3 for 4 tris and 1 for bottom quad.
			vertices = new Vertex[vertexCount];

			unsigned int index = 0;
			// Bottom
			vertices[index + 0].position = glm::vec4(-unit, -unit, unit, 1.0f);
			vertices[index + 1].position = glm::vec4(unit, -unit, unit, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, -unit, -unit, 1.0f);
			vertices[index + 3].position = glm::vec4(unit, -unit, -unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index + 3].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 1.0f);
			vertices[index + 1].texCoord = glm::vec2(1.0f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 3].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			indices.push_back(index + 1);

			index += 4;
			// Front
			vertices[index + 0].position = glm::vec4(-unit, -unit, unit, 1.0f);
			vertices[index + 1].position = glm::vec4(0.0f, unit, 0.0f, 1.0f);
			vertices[index + 2].position = glm::vec4(unit, -unit, unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 1].texCoord = glm::vec2(0.5f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);

			index += 3;
			// Back
			vertices[index + 0].position = glm::vec4(-unit, -unit, -unit, 1.0f);
			vertices[index + 1].position = glm::vec4(0.0f, unit, 0.0f, 1.0f);
			vertices[index + 2].position = glm::vec4(unit, -unit, -unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 1].texCoord = glm::vec2(0.5f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);

			index += 3;
			// Left
			vertices[index + 0].position = glm::vec4(-unit, -unit, -unit, 1.0f);
			vertices[index + 1].position = glm::vec4(0.0f, unit, 0.0f, 1.0f);
			vertices[index + 2].position = glm::vec4(-unit, -unit, unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 1].texCoord = glm::vec2(0.5f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);

			index += 3;
			// Right
			vertices[index + 0].position = glm::vec4(unit, -unit, -unit, 1.0f);
			vertices[index + 1].position = glm::vec4(0.0f, unit, 0.0f, 1.0f);
			vertices[index + 2].position = glm::vec4(unit, -unit, unit, 1.0f);

			vertices[index + 0].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 1].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertices[index + 2].normal = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

			vertices[index + 0].texCoord = glm::vec2(0.0f, 0.0f);
			vertices[index + 1].texCoord = glm::vec2(0.5f, 1.0f);
			vertices[index + 2].texCoord = glm::vec2(1.0f, 0.0f);

			indices.push_back(index + 0);
			indices.push_back(index + 2);
			indices.push_back(index + 1);

			index += 3;
		} break;
		case PRIMITIVE_CONE:
		{
			// TODO: fix cone generation.
			const int sectorCount = 12;

			vertexCount = (sectorCount + 4) * 2 + 1; // probably incorrect atm.
			vertices = new Vertex[vertexCount];

			std::vector<float> tempVertices;
			constexpr float pi = glm::pi<float>();
			float sectorStep = 2 * pi / sectorCount;
			float sectorAngle = 0.0f;
			float height = unit;

			for (int i = 0; i <= sectorCount; ++i) // calculate temp unit circles
			{
				sectorAngle = i * sectorStep;
				tempVertices.push_back(glm::cos(sectorAngle));
				tempVertices.push_back(0.0f);
				tempVertices.push_back(glm::sin(sectorAngle));
			}

			// vertices
			unsigned int index = 0;
			// sides
			for (int j = 0, k = 0; j <= sectorCount; j += 2) // probably a better way of doing this.
			{
				float ux = tempVertices[k];
				float uy = tempVertices[k + 1];
				float uz = tempVertices[k + 2];

				float u = (float)j / sectorCount; // the u from uv
				float v = 1.0f; // the v from uv

				vertices[index].position = glm::vec4(0.0f, height, 0.0f, 1.0f);
				vertices[index].normal = glm::vec4(ux, uy, uz, 0.0f);
				vertices[index].texCoord = glm::vec2(u, v);
				index++;

				v = 0.0f;
				vertices[index].position = glm::vec4(ux * unit, -height, uz * unit, 1.0f);
				vertices[index].normal = glm::vec4(ux, uy, uz, 0.0f);
				vertices[index].texCoord = glm::vec2(u, v);
				index++;

				k += 3;
				if (k >= sectorCount * 3) break;

				ux = tempVertices[k];
				uy = tempVertices[k + 1];
				uz = tempVertices[k + 2];

				vertices[index].position = glm::vec4(ux * unit, -height, uz * unit, 1.0f);
				vertices[index].normal = glm::vec4(ux, uy, uz, 0.0f);
				vertices[index].texCoord = glm::vec2(u, v);

				index++;
				k += 3;
			}

			unsigned int baseCenterIndex = index;
			// bottom
			vertices[index].position = glm::vec4(0.0f, -height, 0.0f, 1.0f);
			vertices[index].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
			vertices[index].texCoord = glm::vec2(0.5f, 0.5f);
			index++;

			for (int j = 0, k = 0; j < sectorCount; ++j, index++, k += 3)
			{
				float ux = tempVertices[k];
				float uz = tempVertices[k + 2];
				vertices[index].position = glm::vec4(ux * unit, -height, uz * unit, 1.0f);
				vertices[index].normal = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
				vertices[index].texCoord = glm::vec2(-ux * 0.5f + 0.5f, -uz * 0.5f + 0.5f);
			}

			unsigned int k = 0;
			// sides
			for (int i = 0; i < sectorCount + 6; ++i, ++k) // probably a better way of doing this. sectorCount + 6 ????
			{
				indices.push_back(k);
				indices.push_back(k + 1);
				indices.push_back(k + 2);
			}

			k = baseCenterIndex + 1;
			// bottom
			for (int i = 0; i < sectorCount; ++i, ++k)
			{
				if (i < sectorCount - 1)
				{
					indices.push_back(baseCenterIndex);
					indices.push_back(k + 1);
					indices.push_back(k);
				}
				else
				{
					indices.push_back(baseCenterIndex);
					indices.push_back(baseCenterIndex + 1);
					indices.push_back(k);
				}
			}
		} break;
		case PRIMITIVE_CYLINDER:
		{
			// TODO: fix cylinder generation.
			const int sectorCount = 12;

			vertexCount = (sectorCount * 2 + 2) * 2; // algorithm uses a higher vertex count than it should, not sure why. might be accounting for normals?
			vertices = new Vertex[vertexCount];

			std::vector<float> tempVertices;
			constexpr float pi = glm::pi<float>();
			float sectorStep = 2 * pi / sectorCount;
			float sectorAngle = 0.0f;
			float height = unit * 2;

			for (int i = 0; i <= sectorCount; ++i) // calculate temp unit circles
			{
				sectorAngle = i * sectorStep;
				tempVertices.push_back(glm::cos(sectorAngle));
				tempVertices.push_back(0.0f);
				tempVertices.push_back(glm::sin(sectorAngle));
			}

			// vertices
			unsigned int index = 0;
			// sides
			for (int i = 0; i < 2; ++i)
			{
				float h = -height / 2.0f + i * height;
				float v = 1.0f - i; // the v from uv

				for (int j = 0, k = 0; j <= sectorCount; ++j, index++, k += 3)
				{
					float ux = tempVertices[k];
					float uy = tempVertices[k+1];
					float uz = tempVertices[k+2];

					float u = (float)j / sectorCount; // the u from uv

					vertices[index].position = glm::vec4(ux * unit, h, uz * unit, 1.0f);
					vertices[index].normal = glm::vec4(ux, uy, uz, 0.0f);
					vertices[index].texCoord = glm::vec2(u, v);
				}
			}

			unsigned int baseCenterIndex = index;
			unsigned int topCenterIndex = baseCenterIndex + sectorCount + 1;
			// top and bottom
			for (int i = 0; i < 2; ++i)
			{
				float h = -height / 2.0f + i * height;
				float ny = (float)(-1 + i * 2);

				vertices[index].position = glm::vec4(0.0f, h, 0.0f, 1.0f);
				vertices[index].normal = glm::vec4(0.0f, ny, 0.0f, 0.0f);
				vertices[index].texCoord = glm::vec2(0.5f, 0.5f);
				index++;

				for (int j = 0, k = 0; j < sectorCount; ++j, index++, k += 3)
				{
					float ux = tempVertices[k];
					float uz = tempVertices[k+2];
					vertices[index].position = glm::vec4(ux * unit, h, uz * unit, 1.0f);
					vertices[index].normal = glm::vec4(0.0f, ny, 0.0f, 0.0f);
					vertices[index].texCoord = glm::vec2(-ux * 0.5f + 0.5f, -uz * 0.5f + 0.5f);
				}
			}

			// indices
			unsigned int k1 = 0, k2 = sectorCount + 1;
			// sides
			for (int i = 0; i < sectorCount; ++i, ++k1, ++k2)
			{
				indices.push_back(k1);
				indices.push_back(k1 + 1);
				indices.push_back(k2);

				indices.push_back(k2);
				indices.push_back(k1 + 1);
				indices.push_back(k2 + 1);
			}

			unsigned int k = baseCenterIndex + 1;
			// bottom
			for (int i = 0; i < sectorCount; ++i, ++k)
			{
				if (i < sectorCount - 1)
				{
					indices.push_back(baseCenterIndex);
					indices.push_back(k + 1);
					indices.push_back(k);
				}
				else
				{
					indices.push_back(baseCenterIndex);
					indices.push_back(baseCenterIndex + 1);
					indices.push_back(k);
				}
			}

			k = topCenterIndex + 1;
			// top
			for (int i = 0; i < sectorCount; ++i, ++k)
			{
				if (i < sectorCount - 1)
				{
					indices.push_back(topCenterIndex);
					indices.push_back(k);
					indices.push_back(k + 1);
				}
				else
				{
					indices.push_back(topCenterIndex);
					indices.push_back(k);
					indices.push_back(topCenterIndex + 1);
				}
			}
		} break;
		case PRIMITIVE_SPHERE:
		{
			// TODO: fix sphere generation.
			const unsigned int sectorCount = 12;
			const unsigned int stackCount = 6;

			vertexCount = (sectorCount+1) * (stackCount+1); // the proper vertex count should be 62 according to blender's sphere primitive but this algorithm creates 91, not sure why
			vertices = new Vertex[vertexCount];

			float x, y, z, xz;
			float nx, ny, nz;
			float u, v;

			constexpr float pi = glm::pi<float>();
			float sectorStep = 2 * pi / sectorCount;
			float stackStep = pi / stackCount;
			float sectorAngle = 0.0f, stackAngle = 0.0f;
			float invRadius = 1.0f / unit;

			// vertices
			unsigned int index = 0;
			for (int i = 0; i <= stackCount; ++i)
			{
				stackAngle = pi / 2 - i * stackStep;
				xz = unit * glm::cos(stackAngle);
				y = unit * glm::sin(stackAngle);

				for (int j = 0; j <= sectorCount; ++j, ++index)
				{
					sectorAngle = j * sectorStep;
					x = xz * glm::cos(sectorAngle);
					z = xz * glm::sin(sectorAngle);

					vertices[index].position = glm::vec4(x, y, z, 1.0f);

					nx = x * invRadius;
					ny = y * invRadius;
					nz = z * invRadius;
					vertices[index].normal = glm::vec4(nx, ny, nz, 0.0f);

					u = (float)j / sectorCount;
					v = (float)i / stackCount;
					vertices[index].texCoord = glm::vec2(u, v);
				}
			}

			// indices
			unsigned int k1 = 0, k2 = 0;
			for (int i = 0; i < stackCount; ++i)
			{
				k1 = i * (sectorCount + 1);
				k2 = k1 + sectorCount + 1;

				for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
				{
					if (i != 0)
					{
						indices.push_back(k1);
						indices.push_back(k2);
						indices.push_back(k1 + 1);
					}
					if (i != (stackCount - 1))
					{
						indices.push_back(k1 + 1);
						indices.push_back(k2);
						indices.push_back(k2 + 1);
					}
				}
			}
		} break;
	}
	CalculateTangents(vertices, vertexCount, indices);
	Initialize(vertexCount, vertices, (unsigned int)indices.size(), indices.data());

	delete[] vertices;
}

void Mesh::Draw()
{
	glBindVertexArray(m_VAO);
	if (m_EBO != 0)
	{
		// Draw with indices.
		glDrawElements(GL_TRIANGLES, 3 * m_triCount, GL_UNSIGNED_INT, 0);
	}
	else
	{
		// Draw with vertices.
		glDrawArrays(GL_TRIANGLES, 0, 3 * m_triCount);
	}
}

void Mesh::CalculateTangents(Vertex *vertices, unsigned int vertexCount, const std::vector<unsigned int> &indices)
{
	glm::vec4 *tan1 = new glm::vec4[vertexCount * 2]; // Temp array.
	glm::vec4 *tan2 = tan1 + vertexCount;
	memset(tan1, 0, vertexCount * sizeof(glm::vec4) * 2);

	// Compute the tangent at each point.
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
