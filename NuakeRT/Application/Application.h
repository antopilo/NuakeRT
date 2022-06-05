#pragma once

#include <imgui.h>
#include "Raytracer.h"
#include <memory>
#include <NuakeRenderer/Window.h>

class Application
{
public:
	Application()
	{
		mRaytracer = std::make_shared<Raytracer>();
	};
	~Application() = default;

	static Application& Get()
	{
		static Application app;
		return app;
	}

	void Update(float ts);
	void Draw(Vector2 size);

	static void SetWindow(Window* window)
	{
		mWindow = window;
	}
	
	NuakeRenderer::Window* GetWindow() const
	{
		return mWindow;
	}
private:
	static NuakeRenderer::Window* mWindow;
	std::shared_ptr<Raytracer> mRaytracer;
	void SetupDockingFirstTime();
	bool isDockingInitialized = false;
	
};