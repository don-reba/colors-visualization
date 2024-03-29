#pragma once

#include <vector>

struct Subsample final
{
	float dx;
	float dy;
};

using AAMask = std::vector<Subsample>;

static const AAMask aa1x
	{ { 0.5f, 0.5f } };
static const AAMask aa4x
	{ { 0.125f, 0.625f }
	, { 0.375f, 0.125f }
	, { 0.875f, 0.375f }
	, { 0.625f, 0.875f }
	};
static const AAMask aa8x
	{ { 0.375f, 0.000f }
	, { 0.875f, 0.125f }
	, { 0.125f, 0.250f }
	, { 0.625f, 0.375f }
	, { 0.375f, 0.500f }
	, { 0.875f, 0.625f }
	, { 0.125f, 0.650f }
	, { 0.625f, 0.875f }
	};