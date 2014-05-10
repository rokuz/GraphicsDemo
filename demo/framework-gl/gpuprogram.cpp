/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "gpuprogram.h"
#include "application.h"

namespace framework
{

namespace
{

int getTypeByExt(const std::list<std::string>& exts)
{
#define FIND_EXT(str) std::find(exts.cbegin(), exts.cend(), str) != exts.cend()
	if (FIND_EXT("vsh") || FIND_EXT("vs")) return VERTEX_SHADER;
	if (FIND_EXT("gsh") || FIND_EXT("gs")) return GEOMETRY_SHADER;
	if (FIND_EXT("fsh") || FIND_EXT("fs")) return FRAGMENT_SHADER;
#undef FIND_EXT
	return -1;
}

GLint getOGLShaderType(ShaderType type)
{
	switch (type)
	{
	case framework::VERTEX_SHADER:
		return GL_VERTEX_SHADER;

	case framework::GEOMETRY_SHADER:
		return GL_GEOMETRY_SHADER;

	case framework::FRAGMENT_SHADER:
		return GL_FRAGMENT_SHADER;
	}
	return -1;
}

}

GpuProgram::GpuProgram() : m_program(0)
{
	for (size_t i = 0; i < MAX_UNIFORMS; i++)
		m_uniforms[i] = -1;

	m_shaders.resize(SHADERS_COUNT);
}

GpuProgram::~GpuProgram()
{
	destroy();
}

void GpuProgram::addShader(const std::string& fileName)
{
	int shaderType = getTypeByExt(utils::Utils::getExtentions(fileName));
	if (shaderType < 0)
	{
		utils::Logger::toLogWithFormat("Error: could not add shader '%s'. The reason: shader type is undefined.\n", fileName.c_str());
		return;
	}
	m_shaders[shaderType] = fileName;
}

bool GpuProgram::init()
{
	destroy();
	m_program = glCreateProgram();

	std::list<GLuint> compiledShaders;
	for (size_t shaderIndex = 0; shaderIndex < SHADERS_COUNT; shaderIndex++)
	{
		if (m_shaders[shaderIndex].empty()) continue;

		GLuint shader;
		if (!compileShader(&shader, getOGLShaderType((ShaderType)shaderIndex), m_shaders[shaderIndex]))
		{
			destroy();
			utils::Logger::toLogWithFormat("Error: failed to compile shader '%s'.\n", m_shaders[shaderIndex].c_str());
			return false;
		}

		glAttachShader(m_program, shader);
		compiledShaders.push_back(shader);
	}

	if (compiledShaders.empty())
	{
		destroy();
		return false;
	}

	if (!linkProgram(m_program))
	{
		utils::Logger::toLog("Error: failed to link program.\n");

		for (auto it = compiledShaders.begin(); it != compiledShaders.end(); ++it)
		{
			glDeleteShader(*it);
		}
		destroy();
		return false;
	}

	if (!validateProgram(m_program))
	{
		utils::Logger::toLog("Error: failed to validate program.\n");

		for (auto it = compiledShaders.begin(); it != compiledShaders.end(); ++it)
		{
			glDeleteShader(*it);
		}
		destroy();
		return false;
	}

	for (auto it = compiledShaders.begin(); it != compiledShaders.end(); ++it)
	{
		glDetachShader(m_program, *it);
		glDeleteShader(*it);
	}

	initDestroyable();
	return true;
}

bool GpuProgram::isValid() const
{
	return m_program != 0;
}

void GpuProgram::destroy()
{
	if (m_program)
	{
		glDeleteProgram(m_program);
		m_program = 0;
	}
}

bool GpuProgram::compileShader( GLuint* shader, GLenum type, const std::string& fileName )
{
	std::string sourceStr;
	utils::Utils::readFileToString(fileName, sourceStr);
	if (sourceStr.empty())
	{
		utils::Logger::toLogWithFormat("Error: failed to load shader '%s'.\n", fileName.c_str());
		return false;
	}

	GLint status;
	const GLchar *source = sourceStr.c_str();

	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);

	if (Application::instance()->isDebugEnabled())
	{
		GLint logLength;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar *log = new GLchar[logLength];
			glGetShaderInfoLog(*shader, logLength, &logLength, log);
			utils::Logger::toLogWithFormat("Shader compilation log:\n%s", log);
			delete[] log;
		}
	}

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		glDeleteShader(*shader);
		return false;
	}

	return true;
}

bool GpuProgram::linkProgram( GLuint prog )
{
	GLint status;
	glLinkProgram(prog);

	if (Application::instance()->isDebugEnabled())
	{
		GLint logLength;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar *log = new GLchar[logLength];
			glGetProgramInfoLog(prog, logLength, &logLength, log);
			utils::Logger::toLogWithFormat("Gpu program linkage log:\n%s", log);
			delete[] log;
		}
	}

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		return false;
	}

	return true;
}

bool GpuProgram::validateProgram( GLuint prog )
{
	GLint logLength, status;

	glValidateProgram(prog);
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = new GLchar[logLength];
		glGetProgramInfoLog(prog, logLength, &logLength, log);
		utils::Logger::toLogWithFormat("Gpu program validation log:\n%s", log);
		delete [] log;
	}

	glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
	if (status == 0)
	{
		return false;
	}

	return true;
}

bool GpuProgram::use()
{
	if (!isValid()) return false;

	glUseProgram(m_program);
	return true;
}

}