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

#ifndef __UTILS_H__
#define __UTILS_H__

namespace utils
{

class Utils
{
public:
	static void init();
	static bool exists(const std::string& fileName);
	static bool readFileToString(const std::string& fileName, std::string& out);
	static std::string getExtention(const std::string& fileName);
	static std::list<std::string> getExtentions(const std::string& fileName);
	static std::string getPath(const std::string& path);
	static std::string getFilename(const std::string& path);
	static float* convert(const vector4& v);
	static float* convert(const vector3& v);
	static float* convert(const vector2& v);
	static float* convert(const quaternion& q);
	static std::string fromUnicode(const std::wstring& str);
	static std::wstring toUnicode(const std::string& str);
	static vector3 random(float minValue = 0.0f, float maxValue = 1.0f);
	static std::map<std::string, int> parseCommandLine(const std::string& commandLine);
	
	template<typename StringType>
	static std::list<std::pair<size_t, size_t> > tokenize(const StringType& str, typename StringType::value_type delimiter)
	{
		std::list<std::pair<size_t, size_t> > result;
		if (str.empty()) return result;

		size_t offset = 0;
		size_t pos = 0;
		while ((pos = str.find_first_of(delimiter, offset)) != StringType::npos)
		{
			if (pos != 0 && offset != pos)
				result.push_back(std::make_pair((size_t)offset, (size_t)pos - 1));

			offset = pos + 1;
		}

		if (offset < str.length())
		{
			result.push_back(std::make_pair((size_t)offset, str.length() - 1));
		}

		return result;
	}
};

}

#endif