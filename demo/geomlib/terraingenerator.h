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

#ifndef __TERRAIN_GENERATOR_H__
#define __TERRAIN_GENERATOR_H__

#include "geometrygenerator.h"

namespace geom
{

struct TerrainGenerationInfo
{
	vector3 size;
	vector2 uvSize;
	std::vector<unsigned char> heightmap;
	size_t heightmapWidth;
	size_t heightmapHeight;

	TerrainGenerationInfo() : size(10.0f, 10.0f, 2.0f), uvSize(1.0f, 1.0f){}
};

class TerrainGenerator : public GeometryGenerator
{
public:
	TerrainGenerator(){}
	virtual ~TerrainGenerator(){}

	void setTerrainGenerationInfo(const TerrainGenerationInfo& info);
	virtual Data generate();

private:
	TerrainGenerationInfo m_info;
	int m_terrainWidth;
	int m_terrainHeight;

	vector3 getPoint(int x, int y);
	void calculateTangentSpace(int x, int y, vector3& normal, vector3& tangent, vector3& binormal);
};

}

#endif