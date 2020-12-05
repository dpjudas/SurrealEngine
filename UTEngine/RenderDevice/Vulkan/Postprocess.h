#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "PixelBuffer.h"
#include "Math/vec.h"

enum class UniformType
{
	Int,
	UInt,
	Float,
	Vec2,
	Vec3,
	Vec4,
	IVec2,
	IVec3,
	IVec4,
	UVec2,
	UVec3,
	UVec4,
	Mat4
};

class UniformFieldDesc
{
public:
	UniformFieldDesc() { }
	UniformFieldDesc(const char *name, UniformType type, std::size_t offset) : name(name), type(type), offset(offset) { }

	const char *name;
	UniformType type;
	std::size_t offset;
};

enum class PPBlendMode
{
	none,
	additive,
	alpha
};

struct PPViewport
{
	int x, y, width, height;
};

class PPTexture;
class PPShader;
class Postprocess;

enum class PPFilterMode { Nearest, Linear };
enum class PPWrapMode { Clamp, Repeat };
enum class PPTextureType { CurrentPipelineTexture, NextPipelineTexture, PPTexture, SwapChain };

class PPTextureInput
{
public:
	PPFilterMode filter = PPFilterMode::Nearest;
	PPWrapMode wrap = PPWrapMode::Clamp;
	PPTextureType type = PPTextureType::CurrentPipelineTexture;
	PPTexture *texture = nullptr;
};

class PPOutput
{
public:
	PPTextureType type = PPTextureType::NextPipelineTexture;
	PPTexture *texture = nullptr;
};

class PPUniforms
{
public:
	PPUniforms()
	{
	}

	PPUniforms(const PPUniforms &src)
	{
		data = src.data;
	}

	~PPUniforms()
	{
		clear();
	}

	PPUniforms &operator=(const PPUniforms &src)
	{
		data = src.data;
		return *this;
	}

	void clear()
	{
		data.clear();
	}

	template<typename T>
	void set(const T &v)
	{
		if (data.size() != (int)sizeof(T))
		{
			data.resize(sizeof(T));
			memcpy(data.data(), &v, data.size());
		}
	}

	std::vector<uint8_t> data;
};

class PPRenderState
{
public:
	virtual ~PPRenderState() = default;

	virtual void draw() = 0;

	void clear()
	{
		shader = nullptr;
		textures = std::vector<PPTextureInput>();
		uniforms = PPUniforms();
		viewport = PPViewport();
		blendMode = PPBlendMode();
		output = PPOutput();
	}

	void setInputTexture(int index, PPTexture *texture, PPFilterMode filter = PPFilterMode::Nearest, PPWrapMode wrap = PPWrapMode::Clamp)
	{
		if ((int)textures.size() < index + 1)
			textures.resize(index + 1);
		auto &tex = textures[index];
		tex.filter = filter;
		tex.wrap = wrap;
		tex.type = PPTextureType::PPTexture;
		tex.texture = texture;
	}

	void setInputCurrent(int index, PPFilterMode filter = PPFilterMode::Nearest, PPWrapMode wrap = PPWrapMode::Clamp)
	{
		setInputSpecialType(index, PPTextureType::CurrentPipelineTexture, filter, wrap);
	}

	void setInputSpecialType(int index, PPTextureType type, PPFilterMode filter = PPFilterMode::Nearest, PPWrapMode wrap = PPWrapMode::Clamp)
	{
		if ((int)textures.size() < index + 1)
			textures.resize(index + 1);
		auto &tex = textures[index];
		tex.filter = filter;
		tex.wrap = wrap;
		tex.type = type;
		tex.texture = nullptr;
	}

	void setOutputTexture(PPTexture *texture)
	{
		output.type = PPTextureType::PPTexture;
		output.texture = texture;
	}

	void setOutputCurrent()
	{
		output.type = PPTextureType::CurrentPipelineTexture;
		output.texture = nullptr;
	}

	void setOutputNext()
	{
		output.type = PPTextureType::NextPipelineTexture;
		output.texture = nullptr;
	}

	void setOutputSwapChain()
	{
		output.type = PPTextureType::SwapChain;
		output.texture = nullptr;
	}

	void setNoBlend()
	{
		blendMode = PPBlendMode::none;
	}

	void setAdditiveBlend()
	{
		blendMode = PPBlendMode::additive;
	}

	void setAlphaBlend()
	{
		blendMode = PPBlendMode::alpha;
	}

	Postprocess* model = nullptr;
	PPShader *shader;
	std::vector<PPTextureInput> textures;
	PPUniforms uniforms;
	PPViewport viewport;
	PPBlendMode blendMode;
	PPOutput output;
};

class PPResource
{
public:
	PPResource()
	{
		next = first;
		first = this;
		if (next) next->prev = this;
	}

	PPResource(const PPResource &)
	{
		next = first;
		first = this;
		if (next) next->prev = this;
	}

	virtual ~PPResource()
	{
		if (next) next->prev = prev;
		if (prev) prev->next = next;
		else first = next;
	}

	PPResource &operator=(const PPResource &other)
	{
		return *this;
	}

	static void resetAll()
	{
		for (PPResource *cur = first; cur; cur = cur->next)
			cur->resetBackend();
	}

	virtual void resetBackend() = 0;

private:
	static PPResource *first;
	PPResource *prev = nullptr;
	PPResource *next = nullptr;
};

class PPTextureBackend
{
public:
	virtual ~PPTextureBackend() = default;
};

class PPTexture : public PPResource
{
public:
	PPTexture() = default;
	PPTexture(int width, int height, PixelFormat format, std::shared_ptr<void> data = {}) : width(width), height(height), format(format), data(data) { }

	void resetBackend() override { backend.reset(); }

	int width;
	int height;
	PixelFormat format;
	std::shared_ptr<void> data;

	std::unique_ptr<PPTextureBackend> backend;
};

class PPShaderBackend
{
public:
	virtual ~PPShaderBackend() = default;
};

class PPShader : public PPResource
{
public:
	PPShader() = default;
	PPShader(const std::string &fragment, const std::string &defines, const std::vector<UniformFieldDesc> &uniforms, int version = 330) : fragmentShader(fragment), defines(defines), uniforms(uniforms), version(version) { }

	void resetBackend() override { backend.reset(); }

	std::string fragmentShader;
	std::string defines;
	std::vector<UniformFieldDesc> uniforms;
	int version = 330;

	std::unique_ptr<PPShaderBackend> backend;
};

/////////////////////////////////////////////////////////////////////////////

struct PresentUniforms
{
	float invGamma;
	float padding1;
	float padding2;
	float padding3;

	static std::vector<UniformFieldDesc> desc()
	{
		return
		{
			{ "InvGamma", UniformType::Float, offsetof(PresentUniforms, invGamma) }
		};
	}
};

class PPPresent
{
public:
	PPPresent();

	PPTexture dither;
	float gamma = 1.0f;

	PPShader present = { "shaders/Present.frag", "", PresentUniforms::desc() };
};

/////////////////////////////////////////////////////////////////////////////

class Postprocess
{
public:
	PPPresent present;
};
