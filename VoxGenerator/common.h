#pragma once

#include <array>

//data type for a 3D integer position
typedef std::array<int, 3> Position;

//the smallest possible position
const Position MINPOS = { std::numeric_limits<int>::min(), std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

//the largest possible position
const Position MAXPOS = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };

//data type for a voxel
struct Voxel
{
	Position position;

	//color index
	uint8_t color;
};