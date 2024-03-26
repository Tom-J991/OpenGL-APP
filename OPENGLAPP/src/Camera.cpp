#include "Camera.h"

#include "Application.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Camera::Camera()
{ }
Camera::~Camera()
{ }

void Camera::DoFlyCam(float dt)
{
	GLFWwindow *window = (GLFWwindow*)Application::GetInstance()->GetRawWindowHandle();

	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);

	glm::quat pitch = glm::angleAxis(-phiR, glm::vec3(1, 0, 0));
	glm::quat yaw = glm::angleAxis(thetaR, glm::vec3(0, 1, 0));

	glm::quat rotation = glm::normalize(pitch * yaw);

	// Get camera directions. // x forward
	glm::vec3 forward(glm::cos(phiR) * glm::cos(thetaR), glm::sin(phiR), glm::cos(phiR) * glm::sin(thetaR));
	glm::vec3 right(-glm::sin(thetaR), 0.0f, glm::cos(thetaR));
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	// Get camera directions from quaternion. -z forward
	//glm::vec3 forward = glm::rotate(glm::inverse(rotation), glm::vec3(0, 0, -1));
	//glm::vec3 right = glm::rotate(glm::inverse(rotation), glm::vec3(1, 0, 0));
	//glm::vec3 up = glm::rotate(glm::inverse(rotation), glm::vec3(0, 1, 0));

	// Speed up.
	float movingSpeed = 2.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) movingSpeed *= 4.0f;

	// Fly up and down.
	if (glfwGetKey(window, GLFW_KEY_SPACE)) m_position.y += movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) m_position.y -= movingSpeed * dt;

	// WASD movement.
	if (glfwGetKey(window, GLFW_KEY_W)) m_position += forward * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_S)) m_position -= forward * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_A)) m_position -= right * movingSpeed * dt;
	if (glfwGetKey(window, GLFW_KEY_D)) m_position += right * movingSpeed * dt;

	// Look around.
	const float turningSpeed = 0.5f;
	glm::vec2 mouseDelta = Application::GetInstance()->GetMouseDelta();
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		m_theta += turningSpeed * mouseDelta.x;
		m_phi -= turningSpeed * mouseDelta.y;
		if (m_phi > 75.0f) m_phi = 75.0f;
		else if (m_phi < -75.0f) m_phi = -75.0f;
	}
}

glm::mat4 Camera::GetTransformMatrix()
{
	float thetaR = glm::radians(m_theta + 90.0f);
	float phiR = glm::radians(m_phi);

	return glm::translate(glm::mat4(1.0f), m_position) *
		glm::rotate(glm::mat4(1.0f), thetaR, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), phiR, glm::vec3(1, 0, 0));
}

glm::mat4 Camera::GetViewMatrix()
{
	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);
	glm::vec3 forward(glm::cos(phiR) * glm::cos(thetaR), glm::sin(phiR), glm::cos(phiR) * glm::sin(thetaR));
	return glm::lookAt(m_position, m_position + forward, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::GetViewMatrixFromQuaternion()
{
	float thetaR = glm::radians(m_theta + 90.0f);
	float phiR = glm::radians(m_phi);
	glm::quat pitch = glm::angleAxis(-phiR, glm::vec3(1, 0, 0));
	glm::quat yaw = glm::angleAxis(thetaR, glm::vec3(0, 1, 0));

	glm::quat rotation = glm::normalize(pitch * yaw);

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_position);

	return glm::toMat4(rotation) * translation;
}

glm::mat4 Camera::GetProjectionMatrix(float fov, float width, float height)
{
	return glm::perspective(glm::radians(fov), width / height, 0.1f, 1000.0f);
}
