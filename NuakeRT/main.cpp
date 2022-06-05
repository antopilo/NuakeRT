#include <NuakeRenderer/NuakeRenderer.h>
#include <NuakeRenderer/Window.h>
#include "Application/Application.h"
#include <Dependencies/glfw/include/GLFW/glfw3.h>

void main()
{
	using namespace NuakeRenderer;

	auto window = Window("NuakeRT");

	// ImGui Flags and theme.
	ApplyNuakeImGuiTheme();
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Create the app.
	auto& app = Application();
	app.SetWindow(&window);

	float m_Time = 0.f;
	float m_LastFrameTime = 0.f;
	
	while (!window.ShouldClose())
	{
		m_Time = (float)glfwGetTime();
		float timestep = m_Time - m_LastFrameTime;
		m_LastFrameTime = m_Time;

		Begin();
		BeginImGuiFrame();

		// Draw.
		app.Get().Update(timestep);
		app.Get().Draw(window.GetWindowSize());

		EndImGuiFrame();
		window.SwapBuffers();
	}
}