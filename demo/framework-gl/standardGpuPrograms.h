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

#ifndef __STANDARD_GPU_PROGRAMS_H__
#define __STANDARD_GPU_PROGRAMS_H__

namespace framework
{

DECLARE_UNIFORMS_BEGIN(StandardUniforms)
	MODELVIEWPROJECTION_MATRIX,
	COLOR,
	ORIENTATION,
	POSITION,
	DEPTH_MAP
DECLARE_UNIFORMS_END()

class StandardGpuPrograms
{
public:

	static bool init();

	static std::shared_ptr<GpuProgram> getLineRenderer();
	static std::shared_ptr<GpuProgram> getArrowRenderer();
	static std::shared_ptr<GpuProgram> getDepthBufferCopying();

private:
	static std::shared_ptr<GpuProgram> m_lineRenderer;
	static std::shared_ptr<GpuProgram> m_arrowRenderer;
	static std::shared_ptr<GpuProgram> m_copyDepthBuffer;
};

}

#define STD_UF framework::UniformBase<framework::StandardUniforms>::Uniform

#endif // __STANDARD_GPU_PROGRAMS_H__