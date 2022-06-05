#pragma once
#include <vector>
#include <NuakeRenderer/NuakeRenderer.h>
#include <NuakeRenderer/Math.h>
#include <memory>
#include <NuakeRenderer/SSBO.h>

#include <Dependencies/glfw/include/GLFW/glfw3.h>

using namespace NuakeRenderer;

struct Sphere
{
	Vector3 Position;
	float Radius;
	int Material;
	float padding;
	float padding2;
	float padding3;

	Vector3 Albedo;
	float Fuzz;
	float Refraction;
	float padding4;
	float padding6;
	float padding7;
};

struct CamData
{
	Vector3 Position = Vector3(-1, 1, -1);
	float FOV = 90.f;
	Vector3 LookAt = Vector3(0, 0, -1);
};

class Camera
{
public:
	std::shared_ptr<SSBO> mSSBO;
	Vector3 Direction = Vector3(0, 0, -1);
	CamData* data;
	
	Camera(float fov, Vector3 position, Vector3 lookAt)
	{
		data = new CamData{};
		data->FOV = fov;
		data->Position = position;
		data->LookAt = lookAt;

		

		mSSBO = std::make_shared<SSBO>(sizeof(CamData));
	}

	void Update(float dt);
};

class Scene
{
public:
	
	std::vector<Sphere> Spheres;
	std::shared_ptr<Camera> mCam;
	std::shared_ptr<SSBO> mUBO;
	
	Scene();

	void Bind();

	
};