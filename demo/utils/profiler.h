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

#ifndef __PROFILER_H__
#define __PROFILER_H__

namespace utils
{

class Profiler
{
	friend class ProfilerObj;
	
public:
	struct Statistics
	{
		double averageTime;
		double minTime;
		double maxTime;
		int callsCount;
		std::list<double> history;

		Statistics() : averageTime(0.0), minTime(0.0), maxTime(0.0), callsCount(0) {}
	};

	typedef std::function<void(const std::string&, const Statistics&, int)> ProcessNodeFunc;

protected:
	struct Node
	{
		std::string name;
		Node* parent;
		std::list<Node*> children;
		double startTime;
		Statistics statistics;

		Node(const std::string& name) : name(name), parent(nullptr), startTime(0.0) {}
	};

	struct ProfilingTree
	{
		Node* current;
		Node* root;
		std::mutex treeMutex;
		std::string threadDesc;

		ProfilingTree() : current(nullptr), root(nullptr) {}
	};

	std::map<unsigned int, ProfilingTree*> m_profilingTrees;
	bool m_isRun;
	Timer m_timer;

	void beginTrace(const std::string& name);
	void endTrace(bool historical);
	void deleteTree(Node* node);
	void forEachNode(Node* node, ProcessNodeFunc processNode, int depth);
	void cleanup();

	Profiler();
	~Profiler();

public:
	class ProfilerObj
	{
		bool isStarted;
		bool historical;
	public:
		ProfilerObj(const std::string& name, bool enableHistory);
		~ProfilerObj();
	};

	static Profiler& instance();
	bool registerThread(unsigned int id, const std::string& desc = "");
	void run();
	void stop();
	bool isRun() const;
	void forEach(unsigned int id, ProcessNodeFunc processNode);
	std::vector<int> getProfilingThreads() const;
	std::string getProfilingThreadDesc(unsigned int id) const;

	void saveToFile();
};

#define TRACE_FUNCTION utils::Profiler::ProfilerObj __profiler_obj__(__FUNCTION__, false);
#define TRACE_BLOCK(blockName) utils::Profiler::ProfilerObj __profiler_obj__(std::string(__FUNCTION__) + blockName, false);
#define TRACE_FUNCTION_HISTORICAL utils::Profiler::ProfilerObj __profiler_obj__(__FUNCTION__, true);
#define TRACE_BLOCK_HISTORICAL(blockName) utils::Profiler::ProfilerObj __profiler_obj__(std::string(__FUNCTION__) + blockName, true);

}

#endif