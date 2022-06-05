#include <map>
#include <NuakeRenderer/Math.h>

class InputManager
{
private:
	bool m_MouseButtons[5];
	std::map<int, bool> m_Keys;
public:
	bool IsKeyPressed(int keycode);
	bool IsKeyDown(int keycode);
	bool IsKeyReleased(int keycode);

	void HideMouse();
	void ShowMouse();
	bool IsMouseHidden();
	bool IsMouseButtonPressed(int button);
	bool IsMouseButtonDown(int button);
	bool IsMouseButtonReleased(int button);

	float GetMouseX();
	float GetMouseY();
	Vector2 GetMousePosition();

	bool Init();
	void Update();

	static InputManager& Get();
};