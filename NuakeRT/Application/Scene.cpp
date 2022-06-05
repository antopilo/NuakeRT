#include "Scene.h"
#include "IO/InputManager.h"
Scene::Scene()
{
	Sphere s1 = Sphere();
	s1.Position = Vector3(0, 0, 2);
	s1.Radius = 0.5f;
	s1.Material = 0;
	s1.Fuzz = 0.0f;
	s1.Albedo = Vector3(0.7f, 0.2f, 0.1f);
	s1.Refraction = 0.0f;

	Sphere s2 = Sphere();
	s2.Position = Vector3(0, 0, 3);
	s2.Radius = 0.5f;
	s2.Material = 0;
	s2.Fuzz = 0.0f;
	s2.Albedo = Vector3(0.2f, 0.2f, 0.8f);	
	s2.Refraction = 0.0f;

	Sphere s3 = Sphere();
	s3.Position = Vector3(0, 0, -6);
	s3.Radius = 2.5f;
	s3.Material = 0;
	s3.Fuzz = 0.0f;
	s3.Albedo = Vector3(0.1f, 0.8f, 0.1f);	
	s3.Refraction = 0.0f;
	
	sceneData = new SceneData();
	sceneData->spheres = Spheres.data();
	Spheres = { s1, s2, s3};
	sceneData->sphereAmount = Spheres.size();
	mUBO = std::make_shared<SSBO>((sizeof(Sphere) * 32));

	mCam = std::make_shared<Camera>(90.f, Vector3(0, 0, 0), Vector3(0, 0, 1));
}

void Scene::Bind()
{
	mCam->data->SphereAmount = Spheres.size();
	mCam->mSSBO->SetData(mCam->data, sizeof(CamData));
	mCam->mSSBO->Bind(1);
	mCam->mCummulative->Bind(3);

	size_t sizeSphere = sizeof(Sphere) * Spheres.size();
	size_t uboSize = sizeof(SceneData) + sizeSphere;
	mUBO->SetData(Spheres.data(), sizeSphere);

	mUBO->Bind(2);
}

float mouseLastX = 0.f;
float mouseLastY = 0.f;
float yaw = 0.f;
float pitch = 0.f;
bool firstMouse = true;



#include <imgui.h>
void Camera::Update(float dt)
{
	data->ts += dt;
	if (!InputManager::Get().IsMouseButtonDown(GLFW_MOUSE_BUTTON_2))
	{
		firstMouse = true;
		InputManager::Get().ShowMouse();
		frameId++;
		mCummulative->SetData(&frameId, sizeof(int));
		return;
	}
	
	frameId = 1;

	InputManager::Get().HideMouse();

	Vector3 right = glm::normalize(glm::cross(Direction, Vector3(0, 1, 0)));
	
	if (InputManager::Get().IsKeyDown(GLFW_KEY_A))
	{
		data->Position -= right * dt;
	}
	
	if (InputManager::Get().IsKeyDown(GLFW_KEY_D))
	{
		data->Position += right * dt;
	}

	if (InputManager::Get().IsKeyDown(GLFW_KEY_W))
	{
		data->Position += glm::normalize(Direction) * dt;
	}

	if (InputManager::Get().IsKeyDown(GLFW_KEY_S))
	{
		data->Position -= glm::normalize(Direction) * dt;
	}


	if (InputManager::Get().IsKeyDown(GLFW_KEY_SPACE))
	{
		data->Position.y += 1.0 * dt;
	}

	if (InputManager::Get().IsKeyDown(GLFW_KEY_LEFT_SHIFT))
	{
		data->Position.y -= 1.0 * dt;
	}

	// mouse
	float x = InputManager::Get().GetMouseX();
	float y = InputManager::Get().GetMouseY();
	if (firstMouse)
	{
		mouseLastX = x;
		mouseLastY = y;
		firstMouse = false;
	}
	float diffx = x - mouseLastX;
	float diffy = mouseLastY - y;
	mouseLastX = x;
	mouseLastY = y;
	//
	const float sensitivity = 0.1f;
	diffx *= sensitivity;
	diffy *= sensitivity;
	//
	yaw += diffx;
	pitch += diffy;
	//
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	Direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	Direction.y = sin(glm::radians(pitch));
	Direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	//Direction = Vector3(0, 0, 1);
	Direction = glm::normalize(Direction);
	data->LookAt = data->Position + Direction;
	

	mCummulative->SetData(&frameId, sizeof(int));
	
}
