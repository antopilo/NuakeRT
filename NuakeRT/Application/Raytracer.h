#pragma once

#include <memory>

#include <NuakeRenderer/VertexBuffer.h>
#include <NuakeRenderer/VertexArray.h>
#include <NuakeRenderer/Texture.h>
#include <NuakeRenderer/FrameBuffer.h>

#include "Scene.h"

class Raytracer
{
public:
	Scene scene;
	
	Vector2 RenderSize = Vector2(1920, 1080);
	Raytracer();
	~Raytracer() = default;

	std::shared_ptr<NuakeRenderer::Texture> GetTexture() const
	{
		return mTexture;
	}
	
	void DrawTexture();

	NuakeRenderer::Texture* GetFBTexture() const
	{
		return mFramebuffer->GetTextureAttachment(NuakeRenderer::TextureAttachment::COLOR0);
	}
	
	void ReloadCompute();

	void SetViewportSize(Vector2 size);
private:
	void LoadShaders();
	
	std::shared_ptr<NuakeRenderer::Framebuffer> mFramebuffer;
	std::shared_ptr<NuakeRenderer::Texture> mTexture;
	std::shared_ptr<NuakeRenderer::VertexBuffer> mVertexBuffer;
	std::shared_ptr<NuakeRenderer::VertexArray> mVertexArray;
};