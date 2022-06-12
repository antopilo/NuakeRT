#include "Raytracer.h"
#include <NuakeRenderer/ShaderRegistry.h>
#include "IO/FileSystem.h"
#include <NuakeRenderer/Vendors/glm/glm/ext/matrix_clip_space.hpp>
#include <NuakeRenderer/Vendors/glm/glm/ext/matrix_transform.hpp>
#include <memory>
#include <iostream>
#include "IO/Logger.h"
#include <algorithm>

using namespace NuakeRenderer;
struct Vertex
{
	Vector3 position;
	Vector2 UV;
};

Raytracer::Raytracer()
{
	// Load shaders.
	LoadShaders();
	// Create buffers

	mFramebuffer = std::make_shared<Framebuffer>(RenderSize);
	mFramebuffer->SetTextureAttachment(new Texture({}, RenderSize), TextureAttachment::COLOR0);

	const std::vector<Vertex> vertices = 
	{
		{ { 1.f,  1.f, 0.f }, {1.f, 0.f} },
		{ { 1.f,  -1.f, 0.f }, {1.f, 1.f} },
		{ { -1.f,  1.f, 0.f }, {0.f, 0.f} },
		{ { 1.f,  -1.f, 0.f }, {1.f, 1.f} },
		{ { -1.f,  -1.f, 0.f }, {0.f, 1.f} },
		{ { -1.f,  1.f, 0.f }, {0.f, 0.f} },
	};

	mVertexArray = std::make_shared<VertexArray>();
	mVertexArray->Bind();
	mVertexBuffer = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));

	auto vbl = NuakeRenderer::VertexBufferLayout();
	vbl.Push<float>(3); // Position
	vbl.Push<float>(2); // UV

	mVertexArray->AddBuffer(*mVertexBuffer, vbl);
	mVertexArray->Unbind();

	// Create RT texture
	
	auto flags = TextureFlags
	{
		PixelFormat::RGBA32F,
		PixelDataType::FLOAT,

		SamplerFilter::NEAREST,
		SamplerFilter::NEAREST,
		SamplerWrapping::CLAMP_TO_EDGE,
	};
	
	mTexture = std::make_shared<Texture>(flags, RenderSize);
}

void Raytracer::Resize(const Vector2 size)
{
	if (RenderSize != size)
	{
		mTexture->Resize(size);
		RenderSize = size;
	}
}

void Raytracer::LoadShaders()
{
	std::string vert = FileSystem::ReadFile("Resources/quad.vert.glsl");
	std::string frag = FileSystem::ReadFile("Resources/quad.frag.glsl");
	auto shader = new Shader(vert, frag);
	ShaderRegistry::Set("quad", shader);

	std::string rt = FileSystem::ReadFile("Resources/rt.comp.glsl");
	shader = new Shader(rt);
	ShaderRegistry::Set("rt", shader);
}

void Raytracer::ReloadCompute()
{
	std::string rt = FileSystem::ReadFile("Resources/rt.comp.glsl");
	auto old = ShaderRegistry::Get("rt");
	delete old;
	auto newShader = new Shader(rt);
	ShaderRegistry::Set("rt", newShader);

	if (newShader->GetError() != "")
		Logger::Get().Log(newShader->GetError());
}

void Raytracer::SetViewportSize(Vector2 size)
{
	mFramebuffer->QueueResize(size);
}
#include <tgmath.h>
#include "IO/stb_image_write.h"
void Raytracer::DrawTexture()
{
	auto shader = ShaderRegistry::Get("rt");

	if (scene.dirty)
	{
		scene.mCam->frameId = 0;
		scene.dirty = false;
	}

	shader->Bind();
	glBindImageTexture(0, mTexture->GetTextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	scene.Bind();
	

	glDispatchCompute(ceil(mTexture->GetSize().x / 8), ceil(mTexture->GetSize().y / 8), 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	mFramebuffer->Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT);

		float ratio = RenderSize.x / RenderSize.y;
		auto size = mFramebuffer->GetSize();
		float aspectRatio = (size.x / size.y);

		Matrix4 projc;
		// then black bar top.
		if (aspectRatio < ratio)
		{
			float mult = ratio / aspectRatio;
			projc = glm::ortho(-1.0f * aspectRatio * mult, 1.f * aspectRatio * mult, -1.f * mult, 1.f * mult);
		}
		else
			projc = glm::ortho(-1.f * aspectRatio, 1.f * aspectRatio, -1.f , 1.f);
		
		float scale = size.y / RenderSize.y;

		Matrix4 model = Matrix4(1.f);
		model = glm::scale(model, { ratio, 1.f, 1.f });
		
		auto quadShader = ShaderRegistry::Get("quad");
		quadShader->Bind();
		
		mTexture->Bind(0);
		quadShader->SetUniform("u_Texture", 0);
		quadShader->SetUniform("u_Model", model);
		quadShader->SetUniform("u_Projection", projc);
		
		mVertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	mFramebuffer->Unbind();

	
	if (ImGui::Begin("DEBUG"))
	{
		ImGui::Image((void*)mTexture->GetTextureID(), ImGui::GetContentRegionAvail());
	}

	ImGui::End();
}

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void Raytracer::SaveToTexture(const std::string& path)
{
	const int w = RenderSize.x;
	const int h = RenderSize.y;
	const int channel = 4;
	float* data = new float[w * h * channel];
	std::vector<char> dataB = std::vector<char>();

	glBindTexture(GL_TEXTURE_2D, mTexture->GetTextureID());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);

	int i = 0;
	for (int b = 0; b < w * h * channel; b++)
	{
		dataB.push_back(std::min((int)(data[b] * 255), 255));
		i++;
	}
	
	stbi_flip_vertically_on_write(true);
	if (stbi_write_png(path.c_str(), w, h, channel, dataB.data(), w * channel))
	{
		std::cout << "Error! " << std::endl;
	}
	free(data);
}