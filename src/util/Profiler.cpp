#include "Profiler.h"
#include "Logger.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

void Profiler::push(std::string gate)
{
#ifdef ENABLE_PROFILER
	stack.push_back(gate);
	checkpoints[stack] = glfwGetTime();
#endif
}

void Profiler::pop()
{
#ifdef ENABLE_PROFILER
	logger->check(stack.size() > 0, "Tried to pop on an empty profiler stack");
	
	// On each pop we add to the results
	auto it = results.find(stack);
	if(it == results.end())
	{
		it = results.insert(std::make_pair(stack, RunStats())).first;
	}

	RunStats& st = it->second;

	st.count++;
	st.last = glfwGetTime() - checkpoints[stack];
	st.avg = (st.avg * (st.count - 1) + st.last) / st.count;
	if (st.max < st.last) st.max = st.last;
	if (st.min > st.last) st.min = st.last;

	
	stack.pop_back();
#endif
}

void Profiler::show_results()
{
#ifdef ENABLE_PROFILER
	for(auto& pair : results)
	{
		std::string full = "";
		for(size_t i = 0; i < pair.first.size(); i++)
		{
			full += pair.first[i];
			if(i != pair.first.size() - 1)
			{
				full += '.';
			}
		}
		
		logger->info("{0}: max: {2:.4f}ms min: {3:.4f}ms avg: {1:.4f}ms last: {4:.4f}ms", 
				full, pair.second.avg*1000.0, pair.second.max*1000.0, pair.second.min*1000.0, pair.second.last*1000.0);
	}
#endif
}

void Profiler::show_imgui()
{
	// TODO: Think this out, we are showing results while we are generating them
	ImGui::Begin("Profiler");

#ifdef ENABLE_PROFILER
	for(auto& pair : results)
	{
		std::string full = "";
		for(size_t i = 0; i < pair.first.size(); i++)
		{
			full += pair.first[i];
			if(i != pair.first.size() - 1)
			{
				full += '.';
			}
		}
		
		ImGui::Text("%s -> %fms", full.c_str(), pair.second.avg * 1000.0);
	}
#else 
	ImGui::Text("Profiler is compile-time disabled");
#endif

	ImGui::End();
}

ProfileBlock Profiler::block(std::string name)
{
	push(name);
	return std::move(ProfileBlock());
}

Profiler* profiler;

void create_global_profiler()
{
	profiler = new Profiler();
}

void destroy_global_profiler()
{
	delete profiler;
}
