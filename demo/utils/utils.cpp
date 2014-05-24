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
#include "utils.h"

namespace utils
{

void Utils::init()
{
	srand((unsigned int)time(0));
	Profiler::instance();
}

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

std::string Utils::getExtention(const std::string& fileName)
{
	auto tokens = tokenize<std::string>(fileName, '.');
	if (tokens.empty()) return "";
	auto it = tokens.rbegin();
	std::string ext = fileName.substr(it->first, it->second - it->first + 1);
	std::locale loc;
	for (std::string::size_type i = 0; i < ext.length(); ++i)
	{
		ext[i] = std::tolower(ext[i], loc);
	}
	
	return std::move(ext);
}

std::list<std::string> Utils::getExtentions(const std::string& fileName)
{
	std::list<std::string> result;
	auto tokens = tokenize<std::string>(fileName, '.');
	if (tokens.empty()) return result;
	
	auto it = tokens.begin();
	it++;
	for (; it != tokens.end(); ++it)
	{
		std::string ext = fileName.substr(it->first, it->second - it->first + 1);
		std::locale loc;
		for (std::string::size_type i = 0; i < ext.length(); ++i)
		{
			ext[i] = std::tolower(ext[i], loc);
		}
		result.push_back(std::move(ext));
	}

	return std::move(result);
}

std::string Utils::getPath(const std::string& fileName)
{
	size_t p_slash = fileName.find_last_of('/');
	size_t p_backslash = fileName.find_last_of('\\');
	if (p_slash == std::string::npos && p_backslash == std::string::npos)
		return "";

	size_t p = 0;
	if (p_slash != std::string::npos && p_backslash != std::string::npos)
		p = std::max(p_slash, p_backslash);
	else if (p_slash != std::string::npos)
		p = p_slash;
	else
		p = p_backslash;

	if (p == 0) return "";

	std::string path = fileName.substr(0, p + 1);
	return std::move(path);
}

std::string Utils::getFilename(const std::string& path)
{
	std::string p = path;
	std::replace(p.begin(), p.end(), '\\', '/');
	auto tokens = tokenize<std::string>(p, '/');
	if (tokens.empty()) return path;
	auto it = tokens.rbegin();
	return p.substr(it->first, it->second - it->first + 1);
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

float* Utils::convert(const vector2& v)
{
	static float arr[2];
	arr[0] = v.x; arr[1] = v.y;
	return arr;
}

float* Utils::convert(const quaternion& q)
{
	static float arr[4];
	arr[0] = q.x; arr[1] = q.y; arr[2] = q.z; arr[3] = q.w;
	return arr;
}

std::string Utils::fromUnicode( const std::wstring& str )
{
	size_t maxLen = str.length() + 1;
	std::unique_ptr<char[]> buffer(new char[maxLen]);

	int reslen = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, buffer.get(), maxLen, NULL, NULL);
	if (reslen < 0) reslen = 0;
	if (reslen < (int)maxLen) buffer.get()[reslen] = 0;
	else if (buffer.get()[maxLen - 1]) buffer.get()[0] = 0;

	std::string output = buffer.get();
	return std::move(output);
}

std::wstring Utils::toUnicode( const std::string& str )
{
	size_t maxLen = str.length() + 1;
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[maxLen]);

	int reslen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer.get(), maxLen);
	if (reslen < 0) reslen = 0;
	if (reslen < (int)maxLen) buffer.get()[reslen] = 0;
	else if (buffer.get()[maxLen - 1]) buffer.get()[0] = 0;

	std::wstring output = buffer.get();
	return std::move(output);
}

vector3 Utils::random(float minValue, float maxValue)
{
	float r1 = float(rand() % 1000) / float(999);
	float r2 = float(rand() % 1000) / float(999);
	float r3 = float(rand() % 1000) / float(999);
	float d = maxValue - minValue;
	return vector3(minValue + r1 * d, minValue + r2 * d, minValue + r3 * d);
}

std::map<std::string, int> Utils::parseCommandLine(const std::string& commandLine)
{
	std::map<std::string, int> result;
	if (commandLine.size() < 2) return std::move(result);

	std::list<size_t> offsets;
	size_t offset = 0;
	while (offset < commandLine.size())
	{
		offset = commandLine.find("--", offset);
		if (offset == std::string::npos) break;
		offsets.push_back(offset);

		offset += 2;
	}

	for (auto it = offsets.begin(); it != offsets.end(); ++it)
	{
		size_t start = (*it) + 2;
		auto it_next = it; it_next++;
		size_t end = it_next == offsets.end() ? commandLine.size() : (*it_next);
		std::string str = commandLine.substr(start, end - start);
		
		size_t p = str.find_first_of(' ');
		if (p == std::string::npos)
		{
			result[str] = 0;
		}
		else
		{
			std::string s = str.substr(0, p);
			size_t p2 = str.find_first_not_of(' ', p);
			if (p2 == std::string::npos)
			{
				result[s] = 0;
			}
			else
			{
				size_t p3 = str.find_first_of(' ', p2);
				std::string v = str.substr(p2, p3);
				int iv = atoi(v.c_str());
				result[s] = iv;
			}
		}
	}

	return std::move(result);
}

std::string Utils::currentTimeDate(bool withoutSpaces)
{
	std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(tp);
	std::string str = std::ctime(&t);
	if (withoutSpaces)
	{
		std::replace(str.begin(), str.end(), ' ', '_');
		std::replace(str.begin(), str.end(), ':', '_');
	}
	str.pop_back(); // remove \n
	return std::move(str);
}

}