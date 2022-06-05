#include "Application.h"
#include <Dependencies/imgui/imgui.cpp>
#include <exception>
#include "IO/Logger.h"

void Application::Update(float ts)
{
	mRaytracer->scene.mCam->Update(ts);
}

NuakeRenderer::Window* Application::mWindow;

void Application::Draw(Vector2 size)
{
	mRaytracer->DrawTexture();

	SetupDockingFirstTime();

	if (ImGui::Begin("inspector"))
	{
		if (ImGui::Button("Reload Shader"))
		{
			Logger::Get().Clear();
			mRaytracer->ReloadCompute();
		}
		float availWidth = (ImGui::GetContentRegionAvail().x / 3.0) - 9.0;
		ImGui::Text("Camera Position");
		ImGui::PushItemWidth(availWidth);
		std::string camID = "##cam";
		ImGui::DragFloat(("x" + camID).c_str(), &mRaytracer->scene.mCam->data->Position.x, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(availWidth);
		ImGui::DragFloat(("y" + camID).c_str(), &mRaytracer->scene.mCam->data->Position.y, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(availWidth);
		ImGui::DragFloat(("z" + camID).c_str(), &mRaytracer->scene.mCam->data->Position.z, 0.01f);
		ImGui::PopItemWidth();

		ImGui::Text("Camera Target");
		ImGui::PushItemWidth(availWidth);
		camID = "##camTarget";
		ImGui::DragFloat(("x" + camID).c_str(), &mRaytracer->scene.mCam->data->LookAt.x, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(availWidth);
		ImGui::DragFloat(("y" + camID).c_str(), &mRaytracer->scene.mCam->data->LookAt.y, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(availWidth);
		ImGui::DragFloat(("z" + camID).c_str(), &mRaytracer->scene.mCam->data->LookAt.z, 0.01f);
		ImGui::PopItemWidth();
		
		ImGui::Separator();

		int i = 0;
		for (auto& s : mRaytracer->scene.Spheres)
		{
			std::string id = "##" + std::to_string(i);

			float availWidth = (ImGui::GetContentRegionAvail().x / 3.0) - 9.0;
			ImGui::PushItemWidth(availWidth);
			ImGui::DragFloat(("x" + id).c_str(), &s.Position.x, 0.01f); 
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(availWidth);
			ImGui::DragFloat(("y" + id).c_str(), &s.Position.y, 0.01f); 
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(availWidth);
			ImGui::DragFloat(("z" + id).c_str(), &s.Position.z, 0.01f); 
			ImGui::PopItemWidth();
			
			ImGui::DragFloat(("radius" + id).c_str(), &s.Radius, 0.01f, 0.0f);
			ImGui::DragInt(("material" + id).c_str(), &s.Material, 1, 0, 3);
			
			ImGui::ColorEdit3(("Abledo" + id).c_str(), &s.Albedo.x);

			ImGui::DragFloat(("Fuzz" + id).c_str(), &s.Fuzz);
			ImGui::DragFloat(("Refraction" + id).c_str(), &s.Refraction, 0.001f);
			i++;

			ImGui::Separator();
		}
		
	}
	ImGui::End();
	if (ImGui::Begin("viewport"))
	{
		ImVec2 pos = ImGui::GetCursorPos();
		ImVec2 availSize = ImGui::GetContentRegionAvail();
		mRaytracer->SetViewportSize({ availSize.x, availSize.y });
		ImGui::Image((void*)mRaytracer->GetFBTexture()->GetTextureID(), availSize);
	
		// text shadow
		auto logs = Logger::Get().GetLogs();
		ImGui::SetCursorPos(pos + ImVec2(1, 1));
		for (auto& l : logs)
		{
			ImGui::TextColored(ImVec4(0, 0, 0, 1), l.c_str());
		}
		
		ImGui::SetCursorPos(pos);
		for (auto& l : logs)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), l.c_str());
		}
	}
	ImGui::End();
}

void Application::SetupDockingFirstTime()
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.0f));
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	auto& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::Begin("Dockspace", nullptr, window_flags);
	{
		ImGui::PopStyleVar(3);

		ImGuiIO const& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("Dockspace");

			isDockingInitialized = ImGui::DockBuilderGetNode(dockspace_id) == NULL;
			ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), dockspace_flags);

			if (isDockingInitialized)
			{
				ImGuiID dock_main_id = dockspace_id;
				ImGuiID dock_id_prop_opp = 0;
				ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, &dock_id_prop_opp, &dock_main_id);
				ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

				ImGui::DockBuilderDockWindow("inspector", dock_id_prop);
				ImGui::DockBuilderDockWindow("viewport", dock_main_id);
				ImGui::DockBuilderFinish(dockspace_id);
			}
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "CTRL+N"))
					//OpenImage();

				ImGui::Separator();

				if (ImGui::MenuItem("Export Image", "CTRL+E"))
					//SaveProject();


				ImGui::Separator();

				if (ImGui::MenuItem("Exit", "CTRL+Q"))
				{
					

					std::terminate();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
}
