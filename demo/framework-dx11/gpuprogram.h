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

#pragma once

namespace framework
{

const int MAX_UNIFORMS = 16;

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

class GpuProgram : public Destroyable
{
	friend class Application;

public:
	GpuProgram();
	virtual ~GpuProgram();

	int getId() const { return m_id; }

	void addShader(const std::string& fileName, const std::string& mainFunc = "");
	bool init(bool autoInputLayout = false);
	bool isValid() const;

	template<typename UniformType>
	void bindUniform(typename UniformBase<UniformType>::Uniform uniform, const std::string& name)
	{
		if (!isValid()) return;
		if (uniform >= MAX_UNIFORMS)
		{
			utils::Logger::toLog("Error: uniform index is out of bound.\n");
			return;
		}

		bindUniformByIndex((int)uniform, name);
	}

	template<typename UniformType>
	void setUniform(typename UniformBase<UniformType>::Uniform uniform, std::shared_ptr<UniformBuffer> buffer)
	{
		setUniformByIndex((int)uniform, std::move(buffer));
	}

	template<typename UniformType>
	void setUniform(typename UniformBase<UniformType>::Uniform uniform, std::shared_ptr<Texture> texture)
	{
		setUniformByIndex((int)uniform, std::move(texture));
	}

	template<typename UniformType>
	void setUniform(typename UniformBase<UniformType>::Uniform uniform, std::shared_ptr<RenderTarget> renderTarget, unsigned int rtIndex = 0)
	{
		setUniformByIndex((int)uniform, std::move(renderTarget), rtIndex);
	}

	template<typename UniformType>
	void setUniform(typename UniformBase<UniformType>::Uniform uniform, std::shared_ptr<Sampler> sampler)
	{
		setUniformByIndex((int)uniform, std::move(sampler));
	}

	int bindInputLayoutInfo(const std::vector<D3D11_INPUT_ELEMENT_DESC>& info);
	bool use();
	void applyInputLayout(int inputLayoutIndex);

private:
	struct ShaderResourceData
	{
		std::string name;
		unsigned int sizeInBytes;
		unsigned int bindingPoint;
		unsigned int bindingCount;
		ShaderResourceData() : sizeInBytes(0), bindingPoint(0), bindingCount(0) {}
	};
	struct ShaderData
	{
		std::string filename;
		std::string mainFunction;
		ID3DBlob* compiledShader;
		std::vector<std::shared_ptr<ShaderResourceData> > m_resources;
		ShaderData() : compiledShader(0){}
	};
	std::vector<ShaderData> m_data;
	std::weak_ptr<ShaderResourceData> m_uniforms[SHADERS_COUNT][MAX_UNIFORMS];

	int m_id;
	ID3D11VertexShader* m_vertexShader;
	ID3D11HullShader* m_hullShader;
	ID3D11DomainShader* m_domainShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11ComputeShader* m_computeShader;

	struct InputLayoutData
	{
		ID3D11InputLayout* inputLayout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutInfo;
		InputLayoutData() : inputLayout(0) {}
		~InputLayoutData();
		void clear();
	};
	InputLayoutData m_inputLayout;
	std::list<std::string> m_inputLayoutStringsPool;
	std::vector<InputLayoutData> m_inputLayoutInfoBase;

	ID3DBlob* compileShader(ShaderType shaderType, const std::string& filename, const std::string& function, const D3D_SHADER_MACRO* defines = 0);
	bool createShaderByType(const Device& device, ShaderType shaderType, ID3DBlob* compiledShader);
	bool reflectShaders(const Device& device, bool autoInputLayout);
	void bindUniformByIndex(int index, const std::string& name);
	void setUniformByIndex(int index, std::shared_ptr<UniformBuffer> buffer);
	void setUniformByIndex(int index, std::shared_ptr<Texture> texture);
	void setUniformByIndex(int index, std::shared_ptr<Sampler> sampler);
	void setUniformByIndex(int index, std::shared_ptr<RenderTarget> renderTarget, unsigned int rtIndex);
	void setUniformByIndex(int index, ID3D11ShaderResourceView* view);
	const char* stringInPool(const char* str);
	bool compareInputLayoutInfos(const std::vector<D3D11_INPUT_ELEMENT_DESC>& info1, const std::vector<D3D11_INPUT_ELEMENT_DESC>& info2);

	virtual void destroy();

	static int generateId();
};

}