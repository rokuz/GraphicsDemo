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
#include "geomsaver.h"
#include "json/json.h"

namespace geom
{

bool GeomSaver::save(const Data& data, const std::string& filename)
{
	FILE* fp = 0;
	fp = fopen(filename.c_str(), "wb");
	if (!fp)
	{
		return false;
	}

	fwrite(&MAGIC_GEOM, sizeof(MAGIC_GEOM), 1, fp);

	float fbuf[3] = { data.getBoundingBox().vmin.x, data.getBoundingBox().vmin.y, data.getBoundingBox().vmin.z };
	fwrite(&fbuf, sizeof(fbuf), 1, fp);
	float fbuf2[3] = { data.getBoundingBox().vmax.x, data.getBoundingBox().vmax.y, data.getBoundingBox().vmax.z };
	fwrite(&fbuf2, sizeof(fbuf2), 1, fp);

	size_t componentsCount = data.getVertexComponentsCount();
	fwrite(&componentsCount, sizeof(componentsCount), 1, fp);

	size_t uvs = data.getAdditionalUVsCount();
	fwrite(&uvs, sizeof(uvs), 1, fp);

	size_t vertexSize = data.getVertexSize();
	fwrite(&vertexSize, sizeof(vertexSize), 1, fp);

	for (size_t c = 0; c < componentsCount; c++)
	{
		size_t vcs = data.getVertexComponentSize(c);
		fwrite(&vcs, sizeof(vcs), 1, fp);
		size_t vco = data.getVertexComponentOffset(c);
		fwrite(&vco, sizeof(vco), 1, fp);
	}

	size_t meshesCount = data.getMeshes().size();
	fwrite(&meshesCount, sizeof(meshesCount), 1, fp);
	for (size_t m = 0; m < meshesCount; m++)
	{
		fwrite(&data.getMeshes()[m].offsetInIB, sizeof(data.getMeshes()[m].offsetInIB), 1, fp);
		fwrite(&data.getMeshes()[m].indicesCount, sizeof(data.getMeshes()[m].indicesCount), 1, fp);
	}

	size_t vbsize = data.getVertexBuffer().size();
	fwrite(&vbsize, sizeof(vbsize), 1, fp);
	fwrite(data.getVertexBuffer().data(), vbsize, 1, fp);

	size_t ibsize = data.getIndexBuffer().size() * sizeof(unsigned int);
	fwrite(&ibsize, sizeof(ibsize), 1, fp);
	fwrite(data.getIndexBuffer().data(), ibsize, 1, fp);

	fclose(fp);

	saveMaterial(data, utils::Utils::trimExtention(filename) + ".material");

	return true;
}

void GeomSaver::saveMaterial(const Data& data, const std::string& filename)
{
	bool hasMaterial = false;
	auto meshes = data.getMeshes();
	for (size_t i = 0; i < meshes.size(); i++)
	{
		if (!meshes[i].material.diffuseMapFilename.empty() ||
			!meshes[i].material.normalMapFilename.empty() ||
			!meshes[i].material.specularMapFilename.empty()) hasMaterial = true;
	}

	if (hasMaterial)
	{
		FILE* fp = 0;
		fp = fopen(filename.c_str(), "w");
		if (!fp) return;

		Json::Value root;
		for (size_t i = 0; i < meshes.size(); i++)
		{
			Json::Value mat;
			mat["diffuseMap"] = meshes[i].material.diffuseMapFilename;
			mat["normalMap"] = meshes[i].material.normalMapFilename;
			mat["specularMap"] = meshes[i].material.specularMapFilename;
			root.append(mat);
		}
		Json::StyledWriter writer;
		std::string data = writer.write(root);
		fwrite(data.c_str(), data.length(), 1, fp);

		fclose(fp);
	}
}

}