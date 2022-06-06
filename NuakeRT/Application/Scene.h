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
	float ts = 0.f;
	int SphereAmount = 0;
	float Aperture = 0.0;
	float focusDistance = 1.0;
};

class Camera
{
public:
	int frameId = 0;
	std::shared_ptr<SSBO> mSSBO;
	std::shared_ptr<SSBO> mCummulative;
	Vector3 Direction = Vector3(0, 0, -1);
	CamData* data;
	
	Camera(float fov, Vector3 position, Vector3 lookAt)
	{
		data = new CamData{};
		data->FOV = fov;
		data->Position = position;
		data->LookAt = lookAt;

		
		mCummulative = std::make_shared<SSBO>(sizeof(int));
		mSSBO = std::make_shared<SSBO>(sizeof(CamData));
	}

	void Update(float dt);
};

struct SceneData
{
	int sphereAmount = 0;
	float padding1;
	float padding2;
	float padding3;
	void* spheres;
};

class Scene
{
public:
	bool dirty = false;
	std::vector<Sphere> Spheres;
	SceneData* sceneData;
	std::shared_ptr<Camera> mCam;
	std::shared_ptr<SSBO> mUBO;
	
	Scene();

	void Bind();

	
};