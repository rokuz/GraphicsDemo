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

#include "stdafx.h"
#include "standardgpuprograms.h"

namespace framework
{

std::shared_ptr<GpuProgram> StandardGpuPrograms::m_lineRenderer;
std::shared_ptr<GpuProgram> StandardGpuPrograms::m_arrowRenderer;
std::shared_ptr<GpuProgram> StandardGpuPrograms::m_skyboxRenderer;

std::shared_ptr<UniformBuffer> StandardGpuPrograms::m_skyboxDataBuffer;

bool StandardGpuPrograms::init()
{
	bool result = true;

	m_lineRenderer.reset(new GpuProgram());
	m_lineRenderer->addShader(STANDARD_SHADERS_PATH + "line.vsh.hlsl");
	m_lineRenderer->addShader(STANDARD_SHADERS_PATH + "line.psh.hlsl");
	result &= m_lineRenderer->init(true);
	if (!result) return false;
	m_lineRenderer->bindUniform<StandardUniforms>(STD_UF::LINE_RENDERER_DATA, "lineData");

	m_arrowRenderer.reset(new GpuProgram());
	m_arrowRenderer->addShader(STANDARD_SHADERS_PATH + "arrow.vsh.hlsl");
	m_arrowRenderer->addShader(STANDARD_SHADERS_PATH + "arrow.psh.hlsl");
	m_arrowRenderer->addShader(STANDARD_SHADERS_PATH + "arrow.gsh.hlsl");
	result &= m_arrowRenderer->init(true);
	if (!result) return false;
	m_arrowRenderer->bindUniform<StandardUniforms>(STD_UF::ARROW_RENDERER_DATA, "arrowData");

	m_skyboxRenderer.reset(new GpuProgram());
	m_skyboxRenderer->addShader(STANDARD_SHADERS_PATH + "skybox.vsh.hlsl");
	m_skyboxRenderer->addShader(STANDARD_SHADERS_PATH + "skybox.psh.hlsl");
	m_skyboxRenderer->addShader(STANDARD_SHADERS_PATH + "skybox.gsh.hlsl");
	result &= m_skyboxRenderer->init(true);
	if (!result) return false;
	m_skyboxRenderer->bindUniform<StandardUniforms>(STD_UF::SKYBOX_RENDERER_DATA, "skyboxData");
	m_skyboxRenderer->bindUniform<StandardUniforms>(STD_UF::SKYBOX_MAP, "skyboxMap");
	m_skyboxRenderer->bindUniform<StandardUniforms>(STD_UF::DEFAULT_SAMPLER, "defaultSampler");

	m_skyboxDataBuffer.reset(new framework::UniformBuffer());
	result &= m_skyboxDataBuffer->initDefaultConstant<SkyboxRendererData>();
	if (!result) return false;

	return result;
}

std::shared_ptr<GpuProgram> StandardGpuPrograms::getLineRenderer()
{
	return m_lineRenderer;
}

std::shared_ptr<GpuProgram> StandardGpuPrograms::getArrowRenderer()
{
	return m_arrowRenderer;
}

std::shared_ptr<GpuProgram> StandardGpuPrograms::getSkyboxRenderer()
{
	return m_skyboxRenderer;
}

std::shared_ptr<UniformBuffer> StandardGpuPrograms::getSkyboxDataBuffer()
{
	return m_skyboxDataBuffer;
}

}