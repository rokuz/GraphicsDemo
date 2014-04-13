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

#include "geometry.h"

#include "geomloader.h"
#include "geomsaver.h"
#ifdef _USE_FBX
#include "fbxloader.h"
#endif

#include "utils.h"

namespace geom
{

Geometry::Geometry()
{
	registerLoader<GeomLoader>("geom");
	registerSaver<GeomSaver>("geom");

	#ifdef _USE_FBX
	registerLoader<FbxLoader>("fbx");
	#endif
}

std::shared_ptr<GeometryLoader> Geometry::getLoader(const std::string& extention) const
{
	if (m_loaders.find(extention) == m_loaders.end()) return std::shared_ptr<GeometryLoader>(new GeometryLoader());
	return m_loaders.at(extention);
}

std::shared_ptr<GeometrySaver> Geometry::getSaver(const std::string& extention) const
{
	if (m_loaders.find(extention) == m_loaders.end()) return std::shared_ptr<GeometrySaver>(new GeometrySaver());
	return m_savers.at(extention);
}

Data Geometry::load(const std::string& filepath)
{
	std::string ext = utils::Utils::getExtention(filepath);
	if (ext.empty())
	{
		GeometryLoader loader;
		return loader.load(filepath);
	}
	return getLoader(ext)->load(filepath);
}

bool Geometry::save(const Data& data, const std::string& filepath)
{
	std::string ext = utils::Utils::getExtention(filepath);
	if (ext.empty())
	{
		GeometrySaver saver;
		return saver.save(data, filepath);
	}
	return getSaver(ext)->save(data, filepath);
}

}