#include "Application.h"

#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <glad.h>

struct GLFWStuff
{
	GLFWwindow *window;
};
static GLFWStuff s_glfwStuff; // An abstraction that isn't really necessary in this application on hindsight.

static void WindowResizeCallback(GLFWwindow *window, int width, int height);
static void MousePositionCallback(GLFWwindow *window, double x, double y);

Application *Application::s_instance = nullptr; // So the static variable is in a translation unit and can be used.
Application::Application(const char *title, unsigned int width, unsigned int height, bool fullscreen)
	: m_windowTitle(title)
	, m_windowWidth(width)
	, m_windowHeight(height)
	, m_isFullscreen(fullscreen)
{ }
Application::~Application()
{ }

bool Application::Run() // Basically does everything.
{
	s_instance = this;

	// Initialise GLFW and window.
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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // Load OpenGL functions with GLAD.
	{
		std::cout << "Error occured in gladLoadGLLoader" << std::endl;
		glfwDestroyWindow(s_glfwStuff.window);
		glfwTerminate();
		return false;
	}
	std::cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << std::endl;

	// Setup input/window callbacks.
	glfwSetWindowSizeCallback(s_glfwStuff.window, &WindowResizeCallback);
	glfwSetCursorPosCallback(s_glfwStuff.window, &MousePositionCallback);

	// Setup OpenGL.
	//glEnable(GL_FRAMEBUFFER_SRGB); // Enable SRGB.
	//glDisable(0x809D); // Disable Multisampling.

	glEnable(GL_DEPTH_TEST);

	// Call application init.
	if (!Init()) { std::cout << "Error occured whilst initializing application!" << std::endl; return false; }

	double prevTime = glfwGetTime();

	// Do Update Loop.
	m_isRunning = true;
	while (m_isRunning == true)
	{
		double currentTime = glfwGetTime();
		float deltaTime = (float)(currentTime - prevTime);
		prevTime = currentTime;

		// Call application update.
		if (!Update(deltaTime)) { std::cout << "Error occured in application update!" << std::endl; return false; }

		m_lastMousePosition = m_mousePosition; // Set last position after update.

		// Call application draw.
		if (!Draw()) { std::cout << "Error occured in application draw!" << std::endl; return false; }

		UpdateWindow(); // Update the window.
		m_isRunning = !glfwWindowShouldClose(s_glfwStuff.window); // Check if application should keep updating.
	}
	
	// Call application shutdown.
	if (!Shutdown()) { std::cout << "Error occured whilst shutting down application!" << std::endl; return false; }

	// Destroy GLFW.
	glfwDestroyWindow(s_glfwStuff.window);
	glfwTerminate();

	return true;
}

void Application::ClearScreen()
{
	// Hopefully self explanatory.
	glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_windowWidth, m_windowHeight); // Set viewport to fill window.
}

void Application::UpdateWindow()
{
	// Swap back buffer and poll window messages.
	glfwSwapBuffers(s_glfwStuff.window);
	glfwPollEvents();
}

void *Application::GetRawWindowHandle()
{
	return (void*)s_glfwStuff.window; // Returns window, has to be casted back to GLFWwindow* because of the unnecessary abstraction I did.
}

void WindowResizeCallback(GLFWwindow *window, int width, int height)
{
	// Set new window size and update viewport.
	Application::Instance()->SetWindowWidth(width);
	Application::Instance()->SetWindowHeight(height);
	glViewport(0, 0, width, height);
}

void MousePositionCallback(GLFWwindow *window, double x, double y)
{
	// Gets the mouse position.
	glm::vec2 position = glm::vec2((float)x, (float)y);
	Application::Instance()->SetMousePosition(position);
}
