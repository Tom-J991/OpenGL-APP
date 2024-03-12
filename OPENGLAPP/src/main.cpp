#include "Common.h"

#include "Application.h"

#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <glad.h>

#include "imgui_glfw3.h"

#include "Gizmos.h"
using aie::Gizmos;

#include "Texture.h"

#include "Shader.h"
#include "Mesh.h"

#include "Camera.h"

class MyApp : public Application
{
	using Application::Application;

protected:
	virtual bool Init() override
	{
		GLFWwindow *window = (GLFWwindow*)GetRawWindowHandle();

		SetBackgroundColor({ 0.25f, 0.25f, 0.25f, 1.0f });

		aie::ImGui_Init(window, true);

		Gizmos::create(10000, 10000, 10000, 10000);
		float aspectRatio = (float)GetWindowWidth() / GetWindowHeight();

		m_gridTexture.load("./res/textures/numbered_grid.tga");

		m_shader.loadShader(aie::eShaderStage::VERTEX, "./res/shaders/simple.vert");
		m_shader.loadShader(aie::eShaderStage::FRAGMENT, "./res/shaders/simple.frag");
		if (m_shader.link() == false)
		{
			std::cout << "Error whilst linking shader program: " << m_shader.getLastError() << std::endl;
			return false;
		}

		//m_quadMesh.InitializeQuad();
		m_quadMesh.InitializeFromFile("./res/models/soulspear/soulspear.obj");
		m_quadMesh.LoadMaterial("./res/models/soulspear/soulspear.mtl");
		m_quadTransform = glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });

		m_ambientLight = { 0.25f, 0.25f, 0.25f };
		m_light.color = { 1.0f, 1.0f, 1.0f };

		return true;
	}
	virtual bool Shutdown() override
	{
		aie::ImGui_Shutdown();

		Gizmos::destroy();

		return true;
	}
	virtual bool Update(float dt) override
	{
		GLFWwindow *window = (GLFWwindow*)GetRawWindowHandle();
		
		aie::ImGui_NewFrame();

		//float time = (float)glfwGetTime();
		//m_light.direction = glm::normalize(glm::vec3(glm::cos(time * 2), glm::sin(time * 2), 0.0f));

		m_camera.Update(dt);

		ImGui::Begin("Light Settings");
		ImGui::DragFloat3("Sunlight direction", &m_light.direction[0], 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat3("Sunlight Colour", &m_light.color[0], 0.1f, 0.0f, 2.0f);
		ImGui::End();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			Quit();
			return true;
		}

		return true;
	}
	virtual bool Draw() override
	{
		glm::mat4 pv = m_camera.GetProjectionMatrix(90.0f, (float)GetWindowWidth(), (float)GetWindowHeight()) * m_camera.GetViewMatrix();

		Gizmos::clear();

		// Draw Grid.
		Gizmos::addTransform(glm::mat4(1.0f));
		glm::vec4 whiteColor(1.0f);
		glm::vec4 blackColor(0.0f, 0.0f, 0.0f, 1.0f);
		for (int i = 0; i < 21; ++i)
		{
			Gizmos::addLine(glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), i == 10 ? whiteColor : blackColor);
			Gizmos::addLine(glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), i == 10 ? whiteColor : blackColor);
		}

		Gizmos::draw(pv);

		m_shader.bind();
		m_shader.bindUniform("cameraPosition", m_camera.GetPosition());
		m_shader.bindUniform("ambientColor", m_ambientLight);
		m_shader.bindUniform("lightDir", m_light.direction);
		m_shader.bindUniform("lightColor", m_light.color);
		m_shader.bindUniform("mvp", pv * m_quadTransform);
		m_shader.bindUniform("model", m_quadTransform);
		m_gridTexture.bind(0);
		m_shader.bindUniform("diffuseTex", 0);
		m_quadMesh.ApplyMaterial(&m_shader);
		m_quadMesh.Draw();

		ImGui::Render();

		return true;
	}

private:
	struct Light
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

private:
	Camera m_camera;

	aie::Texture m_gridTexture;

	aie::ShaderProgram m_shader;

	glm::mat4 m_quadTransform;
	Mesh m_quadMesh;

	glm::vec3 m_ambientLight;
	Light m_light;

};

int main()
{
	MyApp *application = new MyApp("My Application", 1280, 720);
	if (!application->Run())
	{
		std::cout << "Fatal error whilst running application. Exiting.." << std::endl;
		std::cin.ignore();
		return -1;
	}
	delete application; application = nullptr;

	return 0;
}