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

#include "standardGpuPrograms.h"

namespace framework
{

std::shared_ptr<GpuProgram> StandardGpuPrograms::m_lineRenderer;
std::shared_ptr<GpuProgram> StandardGpuPrograms::m_arrowRenderer;

bool StandardGpuPrograms::init()
{
	bool result = true;

	std::string shadersPath = "data/shaders/gl/win32/standard/";

	m_lineRenderer.reset(new GpuProgram());
	result &= m_lineRenderer->initWithVFShaders(shadersPath + "line.vsh.glsl", shadersPath + "line.fsh.glsl");
	if (!result) return false;
	m_lineRenderer->bindUniform<StandardUniforms>(STD_UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
	m_lineRenderer->bindUniform<StandardUniforms>(STD_UF::COLOR, "color");

	m_arrowRenderer.reset(new GpuProgram());
	result &= m_arrowRenderer->initWithVGFShaders(shadersPath + "arrow.vsh.glsl", shadersPath + "arrow.gsh.glsl", shadersPath + "arrow.fsh.glsl");
	if (!result) return false;
	m_arrowRenderer->bindUniform<StandardUniforms>(STD_UF::MODELVIEWPROJECTION_MATRIX, "viewProjectionMatrix");
	m_arrowRenderer->bindUniform<StandardUniforms>(STD_UF::POSITION, "position");
	m_arrowRenderer->bindUniform<StandardUniforms>(STD_UF::ORIENTATION, "orientation");
	m_arrowRenderer->bindUniform<StandardUniforms>(STD_UF::COLOR, "color");

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

}