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

#include <list>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

#include "vector.h"
#include "bbox.h"

#include "geometry.h"

using namespace std;

void convert(const std::string& filename)
{
	std::size_t pos = filename.find('.');
	std::string outname;
	if (pos == std::string::npos)
	{
		outname = filename + ".geom";
	}
	else
	{
		outname = filename.substr(0, pos + 1);
		outname.append("geom");
	}

	// converting
	bool result = false;
	{
		auto data = geom::Geometry::instance().load(filename);
		if (data.isCorrect())
		{
			result = geom::Geometry::instance().save(data, outname);
			if (!result)
			{
				cout << "geomconv error: Failed to save file '" << outname << "'.\n";
			}
		}
		else
		{
			cout << "geomconv error: Failed to load file '" << filename << "'. Reason: "<< data.getLastError() << ".\n";
		}
	}

	if (result)
	{
		cout << "Converting has finished successfully.\n";
	}
}

int main(int argc, const char ** argv)
{
	if (argc != 2)
	{
		cout << "geomconv error: Command line arguments are incorrect. You have to call [geomconv filename.fbx].\n";
		return -1;
	}
	convert(std::string(argv[1]));

	return 0;
}