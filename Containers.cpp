// Containers benchmark for CppCon 2026 Back to Basics: Containers lecture.
//
// Copyright 2026 Alan D. Talbot

#include <windows.h>
#include <psapi.h>

// Turn off nasty Windows macros which break everything.
#undef max
#undef min

#include <vector>
#include <list>
#include <deque>
#include <print>
#include <chrono>

#include <beman/inplace_vector/inplace_vector.hpp>

using namespace std;
using namespace beman::inplace_vector;

constexpr long reserve_size = 10;	// Set the reserve size (and inplace size) here; 0 means reserve will not be called.
constexpr enum : bool {BACK, FRONT} location = BACK;	// Set the insertion location.

using container_type = list<long>;
//using container_type = deque<long>;
//using container_type = vector<long>;
//using container_type = inplace_vector<long, reserve_size>;

using value_type = container_type::value_type;

size_t get_memory()
{
    PROCESS_MEMORY_COUNTERS_EX pmc;	// Use the extended structure to gain access to PrivateUsage
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
		return pmc.PrivateUsage;
	else
	{
        println("Failed to retrieve memory statistics. Error: {}", GetLastError());
		return 0;
    }
}

template<typename T>
struct element {

	using data_type = T::value_type;

	element() requires
		requires(T& t) { t.reserve(reserve_size); }
	{
		if constexpr (reserve_size)
			container.reserve(reserve_size);
	}
	element() = default;

	void add_front(data_type x) requires
		requires(const T& t) { t.push_front(x); }
	{
		container.push_front(x);
	}
	void add_front(data_type x)
	{
		container.insert(container.begin(), x);
	}

	void add_back(data_type x)
	{
		container.push_back(x);
	}

	void add(data_type v)
	{
		if constexpr (location)
			add_front(v);
		else
			add_back(v);
	}

	T container;
};

int main(int argc, char* argv[])
{
	long cont_size = 10;
	if (argc > 1) cont_size = atol(argv[1]);
	long rack_size = 1000;
	if (argc > 2) rack_size = atol(argv[2]);

	auto start_mem = get_memory();

	vector<element<container_type>> rack;
	rack.resize(rack_size);

    auto start_time = chrono::high_resolution_clock::now();

	for (value_type n = 1; n <= cont_size; ++n)
		for (auto&& e : rack)
			e.add(n);

    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
    auto end_mem = get_memory();

	println();
	println("Type = \t{}", typeid(container_type).name());
	println("Size = \t{} ({})", cont_size, location ? "front" : "back");
	println("Rack = \t{} ({})", rack_size, reserve_size);
	println();
	println("Time = \t{} ms", double(elapsed.count()) / 1000.0);				// Convert chrono μs to floating ms.
	println("MemU = \t{} MB", double(end_mem - start_mem) / (1024.0 * 1024.0));	// Convert bytes to megabytes.
	println();
	
	return 0;
}

