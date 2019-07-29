#pragma once

#include "AlignedPtr.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

template<typename T>
class Cache final
{
private:

	struct Entry
	{
		Entry(std::string id, std::uint32_t refcount, aligned_unique_ptr<T> data)
			: id(std::move(id)), refcount(refcount), data(std::move(data)) {}
		std::string           id;
		std::uint32_t         refcount;
		aligned_unique_ptr<T> data;
	};

	using lock_guard = std::lock_guard<std::mutex>;

public:

	using CreateType = std::function<aligned_unique_ptr<T>()>;

public:

	T * Load(const char * id, const CreateType &Create)
	{
		using namespace std;

		lock_guard lock(mutex);

		auto iter = find_if(begin(cache), end(cache), [=](const auto &entry) { return entry.id == id; });
		if (iter == cache.end())
			return cache.emplace_back(id, 1u, Create()).data.get();
		++iter->refcount;
		return iter->data.get();
	}

	void Unload(const T * data)
	{
		using namespace std;

		lock_guard lock(mutex);

		auto iter = find_if(begin(cache), end(cache), [=](const auto &entry) { return entry.data.get() == data; });
		if (iter == cache.end())
			throw std::logic_error("Unloading invalid cache entry.");
		if (--iter->refcount == 0)
			cache.erase(iter);
	}

	bool IsEmpty() const
	{
		lock_guard lock(mutex);

		return cache.empty();
	}

private:

	std::vector<Entry> cache;

	mutable std::mutex mutex;
};