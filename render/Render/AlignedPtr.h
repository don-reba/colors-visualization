#pragma once

#include <boost/align/aligned_delete.hpp>

#include <memory>

template <typename T>
using aligned_unique_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

template <typename T, typename ... Args>
aligned_unique_ptr<T> make_aligned_unique(Args && ...args)
{
	void * ptr = boost::alignment::aligned_alloc(32, sizeof(T));
	return aligned_unique_ptr<T>(new(ptr) T(std::forward<Args>(args)...));
}