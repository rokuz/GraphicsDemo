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

#include "utils.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <stdio.h>
#include <algorithm>
#endif

namespace utils
{

bool Utils::exists(const std::string& fileName)
{
#ifdef WIN32
	DWORD dwAttrib = GetFileAttributesA(fileName.c_str());
	if (dwAttrib != INVALID_FILE_ATTRIBUTES) return true;
#endif
	return false;
}

bool Utils::readFileToString( const std::string& fileName, std::string& out )
{
	FILE* fp = 0;
	size_t filesize = 0;

	fp = fopen(fileName.c_str(), "rb");

	if (!fp) return false;

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	out.resize(filesize + 1);
	fread(&out[0], 1, filesize, fp);
	out[filesize] = 0;

	fclose(fp);

	return true;
}

float* Utils::convert(const vector4& v)
{
	static float arr[4];
	arr[0] = v.x; arr[1] = v.y; arr[2] = v.z; arr[3] = v.w;
	return arr;
}

float* Utils::convert(const vector3& v)
{
	static float arr[3];
	arr[0] = v.x; arr[1] = v.y; arr[2] = v.z;
	return arr;
}

float* Utils::convert(const quaternion& q)
{
	static float arr[4];
	arr[0] = q.x; arr[1] = q.y; arr[2] = q.z; arr[3] = q.w;
	return arr;
}

}