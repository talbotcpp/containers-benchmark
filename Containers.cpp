// Containers benchmark for CppCon 2026 Back to Basics: Containers lecture.
//
// Copyright 2026 Alan D. Talbot

#include <windows.h>
#include <psapi.h>

// Turn off nasty Windows macros which break everything.
#undef max
#undef min

//import sequence;
//import std;

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

//constexpr sequence_traits inplace {			// This sets "sequence" up to match the behavior of inplace_vector.
//		.storage = storage_modes::VARIABLE,	// This makes the storage be local (only), no dynamic allocation.
//		.location = location_modes::FRONT,	// This sets where the data gather in the capacity: vector/inplace_vector have data at the FRONT.
//		.capacity = reserve_size,			// The reserve_size value is used here for the compile-time fixed capacity.
//};
//using container_type = sequence<long, inplace>;
//using container_type = vector<long>;
//using container_type = deque<long>;
using container_type = list<long>;
//using container_type = inplace_vector<long, reserve_size>;

using value_type = container_type::value_type;

size_t get_memory()
{
    // Use the extended structure to gain access to PrivateUsage
    PROCESS_MEMORY_COUNTERS_EX pmc;
    
    // Call the API targeting the current process
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        
        // Convert raw byte allocations into Megabytes for readability
        double physicalRamMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        double privateBytesMB = static_cast<double>(pmc.PrivateUsage) / (1024.0 * 1024.0);

		return pmc.PrivateUsage;
    } else {
        println("Failed to retrieve memory statistics. Error: {}", GetLastError());
		return 0;
    }
}

template<typename T>
concept reservable = requires(T& t) {
    t.reserve(0);
};

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

	T container;
};

//template<typename E>
//struct element<vector<E>> {
//
//	using data_type = E;
//
//	element()
//	{
//		if constexpr (reserve_size)
//			container.reserve(reserve_size);
//	}
//
//	void add_front(data_type x)
//	{
//		container.insert(container.begin(), x);
//	}
//	void add_back(data_type x)
//	{
//		container.push_back(x);
//	}
//
//	vector<E> container;
//};

void add(auto& element, value_type v)
{
	if constexpr (location)
		element.add_front(v);
	else
		element.add_back(v);
}

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
			add(e, n);

    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::microseconds>(end_time - start_time); //.count();
    auto end_mem = get_memory();

	println();
	println("Type = \t{}", typeid(container_type).name());
	println("Size = \t{} ({})", cont_size, location ? "front" : "back");
	println("Rack = \t{} ({})", rack_size, reserve_size);
	println();
	println("Time = \t{}", double(elapsed.count()) / 1000.0);
	println();
	println("StaM = \t{}", start_mem);
	println("EndM = \t{}", end_mem);
	println("MemU = \t{}", double(end_mem - start_mem) / (1024.0 * 1024.0));
	println();
	
	return 0;
}

