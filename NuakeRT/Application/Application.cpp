#include "Application.h"
#include <Dependencies/imgui/imgui.cpp>
#include <exception>
#include "IO/Logger.h"
#include "IO/ImGuizmo.h"
#include "NuakeRenderer/Vendors/glm/glm/gtc/type_ptr.hpp"
#include "NuakeRenderer/Vendors/glm/glm/gtx/matrix_decompose.hpp"
void Application::Update(float ts)
{
	mRaytracer->scene.mCam->Update(ts);
}

NuakeRenderer::Window* Application::mWindow;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "IO/stb_image_write.h"
#include "IO/FileSystem.h"

Sphere* mSelectedSphere = nullptr;
void Application::Draw(Vector2 size)
{
	mRaytracer->DrawTexture();

	SetupDockingFirstTime();

	if (ImGui::Begin("Camera"))
	{
		// Resolution
		auto oldResolution = (glm::ivec2)mRaytracer->RenderSize;
		ImGui::InputInt2("Resolution", &oldResolution.x);
		mRaytracer->Resize(oldResolution);
		
		float oldFov = mRaytracer->scene.mCam->data->FOV;
		float oldAperture = mRaytracer->scene.mCam->data->Aperture;
		float oldFocus = mRaytracer->scene.mCam->data->focusDistance;
		ImGui::DragFloat("FOV", &mRaytracer->scene.mCam->data->FOV, 2.0, 0.1);
		ImGui::DragFloat("Aperture", &mRaytracer->scene.mCam->data->Aperture, 0.001f, 0.0f);
		ImGui::DragFloat("Focus Length", &mRaytracer->scene.mCam->data->focusDistance, 0.001f, 0.0f);

		if (mRaytracer->scene.mCam->data->FOV != oldFov)
			mRaytracer->scene.dirty = true;
		if (mRaytracer->scene.mCam->data->Aperture != oldAperture)
			mRaytracer->scene.dirty = true;
		if (mRaytracer->scene.mCam->data->focusDistance != oldFocus)
			mRaytracer->scene.dirty = true;
	}
	ImGui::End();

	if (ImGui::Begin("viewport"))
	{
		ImVec2 pos = ImGui::GetCursorPos();
		ImVec2 availSize = ImGui::GetContentRegionAvail();
		ImGuizmo::SetDrawlist();
	

		ImGuizmo::SetRect(0, 0, availSize.x, availSize.y);

		mRaytracer->SetViewportSize({ availSize.x, availSize.y });
		ImGui::Image((void*)mRaytracer->GetFBTexture()->GetTextureID(), availSize);


		auto cam = mRaytracer->scene.mCam;
		Matrix4 camproj = glm::perspective<float>(glm::radians(mRaytracer->scene.mCam->data->FOV), 16.0 / 9.0, 0.1f, 99999);
		Matrix4 camView = glm::lookAt(cam->data->Position, cam->data->LookAt, Vector3(0, 1, 0));


		Matrix4 model = Matrix4(1.0f);

		if (mSelectedSphere != nullptr)
		{
			model = glm::translate(model, mSelectedSphere->Position);
			ImGuizmo::Manipulate(glm::value_ptr(camView), glm::value_ptr(camproj), 
				ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, glm::value_ptr(model), 
				NULL, NULL);
			
			Vector3 newPos = Vector3();
			if (ImGuizmo::IsUsing())
			{
				mRaytracer->scene.dirty = true;
				Vector3 scale;
				glm::quat rotation;
				Vector3 translation = Vector3();
				Vector3 skew;
				Vector4 perspective;
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(translation), glm::value_ptr(scale), glm::value_ptr(rotation));
				
				mSelectedSphere->Position = translation;
			}
		}
		

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

	if (ImGui::Begin("Scene"))
	{
		if (ImGui::Button("Add Sphere"))
		{
			mRaytracer->scene.Spheres.push_back(Sphere());
			mRaytracer->scene.dirty = true;
		}
			

		int i = 0;
		if (ImGui::BeginChild("##Tree", ImGui::GetContentRegionAvail(), true))
		{
			for (auto& s : mRaytracer->scene.Spheres)
			{
				auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

				if(&s == mSelectedSphere)
					flags |= ImGuiTreeNodeFlags_Selected;
				
				bool opened = ImGui::TreeNodeEx(("Sphere " + std::to_string(i)).c_str(), flags);

				if (ImGui::IsItemClicked())
					mSelectedSphere = &s;

				i++;
			}
			
			ImGui::EndChild();
		}
	}
	ImGui::End();

	if (ImGui::Begin("inspector"))
	{
		if (ImGui::Button("Reload Shader"))
		{
			Logger::Get().Clear();
			mRaytracer->ReloadCompute();
		}

		if (ImGui::BeginChild("##Properties", ImGui::GetContentRegionAvail(), true))
		{
			if (mSelectedSphere == nullptr)
			{
				ImGui::Text("No sphere selected.");
				ImGui::EndChild();
				ImGui::End();
				return;
			}
			float availWidth = (ImGui::GetContentRegionAvail().x / 3.0) - 16.0;
			ImGui::PushItemWidth(availWidth);
			ImGui::DragFloat("x", &mSelectedSphere->Position.x, 0.01f);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(availWidth);
			ImGui::DragFloat("y", &mSelectedSphere->Position.y, 0.01f);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(availWidth);
			ImGui::DragFloat("z", &mSelectedSphere->Position.z, 0.01f);
			ImGui::PopItemWidth();

			float oldRadius = mSelectedSphere->Radius;
			ImGui::DragFloat("radius" , &mSelectedSphere->Radius, 0.01f, 0.0f);

			if (oldRadius != mSelectedSphere->Radius)
				mRaytracer->scene.dirty = true;

			int oldMaterial = mSelectedSphere->Material;
			ImGui::DragInt("material", &mSelectedSphere->Material, 1, 0, 3);

			if (oldMaterial != mSelectedSphere->Material)
				mRaytracer->scene.dirty = true;

			Vector3 oldColor = mSelectedSphere->Albedo;
			ImGui::ColorEdit3("Albedo" , &mSelectedSphere->Albedo.x);

			if (oldColor != mSelectedSphere->Albedo)
				mRaytracer->scene.dirty = true;

			float oldFuzz = mSelectedSphere->Fuzz;
			ImGui::DragFloat("Fuzz", &mSelectedSphere->Fuzz, 0.01f);

			if (oldFuzz != mSelectedSphere->Fuzz)
				mRaytracer->scene.dirty = true;

			float oldRefraction = mSelectedSphere->Refraction;
			ImGui::DragFloat("Refraction", &mSelectedSphere->Refraction, 0.001f);

			if (oldRefraction != mSelectedSphere->Refraction)
				mRaytracer->scene.dirty = true;
		}

		ImGui::EndChild();
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
				{
					std::string path = FileSystem::SaveImage();
					if (path != "")
					{
						int width = mRaytracer->RenderSize.x;
						int height = mRaytracer->RenderSize.y;
						
						mRaytracer->SaveToTexture(path);
					}
						
				}


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
