#include "ModelCache.h"

#include "FgtVolume.h"
#include "Volume.h"

#include <filesystem>

IModel * ModelCache::Load(const char * path)
{
	return cache.Load(path, [=]{ return LoadFromFile(path); });
}

void ModelCache::Unload(const IModel * model)
{
	cache.Unload(model);
}

IModel * ModelCache::Swap(const IModel * model, const char * path)
{
	IModel * newModel = Load(path);
	if (model)
		Unload(model);
	return newModel;
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