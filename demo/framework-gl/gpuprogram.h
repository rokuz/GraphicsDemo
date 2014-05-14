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

#ifndef __GPUPROGRAM_H__
#define __GPUPROGRAM_H__

namespace framework
{

const int MAX_UNIFORMS = 256;
const int MAX_BOUND_TEXTURES = 32;

template<typename UniformType>
class UniformBase
{
public:
	enum Uniform
	{
		FAKE_UNIFORM = 0
	};
};

#define DECLARE_UNIFORMS_BEGIN(UniformType) class UniformType{}; \
template<> class framework::UniformBase<UniformType> { public: enum Uniform {
#define DECLARE_UNIFORMS_END() }; }; 

enum ShaderType
{
	VERTEX_SHADER = 0,
	GEOMETRY_SHADER,
	FRAGMENT_SHADER,

	SHADERS_COUNT
};

class Texture;

class GpuProgram : public Destroyable
{
	friend class Application;

public:
	GpuProgram();
	virtual ~GpuProgram();

	void addShader(const std::string& fileName);
	bool init();
	bool isValid() const;

	template<typename UniformType>
	void bindUniform(typename UniformBase<UniformType>::Uniform uniform, const std::string& name)
	{
		if (!isValid()) return;
		if (uniform >= MAX_UNIFORMS) return;

		m_uniforms[uniform] = glGetUniformLocation(m_program, name.c_str());
		if (m_uniforms[uniform] < 0)
		{
			utils::Logger::toLogWithFormat("Error: Uniform '%s' has not been found to bind.\n", name.c_str());
		}
	}

	template<typename UniformType>
	void bindUniformBuffer(typename UniformBase<UniformType>::Uniform uniform, const std::string& name)
	{
		if (!isValid()) return;
		if (uniform >= MAX_UNIFORMS) return;

		m_uniforms[uniform] = glGetUniformBlockIndex(m_program, name.c_str());
		if (m_uniforms[uniform] < 0)
		{
			utils::Logger::toLogWithFormat("Error: Uniform buffer '%s' has not been found to bind.\n", name.c_str());
		}
	}

	template<typename UniformType>
	GLint getUniform(typename UniformBase<UniformType>::Uniform uniform)
	{
		return m_uniforms[uniform];
	}

	template<typename UniformType>
	GLint getUniformBuffer(typename UniformBase<UniformType>::Uniform uniform)
	{
		return m_uniforms[uniform];
	}

	template<typename UniformType>
	void setUniformBuffer(typename UniformBase<UniformType>::Uniform uniform, std::shared_ptr<UniformBuffer> buffer, int index)
	{
		if (!buffer) return;

		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;

		buffer->bind(index);
		glUniformBlockBinding(m_program, uf, index);
	}

	template<typename UniformType>
	void setVector(typename UniformBase<UniformType>::Uniform uniform, const vector2& vec)
	{
		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;
		glUniform2fv(uf, 1, utils::Utils::convert(vec));
	}

	template<typename UniformType>
	void setVector(typename UniformBase<UniformType>::Uniform uniform, const vector3& vec)
	{
		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;
		glUniform3fv(uf, 1, utils::Utils::convert(vec));
	}

	template<typename UniformType>
	void setVector(typename UniformBase<UniformType>::Uniform uniform, const vector4& vec)
	{
		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;
		glUniform4fv(uf, 1, utils::Utils::convert(vec));
	}

	template<typename UniformType>
	void setVector(typename UniformBase<UniformType>::Uniform uniform, const quaternion& quat)
	{
		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;
		glUniform4fv(uf, 1, utils::Utils::convert(quat));
	}

	template<typename UniformType>
	void setMatrix(typename UniformBase<UniformType>::Uniform uniform, const matrix44& mat)
	{
		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;
		glUniformMatrix4fv(uf, 1, false, &mat.m[0][0]);
	}

	template<typename UniformType>
	void setTexture(typename UniformBase<UniformType>::Uniform uniform, std::shared_ptr<Texture> texture)
	{
		if (!texture) return;

		GLint uf = getUniformBuffer<UniformType>(uniform);
		if (uf < 0) return;
		if (m_freeTextureSlot >= MAX_BOUND_TEXTURES) return;

		glActiveTexture(GL_TEXTURE0 + m_freeTextureSlot);
		texture->bind();
		glUniform1i(uf, m_freeTextureSlot);

		m_freeTextureSlot++;
	}

	bool use();

private:
	GLuint m_program;
	GLint m_uniforms[MAX_UNIFORMS];
	std::vector<std::string> m_shaders;
	int m_freeTextureSlot;

	bool compileShader(GLuint* shader, GLenum type, const std::string& fileName);
	bool linkProgram(GLuint prog);
	bool validateProgram(GLuint prog);
	std::string getProgramName();

	virtual void destroy();
};

}

#endif //__GPUPROGRAM_H__