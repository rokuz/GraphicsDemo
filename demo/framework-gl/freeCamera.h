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

#ifndef __FREE_CAMERA_H__
#define __FREE_CAMERA_H__

#include "camera.h"

namespace framework
{

class FreeCamera : public Camera
{
public:
	FreeCamera();
	virtual ~FreeCamera(){}

	void initWithPositionDirection(int width, int height, const vector3& from, const vector3& to);
	
	void onKeyButton(int key, int scancode, bool pressed);
	void onMouseButton(double xpos, double ypos, int button, bool pressed);
	void onMouseMove(double xpos, double ypos);

	void update(double elapsedTime);

private:
	bool m_moveForward;
	bool m_moveBackward;
	bool m_moveLeft;
	bool m_moveRight;
	bool m_rotationMode;
	float m_speed;
	vector2 m_lastMousePosition;
	vector2 m_currentMousePosition;
	vector2 m_angles;
};

}

#endif // __FREE_CAMERA_H__