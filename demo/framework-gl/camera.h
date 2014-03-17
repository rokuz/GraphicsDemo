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

#ifndef __CAMERA_H__
#define __CAMERA_H__
#ifdef WIN32
    #pragma once
#endif

#include "ncamera2.h"
#include "quaternion.h"

namespace framework
{

class Camera
{
public:
	Camera();
	virtual ~Camera();

	void init(int width, int height);
	void updateResolution(int width, int height);

	const matrix44& getView();
	const matrix44& getProjection();
	nCamera2& getInternalCamera() { return m_camera; }

	void setPosition(const vector3& position)
	{
		m_position = position;
	}

	const vector3& getPosition() const
	{
		return m_position;
	}

	void setOrientation(const quaternion& orientation)
	{
		m_orientation = orientation;
	}

	const quaternion& getOrientation() const
	{
		return m_orientation;
	}

protected:
	nCamera2 m_camera;
	quaternion m_orientation;
	vector3 m_position;

	matrix44 m_view;
};

}

#endif