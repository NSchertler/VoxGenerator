#pragma once

#include <unordered_map>
#include <array>
#include <vector>
#include "common.h"
struct Cell
{
	std::vector<Voxel> voxels;

	std::array<int, 3> min = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
	std::array<int, 3> max = { std::numeric_limits<int>::min(), std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };
};

//hash function
struct GridHashFunc
{
	size_t operator()(const Position& idx) const
	{
		static const int p1 = 131071;
		static const int p2 = 524287;
		static const int p3 = 8191;
		return idx[0] * p1 + idx[1] * p2 + idx[2] * p3;
	}
};
//type of internal hash map
typedef std::unordered_map<std::array<int, 3>, Cell, GridHashFunc> HashGridType;