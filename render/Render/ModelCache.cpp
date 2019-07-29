#include "ModelCache.h"

#include "FgtVolume.h"
#include "Volume.h"

#include <filesystem>

ModelCache::Entry ModelCache::Load(const char * path)
{
	return Entry(&cache, cache.Load(path, [=]{ return LoadFromFile(path); }));
}

bool ModelCache::IsEmpty() const
{
	return cache.IsEmpty();
}

aligned_unique_ptr<IModel> ModelCache::LoadFromFile(const char * path)
{
	const auto fsPath = std::filesystem::path(path);

	if (!std::filesystem::exists(fsPath))
		throw std::invalid_argument("Path '" + fsPath.string() + "' not found.");

	const auto extension = fsPath.extension().string();
	if (extension == ".fgt")
		return make_aligned_unique<FgtVolume>(path);
	if (extension == ".vxl")
		return make_aligned_unique<Volume>(path);
	throw std::invalid_argument("Unknown extension: '" + extension + "'.");
}

//------------------
// ModelCache::Entry
//------------------

ModelCache::Entry::Entry(Cache<IModel> * cache, IModel * model) noexcept
	: cache(cache), model(model) {}

ModelCache::Entry::Entry(ModelCache::Entry::Entry && entry) noexcept
	: cache(entry.cache), model(entry.model)
{
	entry.cache = nullptr;
	entry.model = nullptr;

ModelCache::Entry::~Entry()
{
	if (cache)
		cache->Unload(model);
}

ModelCache::Entry & ModelCache::Entry::operator= (Entry && entry)
{
	cache = entry.cache;
	model = entry.model;

	entry.cache = nullptr;
	entry.model = nullptr;

	return *this;
}

IModel & ModelCache::Entry::operator* () const noexcept
{
	return *model;
}