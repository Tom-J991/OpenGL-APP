#pragma once

#include "Common.h"

class Application
{
public:
	explicit Application(const char *title, unsigned int width, unsigned int height, bool fullscreen = false);
	virtual ~Application();

	bool Run();

	static Application *Instance() { return s_instance; }

	const char *GetWindowTitle() const { return m_windowTitle; }
	unsigned int GetWindowWidth() const { return m_windowWidth; }
	unsigned int GetWindowHeight() const { return m_windowHeight; }
	void SetWindowWidth(const unsigned int width) { m_windowWidth = width; }
	void SetWindowHeight(const unsigned int height) { m_windowHeight = height; }

	bool IsFullscreen() { return m_isFullscreen; }

	glm::vec4 GetBackgroundColor() const { return m_backgroundColor; }
	void SetBackgroundColor(const glm::vec4 backgroundColor) { m_backgroundColor = backgroundColor; }

	glm::vec2 GetMousePosition() const { return m_mousePosition; }
	glm::vec2 GetMouseDelta() { return (m_mousePosition - m_lastMousePosition); }
	void SetMousePosition(const glm::vec2 position) { m_mousePosition = position; }

	bool IsRunning() const { return m_isRunning; }
	void Quit() { m_isRunning = false; }

	void *GetRawWindowHandle();

protected:
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;
	virtual bool Update(float dt) = 0;
	virtual bool Draw() = 0;

protected:
	static Application *s_instance;

	unsigned int m_windowWidth, m_windowHeight;
	const char *m_windowTitle;
	bool m_isFullscreen; // Not used atm.

	glm::vec4 m_backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	glm::vec2 m_mousePosition;
	glm::vec2 m_lastMousePosition;

private:
	bool m_isRunning = false;

};
