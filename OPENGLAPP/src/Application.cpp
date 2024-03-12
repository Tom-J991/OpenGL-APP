#include "Application.h"

#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <glad.h>

struct GLFWStuff
{
	GLFWwindow *window;
};
static GLFWStuff s_glfwStuff;

static void WindowResizeCallback(GLFWwindow *window, int width, int height);
static void MousePositionCallback(GLFWwindow *window, double x, double y);

Application *Application::s_instance = nullptr;
Application::Application(const char *title, unsigned int width, unsigned int height, bool fullscreen)
	: m_windowTitle(title)
	, m_windowWidth(width)
	, m_windowHeight(height)
	, m_isFullscreen(fullscreen)
{ }
Application::~Application()
{ }

bool Application::Run()
{
	s_instance = this;

	if (!glfwInit()) { std::cout << "Error occured in glfwInit" << std::endl; return false; }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	s_glfwStuff.window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle, nullptr, nullptr);
	if (s_glfwStuff.window == nullptr)
	{
		std::cout << "Error when attempting to create GLFWwindow" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(s_glfwStuff.window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Error occured in gladLoadGLLoader" << std::endl;
		glfwDestroyWindow(s_glfwStuff.window);
		glfwTerminate();
		return false;
	}
	std::cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << std::endl;

	glfwSetWindowSizeCallback(s_glfwStuff.window, &WindowResizeCallback);
	glfwSetCursorPosCallback(s_glfwStuff.window, &MousePositionCallback);

	glEnable(GL_DEPTH_TEST);

	if (!Init()) { std::cout << "Error occured whilst initializing application!" << std::endl; return false; }

	float prevTime = (float)glfwGetTime();

	m_isRunning = true;
	while (m_isRunning == true)
	{
		float currentTime = (float)glfwGetTime();
		float deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		if (!Update(deltaTime)) { std::cout << "Error occured in application update!" << std::endl; return false; }

		m_lastMousePosition = m_mousePosition;

		glClearDepth(1.0f);
		glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, m_windowWidth, m_windowHeight);
		if (!Draw()) { std::cout << "Error occured in application draw!" << std::endl; return false; }

		glfwSwapBuffers(s_glfwStuff.window);
		glfwPollEvents();

		m_isRunning = !glfwWindowShouldClose(s_glfwStuff.window);
	}

	if (!Shutdown()) { std::cout << "Error occured whilst shutting down application!" << std::endl; return false; }

	glfwDestroyWindow(s_glfwStuff.window);
	glfwTerminate();

	return true;
}

void *Application::GetRawWindowHandle()
{
	return (void*)s_glfwStuff.window;
}

void WindowResizeCallback(GLFWwindow *window, int width, int height)
{
	Application::Instance()->SetWindowWidth(width);
	Application::Instance()->SetWindowHeight(height);
	glViewport(0, 0, width, height);
}

void MousePositionCallback(GLFWwindow *window, double x, double y)
{
	glm::vec2 position = glm::vec2((float)x, (float)y);
	Application::Instance()->SetMousePosition(position);
}
