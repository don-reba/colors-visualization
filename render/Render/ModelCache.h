#pragma once

#include "Cache.h"
#include "IModel.h"

class ModelCache final
{
public:

	IModel * Load(const char * path);

	void Unload(const IModel * model);

	IModel * Swap(const IModel * model, const char * path);

	bool IsEmpty() const;

private:

	static aligned_unique_ptr<IModel> LoadFromFile(const char * path);

private:

	Cache<IModel> cache;
};
