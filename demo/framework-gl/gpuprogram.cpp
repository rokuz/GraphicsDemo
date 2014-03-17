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

#include "utils.h"

namespace framework
{

GpuProgram::GpuProgram() : m_isLoaded(false), m_program(0)
{
	for (size_t i = 0; i < MAX_UNIFORMS; i++)
	{
		m_uniforms[i] = -1;
	}
}

GpuProgram::~GpuProgram()
{
	destroy();
}

void GpuProgram::destroy()
{
	if (m_isLoaded)
	{
		if (m_program)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}

		m_isLoaded = false;
	}
}

bool GpuProgram::initWithVFShaders(const std::string& vsFileName, const std::string& fsFileName)
{
	destroy();

	GLuint vertShader, fragShader;

	m_program = glCreateProgram();

	if (!compileShader(&vertShader, GL_VERTEX_SHADER, vsFileName.c_str()))
	{
		utils::Logger::toLogWithFormat("Failed to compile vertex shader '%s'.\n", vsFileName.c_str());
		return false;
	}

	if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fsFileName.c_str()))
	{
		utils::Logger::toLogWithFormat("GpuProgram error: Failed to compile fragment shader '%s'.\n", fsFileName.c_str());
		return false;
	}

	glAttachShader(m_program, vertShader);
	glAttachShader(m_program, fragShader);

	if (!linkProgram(m_program) || !validateProgram(m_program))
	{
		utils::Logger::toLog("GpuProgram error: Failed to link/validate program.\n");

		if (vertShader)
		{
			glDeleteShader(vertShader);
			vertShader = 0;
		}
		if (fragShader)
		{
			glDeleteShader(fragShader);
			fragShader = 0;
		}
		if (m_program)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}

		return false;
	}

	if (vertShader)
	{
		glDetachShader(m_program, vertShader);
		glDeleteShader(vertShader);
	}
	if (fragShader)
	{
		glDetachShader(m_program, fragShader);
		glDeleteShader(fragShader);
	}

	m_isLoaded = true;

	if (m_isLoaded) initDestroyable();
	return m_isLoaded;
}

bool GpuProgram::initWithVGFShaders(const std::string& vsFileName, const std::string& gsFileName, const std::string& fsFileName)
{
	destroy();

	GLuint vertShader, fragShader, geomShader;

	m_program = glCreateProgram();

	if (!compileShader(&vertShader, GL_VERTEX_SHADER, vsFileName.c_str()))
	{
		utils::Logger::toLogWithFormat("GpuProgram error: Failed to compile vertex shader '%s'.\n", vsFileName.c_str());
		return false;
	}

	if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fsFileName.c_str()))
	{
		utils::Logger::toLogWithFormat("GpuProgram error: Failed to compile fragment shader '%s'.\n", fsFileName.c_str());
		return false;
	}

	if (!compileShader(&geomShader, GL_GEOMETRY_SHADER, gsFileName.c_str()))
	{
		utils::Logger::toLogWithFormat("GpuProgram error: Failed to compile geometry shader '%s'.\n", gsFileName.c_str());
		return false;
	}

	glAttachShader(m_program, vertShader);
	glAttachShader(m_program, fragShader);
	glAttachShader(m_program, geomShader);

	if (!linkProgram(m_program) || !validateProgram(m_program))
	{
		utils::Logger::toLog("Failed to link/validate program.\n");

		if (vertShader)
		{
			glDeleteShader(vertShader);
			vertShader = 0;
		}
		if (fragShader)
		{
			glDeleteShader(fragShader);
			fragShader = 0;
		}
		if (geomShader)
		{
			glDeleteShader(geomShader);
			geomShader = 0;
		}
		if (m_program)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}

		return false;
	}

	if (vertShader)
	{
		glDetachShader(m_program, vertShader);
		glDeleteShader(vertShader);
	}
	if (fragShader)
	{
		glDetachShader(m_program, fragShader);
		glDeleteShader(fragShader);
	}
	if (geomShader)
	{
		glDetachShader(m_program, geomShader);
		glDeleteShader(geomShader);
	}

	m_isLoaded = true;

	if (m_isLoaded) initDestroyable();
	return m_isLoaded;
}

bool GpuProgram::compileShader( GLuint* shader, GLenum type, const std::string& fileName )
{
	std::string sourceStr;
	utils::Utils::readFileToString(fileName, sourceStr);
	if (sourceStr.empty())
	{
		utils::Logger::toLogWithFormat("GpuProgram error: Failed to load shader '%s'.\n", fileName.c_str());
		return false;
	}

	GLint status;
	const GLchar *source = sourceStr.c_str();

	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);

#if defined(_DEBUG)
	GLint logLength;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = new GLchar[logLength];
		glGetShaderInfoLog(*shader, logLength, &logLength, log);
		utils::Logger::toLogWithFormat("Shader compile log:\n%s\n", log);
		delete [] log;
	}
#endif

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

#if defined(_DEBUG)
	GLint logLength;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = new GLchar[logLength];
		glGetProgramInfoLog(prog, logLength, &logLength, log);
		utils::Logger::toLogWithFormat("Shader link log:\n%s\n", log);
		delete [] log;
	}
#endif

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
		utils::Logger::toLogWithFormat("Shader validate log:\n%s\n", log);
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
	if (!m_isLoaded) return false;

	glUseProgram(m_program);

	return true;
}

float* GpuProgram::convert(const vector4& v)
{
	static float arr[4];
	arr[0] = v.x; arr[1] = v.y; arr[2] = v.z; arr[3] = v.w;
	return arr;
}

float* GpuProgram::convert(const vector3& v)
{
	static float arr[3];
	arr[0] = v.x; arr[1] = v.y; arr[2] = v.z;
	return arr;
}

float* GpuProgram::convert(const quaternion& q)
{
	static float arr[4];
	arr[0] = q.x; arr[1] = q.y; arr[2] = q.z; arr[3] = q.w;
	return arr;
}

}