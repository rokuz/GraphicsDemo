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
#include "geomloader.h"
#include "json/json.h"
#include <fstream>

namespace geom
{

Data GeomLoader::load(const std::string& filename)
{
	Data data;
	DataWriter writer(&data);

	FILE* fp = 0;
	size_t filesize = 0;
	fp = fopen(filename.c_str(), "rb");
	if (!fp)
	{
		writer.getLastErrorRef() = std::string("Could not open file '") + filename + "'";
		return data;
	}

	size_t magic = 0;
	fread(&magic, sizeof(magic), 1, fp);
	if (magic != MAGIC_GEOM)
	{
		writer.getLastErrorRef() = "Unrecognized (or obsolete) format of geom-file";
		fclose(fp);
		return data;
	}

	float fbuf[3] = { 0, 0, 0 };
	fread(&fbuf, sizeof(fbuf), 1, fp);
	float fbuf2[3] = { 0, 0, 0 };
	fread(&fbuf2, sizeof(fbuf2), 1, fp);
	writer.getBoundingBoxRef().vmin = vector3(fbuf[0], fbuf[1], fbuf[2]);
	writer.getBoundingBoxRef().vmax = vector3(fbuf2[0], fbuf2[1], fbuf2[2]);

	// vertex declaration
	size_t componentsCount = 0;
	fread(&componentsCount, sizeof(componentsCount), 1, fp);
	fread(&writer.getAdditionalUVsCountRef(), sizeof(writer.getAdditionalUVsCountRef()), 1, fp);
	size_t vertexSize = 0;
	fread(&vertexSize, sizeof(vertexSize), 1, fp);
	if (componentsCount != data.getVertexComponentsCount())
	{
		writer.getLastErrorRef() = "Incorrect format of geom-file (componentsCount)";
		fclose(fp);
		return data;
	}
	if (vertexSize != data.getVertexSize())
	{
		writer.getLastErrorRef() = "Geom importer error: Incorrect format of geom-file (vertexSize)";
		fclose(fp);
		return data;
	}
	for (size_t c = 0; c < componentsCount; c++)
	{
		size_t vcs = 0;
		fread(&vcs, sizeof(vcs), 1, fp);
		size_t vco = 0;
		fread(&vco, sizeof(vco), 1, fp);
		if (vcs != data.getVertexComponentSize(c))
		{
			writer.getLastErrorRef() = "Incorrect format of geom-file (size of component %d)";
			fclose(fp);
			return data;
		}
		if (vco != data.getVertexComponentOffset(c))
		{
			writer.getLastErrorRef() = "Incorrect format of geom-file (offset of component %d)";
			fclose(fp);
			return data;
		}
	}

	size_t meshesCount = 0;
	fread(&meshesCount, sizeof(meshesCount), 1, fp);
	if (meshesCount == 0)
	{
		writer.getLastErrorRef() = "Incorrect number of meshes";
		fclose(fp);
		return data;
	}

	// meshes
	writer.getMeshesRef().resize(meshesCount);
	for (size_t m = 0; m < meshesCount; m++)
	{
		fread(&writer.getMeshesRef()[m].offsetInIB, sizeof(writer.getMeshesRef()[m].offsetInIB), 1, fp);
		fread(&writer.getMeshesRef()[m].indicesCount, sizeof(writer.getMeshesRef()[m].indicesCount), 1, fp);
	}

	// vertex buffer
	size_t vbsize = 0;
	fread(&vbsize, sizeof(vbsize), 1, fp);
	writer.getVertexBufferRef().resize(vbsize);
	fread(writer.getVertexBufferRef().data(), sizeof(unsigned char), vbsize, fp);
	writer.getVerticesCountRef() = vbsize / vertexSize;

	// index buffer
	size_t ibsize = 0;
	fread(&ibsize, sizeof(ibsize), 1, fp);
	size_t indicesCount = ibsize / sizeof(unsigned int);
	writer.getIndexBufferRef().resize(indicesCount);
	fread(writer.getIndexBufferRef().data(), sizeof(unsigned int), indicesCount, fp);

	fclose(fp);

	loadMaterial(utils::Utils::trimExtention(filename) + ".material", writer);
	
	return data;
}

void GeomLoader::loadMaterial(const std::string& filename, DataWriter& dataWriter)
{
	std::ifstream matFile(filename);
	if (!matFile.is_open()) return;

	Json::Value root;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(matFile, root);
	matFile.close();
	if (!parsingSuccessful) return;

	for (size_t i = 0; i < root.size(); i++)
	{
		Data::Material mat;
		mat.diffuseMapFilename = root[i]["diffuseMap"].asString();
		mat.normalMapFilename = root[i]["normalMap"].asString();
		mat.specularMapFilename = root[i]["specularMap"].asString();

		if (i < dataWriter.getMeshesRef().size())
		{
			dataWriter.getMeshesRef()[i].material = mat;
		}
	}
}

}