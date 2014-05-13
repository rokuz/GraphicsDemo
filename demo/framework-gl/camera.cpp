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
#include "camera.h"

namespace framework
{

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::init( int width, int height )
{
	m_camera.SetPerspective(65.0f, (float)width/(float)height, 0.1f, 1000.0f);
	m_orientation.ident();
	m_position = vector3(0, 0, 0);
}

const matrix44& Camera::getView()
{
	m_view.set_translation(m_position);
	vector3 dir = vector3(0, 0, 1);
	dir = m_orientation.rotate(dir);
	m_view.lookatRh(m_position + dir, vector3(0, 1, 0));
	m_view.invert_simple();

	return m_view;
}

void Camera::updateResolution( int width, int height )
{
	m_camera.SetAspectRatio((float)width/(float)height);
}

const matrix44& Camera::getProjection()
{
	return m_camera.GetProjection();
}

}