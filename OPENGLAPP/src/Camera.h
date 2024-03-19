#pragma once

#include "Common.h"

class Camera
{
public:
	Camera();
	~Camera();

	void DoFlyCam(float dt);

	glm::vec3 GetPosition() const { return m_position; }
	void SetPosition(const glm::vec3 position) { m_position = position; }
	void SetRotation(const glm::vec2 rotation) { m_theta = rotation.y; m_phi = rotation.x; }

	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix(float fov, float width, float height);

private:
	glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
	float m_theta = 0.0f; // Yaw
	float m_phi = 0.0f; // Pitch

};
