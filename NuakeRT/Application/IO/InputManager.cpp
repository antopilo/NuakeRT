#include "InputManager.h"
#include <NuakeRenderer/Window.h>
#include <GLFW/glfw3.h>
#include "../Application.h"

#pragma region Keys
// Only true if the key is currently being pressed
bool InputManager::IsKeyDown(int keycode)
{
	auto window = Application::Get().GetWindow()->GetHandle();
	int state = glfwGetKey(window, keycode);
	bool result = state == GLFW_PRESS;

	m_Keys[keycode] = state;

	return result;
}

// Only true if the key is pressed for the first frame. no repeat.
bool InputManager::IsKeyPressed(int keycode)
{
	auto window = Application::Get().GetWindow()->GetHandle();
	int state = glfwGetKey(window, keycode);
	bool result = state == GLFW_PRESS;

	// First time pressed?
	if (m_Keys.find(keycode) == m_Keys.end() || m_Keys[keycode] == true)
	{
		if (result)
			m_Keys[keycode] = true;

		return result;
	}

	return false;
}


bool InputManager::IsKeyReleased(int keycode)
{
	auto window = Application::Get().GetWindow()->GetHandle();
	int state = glfwGetKey(window, keycode);
	bool result = state == GLFW_RELEASE;

	// First time pressed?
	if (m_Keys.find(keycode) == m_Keys.end())
		return result;

	if (result && m_Keys[keycode] == true)
	{
		return true;
	}


	return false;
}
#pragma endregion

#pragma region Mouse

// Visibility
void InputManager::HideMouse()
{
	auto window = Application::Get().GetWindow()->GetHandle();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool InputManager::IsMouseHidden()
{
	auto window = Application::Get().GetWindow()->GetHandle();
	return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

void InputManager::ShowMouse()
{
	auto window = Application::Get().GetWindow()->GetHandle();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}


// Action
bool InputManager::IsMouseButtonDown(int button)
{
	auto window = Application::Get().GetWindow()->GetHandle();
	auto state = glfwGetMouseButton(window, button);

	return state == GLFW_PRESS;
}

bool InputManager::IsMouseButtonPressed(int button)
{
	auto window = Application::Get().GetWindow()->GetHandle();
	auto state = glfwGetMouseButton(window, button);

	if (m_MouseButtons[button] == false && state == GLFW_PRESS)
	{
		m_MouseButtons[button] = true;
		return true;
	}

	return false;
}

bool InputManager::IsMouseButtonReleased(int button)
{
	auto window = Application::Get().GetWindow()->GetHandle();
	auto state = glfwGetMouseButton(window, button);

	return state == GLFW_RELEASE && m_MouseButtons[button] == true;
}

// Position
float InputManager::GetMouseX()
{
	auto window = Application::Get().GetWindow()->GetHandle();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return (float)xpos;
}

float InputManager::GetMouseY()
{
	auto window = Application::Get().GetWindow()->GetHandle();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return (float)ypos;
}

Vector2 InputManager::GetMousePosition()
{
	auto window = Application::Get().GetWindow()->GetHandle();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return Vector2(xpos, ypos);
}
#pragma endregion

bool InputManager::Init()
{
	//auto window = Application::Get().GetWindow()->GetNative();
	//glfwSetKeyCallback(window, Input::HandleInputCallback);
	return false;
}

void InputManager::Update()
{
	// Reset all input to false.
	for (auto& k : m_Keys)
	{
		if (!IsKeyDown(k.first))
			k.second = false;
	}

	for (int i = 0; i < 5; i++)
	{
		if (!IsMouseButtonDown(i))
			m_MouseButtons[i] = false;
	}
}

InputManager& InputManager::Get()
{
	static InputManager manager;
	return manager;
}

