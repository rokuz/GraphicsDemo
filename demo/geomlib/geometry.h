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

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#ifdef WIN32
    #pragma once
#endif

#include <string>
#include <map>
#include <memory>
#include "geometryloader.h"
#include "geometrysaver.h"

namespace geom
{

class Geometry
{
	Geometry();
	~Geometry(){}

public:
	static Geometry& instance()
	{
		static Geometry inst;
		return inst;
	}

	template<typename LoaderType>
	void registerLoader(const std::string& extention)
	{
		m_loaders[extention] = std::shared_ptr<GeometryLoader>(new LoaderType());
	}

	std::shared_ptr<GeometryLoader> getLoader(const std::string& extention) const;

	template<typename SaverType>
	void registerSaver(const std::string& extention)
	{
		m_savers[extention] = std::shared_ptr<GeometrySaver>(new SaverType());
	}

	std::shared_ptr<GeometrySaver> getSaver(const std::string& extention) const;

	Data load(const std::string& filepath);
	bool save(const Data& data, const std::string& filepath);

private:
	std::map<std::string, std::shared_ptr<GeometryLoader> > m_loaders;
	std::map<std::string, std::shared_ptr<GeometrySaver> > m_savers;
};


}

#endif