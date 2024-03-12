#include "Camera.h"

#include "Application.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Camera::Camera()
{ }
Camera::~Camera()
{ }

void Camera::Update(float dt)
{
	GLFWwindow *window = (GLFWwindow*)Application::Instance()->GetRawWindowHandle();

	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);

	glm::vec3 forward(glm::cos(phiR) * glm::cos(thetaR), glm::sin(phiR), glm::cos(phiR) * glm::sin(thetaR));
	glm::vec3 right(-glm::sin(thetaR), 0.0f, glm::cos(thetaR));
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	const float movingSpeed = 2.0f;
	if (glfwGetKey(window, GLFW_KEY_SPACE)) m_position += up * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) m_position -= up * movingSpeed * dt;

	if (glfwGetKey(window, GLFW_KEY_W)) m_position += forward * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_S)) m_position -= forward * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_A)) m_position -= right * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_D)) m_position += right * movingSpeed * dt;

	const float turningSpeed = 0.5f;
	glm::vec2 mouseDelta = Application::Instance()->GetMouseDelta();
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		m_theta += turningSpeed * mouseDelta.x;
		m_phi -= turningSpeed * mouseDelta.y;
		if (m_phi > 75.0f) m_phi = 75.0f;
		else if (m_phi < -75.0f) m_phi = -75.0f;
	}
}

glm::mat4 Camera::GetViewMatrix()
{
	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);
	glm::vec3 forward(glm::cos(phiR) * glm::cos(thetaR), glm::sin(phiR), glm::cos(phiR) * glm::sin(thetaR));
	return glm::lookAt(m_position, m_position + forward, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::GetProjectionMatrix(float fov, float width, float height)
{
	return glm::perspective(glm::radians(fov), width / height, 0.1f, 1000.0f);
}
