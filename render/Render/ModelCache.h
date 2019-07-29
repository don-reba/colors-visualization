#pragma once

#include "Cache.h"
#include "IModel.h"

class ModelCache final
{
public:

	class Entry
	{
	public:

		Entry() = default;
		Entry(const Entry &) = delete;
		Entry(Entry && entry) noexcept;

		Entry(Cache<IModel> * cache, IModel * model) noexcept;

		~Entry();

		Entry & operator= (Entry &) = delete;
		Entry & operator= (Entry && entry);

		IModel & operator* () const noexcept;

	private:

		Cache<IModel> * cache = nullptr;
		IModel        * model = nullptr;
	};

public:

	Entry Load(const char * path);

	bool IsEmpty() const;

private:

	static aligned_unique_ptr<IModel> LoadFromFile(const char * path);

private:

	Cache<IModel> cache;
};
