#include "Common.h"

#include "Application.h"

#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <glad.h>

#include "imgui_glfw3.h"

#include "Gizmos.h"
using aie::Gizmos;

#include "RenderTarget.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Instance.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "ParticleSystem.h"

// TODO: Solar system could be it's own class, takes up a lot of space. Whole file could generally be a lot cleaner.

// TODO: Shadow rendering.
// TODO: Deferred rendering, maybe?

class MyApp : public Application // Implement interface;
{
	using Application::Application; // Use superclass's constructor.

protected:
	virtual bool Init() override
	{
		srand((unsigned int)time(nullptr)); // Random Seed.

		GLFWwindow *window = (GLFWwindow*)GetRawWindowHandle();

		SetBackgroundColor({ 1, 0, 0, 1 }); // red loading screen lol.
		ClearScreen();
		UpdateWindow();

		// Initialise render targets.
		if (m_postProcessTarget.initialise(1, GetWindowWidth(), GetWindowHeight()) == false)
		{
			std::cout << "Error whilst initialising Render Target." << std::endl;
			return false;
		}

		if (m_renderTarget.initialise(1, GetWindowWidth(), GetWindowHeight()) == false)
		{
			std::cout << "Error whilst initialising Render Target." << std::endl;
			return false;
		}

		// Initialise ImGUI.
		aie::ImGui_Init(window, true);

		glfwSetKeyCallback(window, &aie::ImGui_KeyCallback);
		glfwSetCharCallback(window, &aie::ImGui_CharCallback);
		glfwSetMouseButtonCallback(window, &aie::ImGui_MouseButtonCallback);
		glfwSetScrollCallback(window, &aie::ImGui_ScrollCallback);

		// Initialise Gizmos.
		Gizmos::create(10000, 10000, 10000, 10000);

		// Load resources.
		m_postProcessShader.loadShader(aie::eShaderStage::VERTEX, "./res/shaders/post.vert");
		m_postProcessShader.loadShader(aie::eShaderStage::FRAGMENT, "./res/shaders/post.frag");
		if (m_postProcessShader.link() == false)
		{
			std::cout << "Error whilst linking shader program: " << m_postProcessShader.getLastError() << std::endl;
			return false;
		}

		m_shader.loadShader(aie::eShaderStage::VERTEX, "./res/shaders/pbr.vert");
		m_shader.loadShader(aie::eShaderStage::FRAGMENT, "./res/shaders/pbr.frag");
		if (m_shader.link() == false)
		{
			std::cout << "Error whilst linking shader program: " << m_shader.getLastError() << std::endl;
			return false;
		}

		m_textureShader.loadShader(aie::eShaderStage::VERTEX, "./res/shaders/texture.vert");
		m_textureShader.loadShader(aie::eShaderStage::FRAGMENT, "./res/shaders/texture.frag");
		if (m_textureShader.link() == false)
		{
			std::cout << "Error whilst linking shader program: " << m_textureShader.getLastError() << std::endl;
			return false;
		}

		m_particleShader.loadShader(aie::eShaderStage::VERTEX, "./res/shaders/particle.vert");
		m_particleShader.loadShader(aie::eShaderStage::FRAGMENT, "./res/shaders/particle.frag");
		if (m_particleShader.link() == false)
		{
			std::cout << "Error whilst linking shader program: " << m_particleShader.getLastError() << std::endl;
			return false;
		}

		// Create and initialize render objects.
		m_fullscreenMesh.InitializeFullscreenQuad(); // For post processing, unused.

		m_quadMesh.InitializeQuad();
		m_quadTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 10.0f)) * 
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0)) * 
			glm::scale(glm::mat4(1.0f), { 10.0f, 10.0f, 10.0f });

		m_spearMesh.InitializeFromFile("./res/models/soulspear/soulspear.obj");
		m_spearMesh.LoadMaterial("./res/models/soulspear/soulspear.mtl");
		m_spearTransform = glm::translate(glm::mat4(1.0f), { 0.0f, 1.0f, 0.0f }) * glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });

		m_emitter = new ParticleEmitter();
		m_emitter->Initialise(1000, 500, 0.1f, 1.0f, 1.0f, 5.0f, 1.0f, 0.1f, glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1));
		m_emitterTransform = glm::translate(glm::mat4(1.0f), { 5.0f, 1.0f, 5.0f });

		// Setup scene.
		m_sunLight.direction = { -0.5f, 0.5f, -1.0f, 1.0f };
		m_sunLight.color = { 1.5f, 1.5f, 1.5f, 1.0f };
		m_scene = new Scene(&m_camera, m_sunLight, { 0.25f, 0.25f, 0.25f });

		for (int i = -4; i <= 4; i++)
		{
			m_scene->AddInstance(new Instance(glm::translate(glm::mat4(1.0f), { i * 2.5f, 0.0f, 0.0f }), &m_spearMesh, &m_shader));
		}

		m_scene->AddInstance(new Instance(glm::translate(glm::mat4(1.0f), { -5.0f, 1.0f, -3.0f }), &m_primitiveMesh, &m_shader));

		m_scene->AddPointLight(PointLight(glm::vec3(6.0f, 3.0f, -3.0f), glm::vec3(1, 0, 0), 50));
		m_scene->AddPointLight(PointLight(glm::vec3(-6.0f, 3.0f, -3.0f), glm::vec3(0, 1, 0), 50));
		m_scene->AddSpotLight(SpotLight(glm::vec3(0.0f, 3.0f, -3.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1, 1, 1), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)), 50));

		m_rtCamera.SetPosition({ 5.5f, 4.0f, -10.0f });
		m_rtCamera.SetRotation({ -180.0f, -75.0f });

		// Init animation.
		m_hipFrames[0].position = glm::vec3(0, 5, 0);
		m_hipFrames[0].rotation = glm::quat(glm::vec3(1, 0, 0));
		m_hipFrames[1].position = glm::vec3(0, 5, 0);
		m_hipFrames[1].rotation = glm::quat(glm::vec3(-1, 0, 0));

		m_kneeFrames[0].position = glm::vec3(0, -2.5f, 0);
		m_kneeFrames[0].rotation = glm::quat(glm::vec3(1, 0, 0));
		m_kneeFrames[1].position = glm::vec3(0, -2.5f, 0);
		m_kneeFrames[1].rotation = glm::quat(glm::vec3(0, 0, 0));

		m_ankleFrames[0].position = glm::vec3(0, -2.5f, 0);
		m_ankleFrames[0].rotation = glm::quat(glm::vec3(-1, 0, 0));
		m_ankleFrames[1].position = glm::vec3(0, -2.5f, 0);
		m_ankleFrames[1].rotation = glm::quat(glm::vec3(0, 0, 0));

		return true;
	}

	virtual bool Shutdown() override
	{
		delete m_emitter; m_emitter = nullptr;
		delete m_scene; m_scene = nullptr;

		aie::ImGui_Shutdown();

		Gizmos::destroy();

		return true;
	}

	virtual bool Update(float dt) override
	{
		GLFWwindow *window = (GLFWwindow *)GetRawWindowHandle();

		aie::ImGui_NewFrame();

		float time = (float)glfwGetTime();
		//m_light.direction = glm::normalize(glm::vec3(glm::cos(time * 2), glm::sin(time * 2), 0.0f));

		m_camera.DoFlyCam(dt);

		// Do animation.
		float s = glm::cos((float)glfwGetTime()) * 0.5f + 0.5f;
		const glm::mat4 animationParent = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
		{
			glm::vec3 p = (1.0f - s) * m_hipFrames[0].position + s * m_hipFrames[1].position;
			glm::quat r = glm::slerp(m_hipFrames[0].rotation, m_hipFrames[1].rotation, s);
			m_hipBone = animationParent * glm::translate(p) * glm::toMat4(r);
		}
		{
			glm::vec3 p = (1.0f - s) * m_kneeFrames[0].position + s * m_kneeFrames[1].position;
			glm::quat r = glm::slerp(m_kneeFrames[0].rotation, m_kneeFrames[1].rotation, s);
			m_kneeBone = m_hipBone * glm::translate(p) * glm::toMat4(r);
		}
		{
			glm::vec3 p = (1.0f - s) * m_ankleFrames[0].position + s * m_ankleFrames[1].position;
			glm::quat r = glm::slerp(m_ankleFrames[0].rotation, m_ankleFrames[1].rotation, s);
			m_ankleBone = m_kneeBone * glm::translate(p) * glm::toMat4(r);
		}

		m_emitter->Update(dt, m_camera.GetViewMatrixFromQuaternion());

		m_scene->Update(dt);
			
		#pragma region IMGUI_WINDOWS
		// ImGui light settings window.
		ImGui::Begin("Light Settings");

		ImGui::Checkbox("Toggle Debug Render", &m_debugRender);

		glm::vec3 &ambientLight = m_scene->GetAmbientLight();
		ImGui::DragFloat3("Ambient Light Colour", &ambientLight[0], 0.1f);

		ImGui::DragFloat3("Sunlight Direction", &m_sunLight.direction[0], 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat3("Sunlight Colour", &m_sunLight.color[0], 0.1f);

		if (ImGui::Button("Add Light"))
			ImGui::OpenPopup("Light_Add");

		if (ImGui::BeginPopup("Light_Add"))
		{
			static int e = 0;
			ImGui::RadioButton("Point Light", &e, 0);
			ImGui::RadioButton("Spot Light", &e, 1);

			if (ImGui::Button("Add"))
			{
				switch (e)
				{
					case 0: // Point Lights
					{
						m_scene->AddPointLight(PointLight(glm::vec3(0), glm::vec3(1), 10.0f));
					} break;
					case 1: // Spotlights
					{
						m_scene->AddSpotLight(SpotLight(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(1), 0.97f, 0.95f, 32.0f));
					} break;
					default: break;
				}
				e = 0;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::CollapsingHeader("Lights in Scene"))
		{
			ImGui::Separator();
			ImGui::Indent();
			for (int i = 0; i < m_scene->GetPointLights()->size(); i++)
			{
				PointLight &light = m_scene->GetPointLights()->data()[i];

				std::string headerText = ("Point Light " + std::to_string(i) + ":");
				ImGui::PushID(headerText.c_str());
				if (ImGui::CollapsingHeader(headerText.c_str()))
				{
					ImGui::Indent();
					ImGui::DragFloat3("Position", &light.position[0], 0.1f);
					ImGui::DragFloat("Intensity", &light.intensity, 0.1f);
					ImGui::ColorEdit4("Colour", &light.color[0], false);
					if (ImGui::Button("Delete Light"))
					{
						m_scene->RemovePointLight(&light);
					}
					ImGui::Unindent();
				}
				ImGui::PopID();
			}

			for (int i = 0; i < m_scene->GetSpotLights()->size(); i++)
			{
				SpotLight &light = m_scene->GetSpotLights()->data()[i];

				std::string headerText = ("Spot Light " + std::to_string(i) + ":");
				ImGui::PushID(headerText.c_str());
				if (ImGui::CollapsingHeader(headerText.c_str()))
				{
					ImGui::Indent();
					ImGui::DragFloat3("Position", &light.position[0], 0.1f);
					ImGui::DragFloat3("Direction", &light.direction[0], 0.01f);
					ImGui::DragFloat("Intensity", &light.intensity, 0.1f);
					ImGui::ColorEdit4("Colour", &light.color[0], false);
					ImGui::DragFloat("Inner Cutoff", &light.innerCutoff, 0.01f);
					ImGui::DragFloat("Outer Cutoff", &light.outerCutoff, 0.01f);
					if (ImGui::Button("Delete Light"))
					{
						m_scene->RemoveSpotLight(&light);
					}
					ImGui::Unindent();
				}
				ImGui::PopID();
			}
			ImGui::Unindent();
		}

		ImGui::End();

		// ImGui instances Window
		ImGui::Begin("Instances");

		if (ImGui::Button("Add Primitive"))
			ImGui::OpenPopup("Primitive_Add");

		if (ImGui::BeginPopup("Primitive_Add"))
		{
			static int e = 0;
			ImGui::RadioButton("Triangle", &e, 0);
			ImGui::RadioButton("Quad", &e, 1);
			ImGui::RadioButton("Cube", &e, 2);
			ImGui::RadioButton("Pyramid", &e, 3);
			ImGui::RadioButton("Cone", &e, 4);
			ImGui::RadioButton("Cylinder", &e, 5);
			ImGui::RadioButton("Sphere", &e, 6);

			if (ImGui::Button("Add"))
			{
				Mesh::PrimitiveID selectedType = (Mesh::PrimitiveID)e;
				// TODO: need to eventually delete this mesh along with the instance, 
				// can't use a stack allocated mesh since it pretty much immediately goes out of scope and gets deleted causing a crash.
				// could also have preallocated meshes that these reuse.
				Mesh *mesh = new Mesh();
				mesh->InitializePrimitive(selectedType);
				mesh->LoadMaterial("./res/models/stanford/Dragon.mtl");
				m_scene->AddInstance(new Instance(glm::mat4(1.0f), mesh, &m_textureShader));

				e = 0;
				ImGui::CloseCurrentPopup();
			}
			
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::CollapsingHeader("Instances in Scene"))
		{
			ImGui::Separator();
			ImGui::Indent();
			for (int i = 0; i < m_scene->GetNumInstances(); i++)
			{
				Instance *instance = m_scene->GetInstances()->data()[i];

				std::string headerText = ("Instance " + std::to_string(i) + ":");
				ImGui::PushID(headerText.c_str());
				if (ImGui::CollapsingHeader(headerText.c_str()))
				{
					glm::vec3 &position = instance->GetPosition();
					glm::vec3 &rotation = instance->GetRotation();
					glm::vec3 &scale = instance->GetScale();

					ImGui::Indent();
					ImGui::DragFloat3("Position", &position[0], 0.1f);
					ImGui::DragFloat3("Rotation", &rotation[0], 0.1f);
					ImGui::DragFloat3("Scale", &scale[0], 0.1f);
					if (ImGui::Button("Delete Instance"))
					{
						m_scene->RemoveInstance(instance);
					}
					ImGui::Unindent();
				}
				ImGui::PopID();
			}
			{
				std::string headerText = ("RT Camera:");
				ImGui::PushID(headerText.c_str());
				if (ImGui::CollapsingHeader(headerText.c_str()))
				{
					glm::vec3 &position = m_rtCamera.GetPosition();
					float &theta = m_rtCamera.GetTheta();
					float &phi = m_rtCamera.GetPhi();

					ImGui::Indent();
					ImGui::DragFloat3("Position", &position[0], 0.1f);
					ImGui::DragFloat("Pitch", &phi, 0.1f);
					ImGui::DragFloat("Yaw", &theta, 0.1f); 
					ImGui::Unindent();
				}
				ImGui::PopID();
			}
			ImGui::Unindent();
		}

		ImGui::End();
		#pragma endregion

		// Quit.
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			Quit();
			return true;
		}

		return true;
	}

	virtual bool LateUpdate(float dt) override
	{
		m_scene->LateUpdate(dt);

		return true;
	}

	virtual bool Draw() override
	{
		float time = (float)glfwGetTime();

		// Draw scene to render target.
		m_renderTarget.bind();
		{
			m_scene->SetCamera(&m_rtCamera);
			SetBackgroundColor({ 0.25f, 0.05f, 0.15f, 1.0f });
			ClearScreen();

			m_scene->Draw();

			// Draw Particle Emitter.
			glm::mat4 pv = (m_rtCamera.GetProjectionMatrix(90.0f, (float)GetWindowWidth(), (float)GetWindowHeight()) * m_rtCamera.GetViewMatrixFromQuaternion());
			m_particleShader.bind();
			m_particleShader.bindUniform("mvp", pv * m_emitterTransform);
			m_emitter->Draw();

			Gizmos::draw(pv);
		}
		m_renderTarget.unbind();

		m_scene->SetCamera(&m_camera);
		SetBackgroundColor({ 0.25f, 0.25f, 0.25f, 1.0f });
		//m_postProcessTarget.bind();
		{
			ClearScreen();

			Gizmos::clear();

			// Draw debug gizmos.
			if (m_debugRender == true)
			{
				// Draw grid.
				Gizmos::addTransform(glm::mat4(1.0f));
				glm::vec4 whiteColor(1.0f);
				glm::vec4 blackColor(0.0f, 0.0f, 0.0f, 1.0f);
				for (int i = 0; i < 21; ++i)
				{
					Gizmos::addLine(glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), i == 10 ? whiteColor : blackColor);
					Gizmos::addLine(glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), i == 10 ? whiteColor : blackColor);
				}

				// Draw lights.
				for (int i = 0; i < m_scene->GetPointLights()->size(); i++)
				{
					PointLight &light = m_scene->GetPointLights()->data()[i];
					Gizmos::addSphere(light.position, 0.1f, 6, 8, light.color);
				}
				for (int i = 0; i < m_scene->GetSpotLights()->size(); i++)
				{
					SpotLight &light = m_scene->GetSpotLights()->data()[i];
					Gizmos::addSphere(light.position, 0.1f, 6, 8, light.color);
					glm::vec3 v1 = light.position + glm::normalize(light.direction) * 0.5f;
					Gizmos::addLine(light.position, v1, light.color);
				}

			#pragma region SOLAR_SYSTEM
				// Draw solar system.
				const int planetCount = 8;
				glm::mat4 parentTransform = glm::translate(glm::mat4(1.0f), { 0, 1, 5 });
				glm::mat4 sunTransform = parentTransform;
				Gizmos::addSphere(glm::vec3(0.0f), 1.0f, 12, 16, { 1, 1, 0, 1 }, &sunTransform); // Add sun.

				static Planet planets[planetCount]; // Randomly generate planets.
				for (int i = 0; i < planetCount; i++)
				{
					if (planets[i].initialized == false) // Initialise planets.
					{
						float distanceRNG = (float)(rand() % 2) / 1.5f; // Random distance offset.
						planets[i].distanceFromSun = -distanceRNG - 2.0f;

						int rotationRNG = rand() % 360; // Random rotation offset.
						planets[i].rotationOffset = (float)i * 90.0f + rotationRNG;

						float scaleRNG = (float)(rand() % 3) / 10.0f; // Random scale.
						if (scaleRNG <= 0.0f) scaleRNG = 0.1f;
						planets[i].radius = scaleRNG;

						float hueRNG = (float)(rand() % 360); // Random color.
						float satRNG = 0.5f + (float)(rand() % 5) / 10.0f;
						float valRNG = 0.8f + (float)(rand() % 2) / 10.0f;
						glm::vec3 color = glm::rgbColor(glm::vec3(hueRNG, satRNG, valRNG));
						planets[i].color = color;

						int ringRNG = rand() % 3; // Has ring or not.
						planets[i].hasRing = ringRNG == 1 ? true : false;

						int moonCount = rand() % 2; // Has moons.
						planets[i].moonCount = moonCount;
						if (moonCount > 0)
						{
							// Initialise moons.
							planets[i].moons = new Planet[moonCount];
							for (int j = 0; j < moonCount; j++)
							{
								rotationRNG = rand() % 360; // Random rotation offset.
								planets[i].moons[j].rotationOffset = (float)i * 45.0f + rotationRNG;
								distanceRNG = (float)(rand() % 1); // Random distance offset.
								planets[i].moons[j].distanceFromSun = -distanceRNG - 2.0f;
								float scale = planets[i].radius * 1.5f; // Random scale.
								if (scale <= 0.0f) scale = 0.1f;
								planets[i].moons[j].radius = scale;
								planets[i].moons[j].color = glm::vec3(1.0f); // White color.
								planets[i].moons[j].speed = planets[i].speed * 2; // Faster than planet.
							}
						}

						planets[i].initialized = true; // Planet has been initialised. (This is because it's being initialised in the draw loop and I didn't feel like moving it out to the initialise function lol.)
					}
					
					float anim = (1.0f + glm::sin(time)) / 2.0f;
					glm::mat4 planetTransform = parentTransform
						* glm::rotate(glm::mat4(1.0f), glm::radians(planets[i].rotationOffset + time * planets[i].speed), { 0, 1, 0 })
						* glm::translate(glm::mat4(1.0f), glm::vec3(planets[i].distanceFromSun + anim, 0.0f, 0.0f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(planets[i].radius));
					Gizmos::addSphere(glm::vec3(0.0f), 1.0f, 12, 16, glm::vec4(planets[i].color, 1.0f), &planetTransform); // Draw planet.

					if (planets[i].hasRing == true) // Draw planet rings.
					{
						glm::mat4 ringTransform = planetTransform
							* glm::rotate(glm::mat4(1.0f), glm::radians(time * planets[i].speed), { 0, 0, 1 })
							* glm::rotate(glm::mat4(1.0f), glm::radians(time * planets[i].speed), { -2, 0, 0 })
							* glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
						Gizmos::addDisk(glm::vec3(0.0f), 1.0f, 16, glm::vec4(planets[i].color, 1.0f), &ringTransform);
					}

					if (planets[i].moonCount > 0) // Draw planet's moons.
					{
						for (int j = 0; j < planets[i].moonCount; j++)
						{
							Planet &moon = planets[i].moons[j];
							glm::mat4 moonTransform = planetTransform
								* glm::rotate(glm::mat4(1.0f), glm::radians(moon.rotationOffset + time * moon.speed), { 0, 1, 0 })
								* glm::translate(glm::mat4(1.0f), glm::vec3(moon.distanceFromSun, 0.0f, 0.0f))
								* glm::scale(glm::mat4(1.0f), glm::vec3(moon.radius));
							Gizmos::addSphere(glm::vec3(0.0f), 1.0f, 6, 8, glm::vec4(moon.color, 1.0f), &moonTransform); // Draw moon.
						}
					}
				}
			#pragma endregion
			}

			// Draw animation.
			glm::vec3 hipPos = glm::vec3(m_hipBone[3].x, m_hipBone[3].y, m_hipBone[3].z);
			glm::vec3 kneePos = glm::vec3(m_kneeBone[3].x, m_kneeBone[3].y, m_kneeBone[3].z);
			glm::vec3 anklePos = glm::vec3(m_ankleBone[3].x, m_ankleBone[3].y, m_ankleBone[3].z);
			glm::vec4 half(0.5f);
			glm::vec4 pink(1, 0, 1, 1);
			Gizmos::addAABBFilled(hipPos, half, pink, &m_hipBone);
			Gizmos::addAABBFilled(kneePos, half, pink, &m_kneeBone);
			Gizmos::addAABBFilled(anklePos, half, pink, &m_ankleBone);

			// Draw scene.
			m_scene->Draw();

			// Draw render target quad.
			glm::mat4 pv = (m_camera.GetProjectionMatrix(90.0f, (float)GetWindowWidth(), (float)GetWindowHeight()) * m_camera.GetViewMatrixFromQuaternion());
			m_textureShader.bind();
			m_textureShader.bindUniform("mvp", pv * m_quadTransform);
			m_textureShader.bindUniform("view", m_camera.GetViewMatrixFromQuaternion());
			m_textureShader.bindUniform("model", m_quadTransform);
			m_renderTarget.getTarget(0).bind(8);
			m_textureShader.bindUniform("diffuseTex", 8);
			m_quadMesh.Draw();

			// Draw Particle Emitter.
			m_particleShader.bind();
			m_particleShader.bindUniform("mvp", pv * m_emitterTransform);
			m_emitter->Draw();

			// Draw gizmos.
			Gizmos::draw(pv);
		}
		//m_postProcessTarget.unbind();

		// Draw post processing quad.
		/*ClearScreen();
		m_postProcessShader.bind();
		m_postProcessTarget.getTarget(0).bind(16);
		m_postProcessShader.bindUniform("colorTarget", 16);
		m_fullscreenMesh.Draw();*/

		// Draw imGUI.
		ImGui::Render();

		return true;
	}

private:
	struct Planet // For solar system.
	{
		glm::vec3 color = { 1, 0, 0 };
		float radius = 0.5f;
		float distanceFromSun = -2.0f;
		float rotationOffset = 0.0f;
		float speed = 90.0f;
		bool initialized = false;

		bool hasRing = true;

		int moonCount = 0;
		Planet *moons = nullptr;

		~Planet() { delete[] moons; moons = nullptr; } // Should be called when out of scope.
	};

	struct KeyFrame
	{
		glm::vec3 position;
		glm::quat rotation;
	};

	KeyFrame m_hipFrames[2];
	KeyFrame m_kneeFrames[2];
	KeyFrame m_ankleFrames[2];
	glm::mat4 m_hipBone;
	glm::mat4 m_kneeBone;
	glm::mat4 m_ankleBone;

	Camera m_camera; // Main scene camera.
	Camera m_rtCamera; // Render target camera.

	aie::RenderTarget m_postProcessTarget;
	aie::RenderTarget m_renderTarget;

	bool m_debugRender = true;

	aie::ShaderProgram m_postProcessShader;
	aie::ShaderProgram m_shader;
	aie::ShaderProgram m_textureShader;
	aie::ShaderProgram m_particleShader;

	glm::mat4 m_quadTransform;
	glm::mat4 m_spearTransform;
	Mesh m_fullscreenMesh; // For post processing, unused.
	Mesh m_quadMesh;
	Mesh m_spearMesh;

	Mesh m_primitiveMesh;

	glm::mat4 m_emitterTransform;
	ParticleEmitter *m_emitter;

	Scene *m_scene;

	SunLight m_sunLight;

};

int main()
{
	// Create and run application.
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