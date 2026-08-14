#pragma once

#include "gl_load/gl_load.h"
#include <utility>
#include <memory>

class GLTexture
{
public:
	GLTexture()
	{
		glGenTextures(1, &Handle);
	}

	~GLTexture()
	{
		glDeleteTextures(1, &Handle);
		Handle = 0;
	}

	GLTexture(const GLTexture& other) = delete;
	GLTexture& operator=(const GLTexture& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_TEXTURE;
};

typedef GLTexture GLTexture1D;
typedef GLTexture GLTexture2D;
typedef GLTexture GLTexture3D;

class GLRenderbuffer
{
public:
	GLRenderbuffer()
	{
		glGenRenderbuffers(1, &Handle);
	}

	~GLRenderbuffer()
	{
		glDeleteRenderbuffers(1, &Handle);
		Handle = 0;
	}

	GLRenderbuffer(const GLRenderbuffer& other) = delete;
	GLRenderbuffer& operator=(const GLRenderbuffer& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_RENDERBUFFER;
};

class GLFramebuffer
{
public:
	GLFramebuffer()
	{
		glGenFramebuffers(1, &Handle);
	}

	~GLFramebuffer()
	{
		glDeleteFramebuffers(1, &Handle);
		Handle = 0;
	}

	GLFramebuffer(const GLFramebuffer& other) = delete;
	GLFramebuffer& operator=(const GLFramebuffer& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_FRAMEBUFFER;
};

class GLShader
{
public:
	GLShader(GLuint type)
	{
		Handle = glCreateShader(type);
	}

	~GLShader()
	{
		glDeleteShader(Handle);
		Handle = 0;
	}

	GLShader(const GLShader& other) = delete;
	GLShader& operator=(const GLShader& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_SHADER;
};

class GLVertexShader : public GLShader
{
public:
	GLVertexShader() : GLShader(GL_VERTEX_SHADER) {}
};

class GLFragmentShader : public GLShader
{
public:
	GLFragmentShader() : GLShader(GL_FRAGMENT_SHADER) {}
};

class GLProgram
{
public:
	GLProgram()
	{
		Handle = glCreateProgram();
	}

	~GLProgram()
	{
		glDeleteProgram(Handle);
		Handle = 0;
	}

	GLProgram(const GLProgram& other) = delete;
	GLProgram& operator=(const GLProgram& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_PROGRAM;
};

class GLBuffer
{
public:
	GLBuffer()
	{
		glGenBuffers(1, &Handle);
	}

	~GLBuffer()
	{
		glDeleteBuffers(1, &Handle);
		Handle = 0;
	}

	GLBuffer(const GLBuffer& other) = delete;
	GLBuffer& operator=(const GLBuffer& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_BUFFER;
};

class GLSampler
{
public:
	GLSampler()
	{
		glGenSamplers(1, &Handle);
	}

	~GLSampler()
	{
		glDeleteSamplers(1, &Handle);
		Handle = 0;
	}

	GLSampler(const GLSampler& other) = delete;
	GLSampler& operator=(const GLSampler& other) = delete;

	GLuint Handle = 0;
	static const int Type = GL_SAMPLER;
};

class GLInputLayout
{
public:
};

class GLBlendDesc
{
public:
	bool IndependentBlendEnable;
	struct Target
	{
		bool RenderTargetWriteMask; // glColorMask
		bool BlendEnable;
		GLenum BlendOp;
		GLenum BlendOpAlpha;
		GLenum SrcBlend;
		GLenum SrcBlendAlpha;
		GLenum DestBlend;
		GLenum DestBlendAlpha;
	} RenderTarget[2];
};

class GLBlendState
{
public:
	GLBlendState(const GLBlendDesc& desc) : desc(desc) {}
	GLBlendDesc desc = {};
};

class GLDepthStencilDesc
{
public:
	bool DepthEnable;
	GLenum DepthFunc;
	bool DepthWriteMask;
};

class GLDepthStencilState
{
public:
	GLDepthStencilState(const GLDepthStencilDesc& desc) : desc(desc) {}
	GLDepthStencilDesc desc = {};
};

class GLRasterizerDesc
{
public:
	bool CullEnable;
	bool FrontCounterClockwise;
	bool DepthClipEnable;
	bool MultisampleEnable;
};

class GLRasterizerState
{
public:
	GLRasterizerState(const GLRasterizerDesc& desc) : desc(desc) {}
	GLRasterizerDesc desc = {};
};
