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
#include "profiler.h"

namespace
{
	unsigned int GetThreadId()
	{
	#ifdef WIN32
		return GetCurrentThreadId();
	#endif
		return 0;
	}
}

namespace utils
{

Profiler& Profiler::instance()
{
	static Profiler profiler;
	return profiler;
}

Profiler::Profiler() :
	m_filename("profiler.txt")
{
	m_isRun = false;
	m_timer.init();

	unsigned int id = GetThreadId();
	this->registerThread(id, "main");
}

Profiler::~Profiler()
{
	cleanup();
}

Profiler::ProfilerObj::ProfilerObj(const std::string& name, bool enableHistory) :
	isStarted(false),
	historical(enableHistory)
{
	if (Profiler::instance().isRun())
	{
		isStarted = true;
		Profiler::instance().beginTrace(name);
	}
}

Profiler::ProfilerObj::~ProfilerObj()
{
	if (isStarted)
	{
		Profiler::instance().endTrace(historical);
	}
}

void Profiler::beginTrace( const std::string& name )
{
	if (!m_isRun) return;

	unsigned int id = GetThreadId();
	auto it = m_profilingTrees.find(id);
	if (it != m_profilingTrees.end())
	{
		Node* cur = it->second->current;

		it->second->treeMutex.lock();

		Node* ccur = nullptr;
		std::find_if(cur->children.begin(), cur->children.end(), [&](Node* node)
		{
			if (node->name == name)
			{
				ccur = node;
				return true;
			}
			return false;
		});
		
		if (ccur == nullptr)
		{
			Node* n = new Node(name);
			n->parent = cur;
			cur->children.push_back(n);
			ccur = n;
		}

		it->second->current = ccur;
		ccur->startTime = m_timer.getTime();

		it->second->treeMutex.unlock();
	}
}

void Profiler::endTrace(bool historical)
{
	if (!m_isRun) return;

	unsigned int id = GetThreadId();
	auto it = m_profilingTrees.find(id);
	if (it != m_profilingTrees.end())
	{
		it->second->treeMutex.lock();

		Node* cur = it->second->current;
		double instantTime = m_timer.getTime() - cur->startTime;
		cur->statistics.callsCount++;
		if (historical)
		{
			cur->statistics.history.push_back(instantTime);
		}
		if (cur->statistics.callsCount > 1)
		{
			cur->statistics.averageTime = (cur->statistics.averageTime + instantTime) / 2.0;
			if (instantTime < cur->statistics.minTime) cur->statistics.minTime = instantTime;
			if (instantTime > cur->statistics.maxTime) cur->statistics.maxTime = instantTime;
		}
		else
		{
			cur->statistics.minTime = instantTime;
			cur->statistics.maxTime = instantTime;
			cur->statistics.averageTime = instantTime;
		}	
		it->second->current = cur->parent;

		it->second->treeMutex.unlock();
	}
}

bool Profiler::registerThread( unsigned int id, const std::string& desc )
{
	if (m_isRun) return false;

	bool result = false;
	auto it = m_profilingTrees.find(id);
	if (it == m_profilingTrees.end())
	{
		ProfilingTree* tree = new ProfilingTree();
		tree->root = new Node("root");
		tree->current = tree->root;
		tree->threadDesc = desc;
		m_profilingTrees.insert(std::make_pair(id, tree));
		result = true;
	}
	return result;
}

void Profiler::deleteTree( Node* node )
{
	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		deleteTree(*it);
	}
	delete node;
}

void Profiler::cleanup()
{
	stop();
	auto it = m_profilingTrees.begin();
	for (; it != m_profilingTrees.end(); ++it)
	{
		deleteTree(it->second->root);
		delete it->second;
	}
	m_profilingTrees.clear();
}

void Profiler::run()
{
	m_isRun = true;
}

void Profiler::stop()
{
	m_isRun = false;
}

bool Profiler::isRun() const
{
	return m_isRun;
}

std::vector<int> Profiler::getProfilingThreads() const
{
	std::vector<int> v;
	auto it = m_profilingTrees.begin();
	v.reserve(m_profilingTrees.size());
	for (; it != m_profilingTrees.end(); ++it)
	{
		v.push_back(it->first);
	}
	return v;
}

std::string Profiler::getProfilingThreadDesc(unsigned int id) const
{
	auto it = m_profilingTrees.find(id);
	if (it != m_profilingTrees.end())
	{
		return it->second->threadDesc;
	}
	return "unknown";
}

void Profiler::forEachNode(Node* node, ProcessNodeFunc processNode, int depth)
{
	if (node == nullptr) return;
	processNode(node->name, node->statistics, depth);

	std::for_each(node->children.begin(), node->children.end(), [&](Node* childNode)
	{
		forEachNode(childNode, processNode, depth + 1);
	});
}

void Profiler::forEach(unsigned int id, ProcessNodeFunc processNode)
{
	if (processNode == nullptr) return;

	auto it = m_profilingTrees.find(id);
	if (it != m_profilingTrees.end())
	{
		forEachNode(it->second->root, processNode, 0);
	}
}

void Profiler::setFilename(const std::string& filename)
{
	m_filename = filename;
}

void Profiler::setHeader(const std::string& header)
{
	m_header = header;
}

void Profiler::saveToFile()
{
	std::vector<int> threads = getProfilingThreads();
	if (!threads.empty())
	{
		std::ofstream profilerFile;
		profilerFile.open(m_filename);
		if (profilerFile.is_open())
		{
			if (!m_header.empty())
			{
				profilerFile << m_header << "\n";
			}
			for (size_t i = 0; i < threads.size(); i++)
			{
				profilerFile << "Thread (" << threads[i] << ", " << getProfilingThreadDesc(threads[i]) << "):\n";
				forEach(threads[i], [&](const std::string& name, const Statistics& stats, int depth)
				{
					for (int d = 0; d < depth; d++) profilerFile << "  ";
					profilerFile << "> " << name;
					if (stats.callsCount == 0) { profilerFile << "\n";  return; }
					profilerFile << ": calls = " << stats.callsCount
								 << ", min = " << stats.minTime * 1000.0 << "ms"
								 << ", max = " << stats.maxTime * 1000.0 << "ms"
								 << ", average = " << stats.averageTime * 1000.0 << "ms\n";
					if (!stats.history.empty())
					{
						for (int d = 0; d < depth; d++) profilerFile << "  ";
						profilerFile << "  history = { ";
						for (auto it = stats.history.begin(); it != stats.history.end(); ++it)
							profilerFile << (*it) * 1000.0 << " ";
						profilerFile << "}\n";
					}
				});
			}
			profilerFile.close();
		}
	}
}

}